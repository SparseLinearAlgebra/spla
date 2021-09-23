/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#include <spla-cpp/SplaLibrary.hpp>
#include <detail/SplaError.hpp>
#include <detail/SplaLibraryPrivate.hpp>
#include <boost/compute/device.hpp>
#include <boost/compute/system.hpp>
#include <memory>

namespace {
    boost::compute::platform FindPlatform(const std::string& platformName) {
        for (const auto& platform : boost::compute::system::platforms()) {
            if (platform.name() == platformName) {
                return platform;
            }
        }
        RAISE_ERROR(DeviceNotPresent, ("No such OpenCL platform: '" + platformName + "'").c_str());
    }

    boost::compute::device FindDevice(const std::string& deviceName) {
        try {
            return boost::compute::system::find_device(deviceName);
        } catch (const boost::compute::no_device_found&) {
            RAISE_ERROR(DeviceNotPresent, ("No such OpenCL device present: '" + deviceName + "'").c_str());
        }
    }

    void CheckAnyDevicePresent(const std::vector<std::string>& devices) {
        if (devices.empty()) {
            RAISE_ERROR(DeviceNotPresent, "No OpenCL found matching given constraints");
        }
    }

    std::vector<std::string> SelectDevices(
        const std::string& platform,
        spla::Library::Config::DeviceType type,
        spla::Library::Config::DeviceAmount amount
    ) {
        std::vector<std::string> selectedDevices;
        for (const auto& device : FindPlatform(platform).devices()) {
            if (static_cast<spla::Library::Config::DeviceType>(device.type()) == type) {
                selectedDevices.push_back(device.name());
                if (amount == spla::Library::Config::DeviceAmount::One) {
                    break;
                }
            }
        }
        CheckAnyDevicePresent(selectedDevices);
        return selectedDevices;
    }

    std::vector<std::string> SelectDevices(
        spla::Library::Config::DeviceType type,
        spla::Library::Config::DeviceAmount amount
    ) {
        std::vector<std::string> platformDevices;
        for (const auto& platform : boost::compute::system::platforms()) {
            std::vector<std::string> thisPlatformDevices =
                SelectDevices(platform.name(), type, amount);
            if (thisPlatformDevices.size() > platformDevices.size()) {
                platformDevices = thisPlatformDevices;
            }
        }
        CheckAnyDevicePresent(platformDevices);
        return platformDevices;
    }

    std::vector<std::string> SelectDevices(
        spla::Library::Config::DeviceAmount amount
    ) {
        if (boost::compute::system::platform_count() == 0) {
            RAISE_ERROR(DeviceNotPresent, "Runtime environment does not have any OpenCL platform");
        }
        std::vector<std::string> platformDevices;
        for (const auto& platform : boost::compute::system::platforms()) {
            if (platform.device_count() > platformDevices.size()) {
                platformDevices.clear();
                for (const auto& platformDevice : platform.devices()) {
                    platformDevices.push_back(platformDevice.name());
                }
            }
        }
        if (amount == spla::Library::Config::DeviceAmount::One && !platformDevices.empty()) {
            platformDevices.resize(1);
        }
        CheckAnyDevicePresent(platformDevices);
        return platformDevices;
    }

    boost::compute::device SelectDefaultDevice() {
        try {
            return boost::compute::system::default_device();
        } catch (const boost::compute::no_device_found&) {
            RAISE_ERROR(DeviceNotPresent, "Default device not found");
        }
    }
}

spla::Library::Library(const Config& config)
    : mPrivate(std::make_unique<LibraryPrivate>(*this, config)) {
}

spla::Library::Library()
    : Library(Config()) {
}

spla::Library::~Library() {

}

void spla::Library::Submit(const spla::RefPtr<spla::Expression> &expression) {
    GetPrivate().GetExprManager()->Submit(expression);
}

class spla::LibraryPrivate& spla::Library::GetPrivate() {
    return *mPrivate;
}

spla::Library::Config::Config(const std::vector<std::string>& devices) : mDevicesNames(std::move(devices)) {}

spla::Library::Config::Config() : Config({SelectDefaultDevice().name()}) {}

spla::Library::Config::Config(
    const std::string& platform,
    DeviceType type,
    DeviceAmount amount
) : Config(SelectDevices(platform, type, amount)) {}

spla::Library::Config::Config(
    DeviceType type,
    DeviceAmount amount
) : Config(SelectDevices(type, amount)) {}

spla::Library::Config::Config(
    DeviceAmount amount
) : Config(SelectDevices(amount)) {}

const std::vector<std::string>& spla::Library::Config::GetDevicesNames() const noexcept {
    return mDevicesNames;
}
