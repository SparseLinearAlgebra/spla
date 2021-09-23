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
#include <memory>

namespace {
    void CheckAnyDevicePresent(const std::vector<boost::compute::device>& devices) {
        if (devices.empty()) {
            RAISE_ERROR(DeviceNotPresent, "No OpenCL found matching given constraints");
        }
    }

    std::vector<boost::compute::device> SelectDevices(
        const boost::compute::platform& platform,
        spla::Library::Config::DeviceType type,
        spla::Library::Config::DeviceAmount amount
    ) {
        std::vector<boost::compute::device> selectedDevices;
        for (const auto& device : platform.devices()) {
            if (static_cast<spla::Library::Config::DeviceType>(device.type()) == type) {
                selectedDevices.push_back(device);
                if (amount == spla::Library::Config::DeviceAmount::One) {
                    break;
                }
            }
        }
        CheckAnyDevicePresent(selectedDevices);
        return selectedDevices;
    }

    std::vector<boost::compute::device> SelectDevices(
        spla::Library::Config::DeviceType type,
        spla::Library::Config::DeviceAmount amount
    ) {
        std::vector<boost::compute::device> platformDevices;
        for (const auto& platform : boost::compute::system::platforms()) {
            std::vector<boost::compute::device> thisPlatformDevices =
                SelectDevices(platform, type, amount);
            if (thisPlatformDevices.size() > platformDevices.size()) {
                platformDevices = thisPlatformDevices;
            }
        }
        CheckAnyDevicePresent(platformDevices);
        return platformDevices;
    }

    std::vector<boost::compute::device> SelectDevices(
        spla::Library::Config::DeviceAmount amount
    ) {
        std::vector<boost::compute::device> platformDevices;
        for (const auto& platform : boost::compute::system::platforms()) {
            if (platform.device_count() > platformDevices.size()) {
                platformDevices = platform.devices();
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

spla::Library::Library() {
    mPrivate = std::make_unique<LibraryPrivate>(*this);
}

spla::Library::~Library() {

}

void spla::Library::Submit(const spla::RefPtr<spla::Expression> &expression) {
    GetPrivate().GetExprManager()->Submit(expression);
}

class spla::LibraryPrivate& spla::Library::GetPrivate() {
    return *mPrivate;
}

spla::Library::Config::Config(std::vector<boost::compute::device> devices)
    : mDevices(std::move(devices)),
      mContext(mDevices) {
}

spla::Library::Config::Config() : Config({SelectDefaultDevice()}) {}

spla::Library::Config::Config(
    const boost::compute::platform& platform,
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
