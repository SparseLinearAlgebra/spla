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

#include <memory>
#include <sstream>
#include <thread>

#include <boost/compute/device.hpp>
#include <boost/compute/system.hpp>

#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaTypes.hpp>

spla::Library::Library(Config config) : mPrivate(std::make_shared<LibraryPrivate>(*this, std::move(config))) {
    Types::RegisterTypes(*this);
}

spla::Library::Library() : Library(Config()) {
}

spla::Library::~Library() = default;

class spla::LibraryPrivate &spla::Library::GetPrivate() const noexcept {
    return *mPrivate;
}

const std::shared_ptr<class spla::LibraryPrivate> &spla::Library::GetPrivatePtr() const noexcept {
    return mPrivate;
}

std::string spla::Library::PrintContextConfig() const noexcept {
    std::stringstream confStream;

    const boost::compute::platform &platform = GetPrivate().GetPlatform();
    const std::vector<boost::compute::device> &devices = GetPrivate().GetDevices();
    confStream << "Platform: " << platform.name() << " (" << platform.id() << ")" << '\n'
               << "   Vendor: " << platform.vendor() << '\n'
               << "   Profile: " << platform.profile() << '\n'
               << "   Version: " << platform.version() << '\n'
               << "Total Devices: " << devices.size() << '\n';
    for (const boost::compute::device &device : devices) {
        confStream << "   * Device: " << device.name() << " (" << device.id() << ")" << '\n'
                   << "        Vendor: " << device.vendor() << '\n'
                   << "        Profile: " << device.profile() << '\n'
                   << "        Version: " << device.version() << '\n';
    }
    return confStream.str();
}

spla::Library::Config::Config() = default;

spla::Library::Config &spla::Library::Config::SetPlatform(std::string platformName) {
    mPlatformName.emplace(std::move(platformName));
    return *this;
}

spla::Library::Config &spla::Library::Config::LimitAmount(std::size_t amount) {
    mDeviceAmount = std::optional{amount};
    return *this;
}

spla::Library::Config &spla::Library::Config::RemoveAmountLimit() {
    mDeviceAmount = std::nullopt;
    return *this;
}

spla::Library::Config &spla::Library::Config::SetDeviceType(DeviceType type) {
    mDeviceType.emplace(type);
    return *this;
}

spla::Library::Config &spla::Library::Config::SetLogFilename(spla::Filename filename) {
    mLogFilename.emplace(std::move(filename));
    return *this;
}

spla::Library::Config &spla::Library::Config::SetBlockSize(std::size_t blockSize) {
    assert(blockSize > 0);
    mBlockSize = blockSize;
    return *this;
}


spla::Library::Config &spla::Library::Config::SetWorkersCount(std::size_t workersCount) {
    assert(workersCount > 0);
    mWorkersCount = workersCount;
    return *this;
}

std::vector<boost::compute::device> GetDevices(const spla::Library::Config &config) {
    std::vector<boost::compute::device> devices;

    auto platforms = boost::compute::system::platforms();
    if (platforms.empty()) {
        RAISE_ERROR(DeviceNotPresent, "No OpenCL platform found");
    }

    if (!config.GetDeviceType().has_value() &&
        !config.GetPlatformName().has_value() &&
        config.GetDeviceAmount() == 1) {
        return {boost::compute::system::default_device()};
    }

    for (const boost::compute::platform &platform : platforms) {
        bool matchPlatform = !config.GetPlatformName().has_value() || platform.name().find(config.GetPlatformName().value()) != std::string::npos;

        if (!matchPlatform)
            continue;

        const auto &deviceType = config.GetDeviceType();

        for (const boost::compute::device &device : platform.devices()) {
            bool matchType = !deviceType.has_value() || ((deviceType.value() == spla::Library::Config::GPU && device.type() == boost::compute::device::type::gpu) ||
                                                         (deviceType.value() == spla::Library::Config::CPU && device.type() == boost::compute::device::type::cpu) ||
                                                         (deviceType.value() == spla::Library::Config::Accelerator && device.type() == boost::compute::device::type::accelerator));
            if (matchType) {
                devices.push_back(device);
            }
        }
    }

    if (config.GetDeviceAmount().has_value() && devices.size() > config.GetDeviceAmount().value()) {
        devices.resize(config.GetDeviceAmount().value());
    }

    return devices;
}

std::vector<spla::Library::ClDeviceId> spla::Library::Config::GetDevicesIds() const {
    std::vector<spla::Library::ClDeviceId> ids;
    for (const boost::compute::device &device : GetDevices(*this)) {
        ids.push_back(static_cast<void *>(device.id()));
    }
    return ids;
}

std::vector<std::string> spla::Library::Config::GetDevicesNames() const {
    std::vector<std::string> names;
    for (const boost::compute::device &device : GetDevices(*this)) {
        std::stringstream nameSs;
        nameSs << device.name() << ' ' << '(' << device.id() << ')';
        names.push_back(nameSs.str());
    }
    return names;
}

std::size_t spla::Library::Config::GetBlockSize() const {
    return mBlockSize;
}

std::size_t spla::Library::Config::GetWorkersCount() const {
    return !mWorkersCount ? std::thread::hardware_concurrency() : mWorkersCount;
}

const std::optional<spla::Filename> &spla::Library::Config::GetLogFilename() const {
    return mLogFilename;
}

std::optional<std::string> spla::Library::Config::GetPlatformName() const {
    return mPlatformName;
}

std::optional<spla::Library::Config::DeviceType> spla::Library::Config::GetDeviceType() const {
    return mDeviceType;
}

std::optional<std::size_t> spla::Library::Config::GetDeviceAmount() const {
    return mDeviceAmount;
}
