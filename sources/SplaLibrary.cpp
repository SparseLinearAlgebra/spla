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

#include <boost/compute/device.hpp>
#include <boost/compute/system.hpp>
#include <detail/SplaError.hpp>
#include <detail/SplaLibraryPrivate.hpp>
#include <memory>
#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaTypes.hpp>
#include <sstream>

spla::Library::Library(Config config) : mPrivate(std::make_shared<LibraryPrivate>(*this, std::move(config))) {
    Types::RegisterTypes(*this);
}

spla::Library::Library() : Library(Config()) {
}

spla::Library::~Library() = default;

void spla::Library::Submit(const spla::RefPtr<spla::Expression> &expression) const {
    GetPrivate().GetExprManager()->Submit(expression);
}

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

spla::Library::Config &spla::Library::Config::SetBlockSize(size_t blockSize) {
    assert(blockSize > 0);
    mBlockSize = blockSize;
    return *this;
}

std::vector<std::string> spla::Library::Config::GetDevicesNames() const {
    std::vector<std::string> devicesNames;

    if (!mDeviceType.has_value() &&
        !mPlatformName.has_value() &&
        mDeviceAmount.has_value() &&
        mDeviceAmount.value() == 1U) {
        return {boost::compute::system::default_device().name()};
    }

    bool platformExists = false;
    for (const boost::compute::platform &platform : boost::compute::system::platforms()) {
        if (mPlatformName.has_value() && platform.name() != mPlatformName.value()) {
            continue;
        }
        for (const boost::compute::device &device : platform.devices()) {
            bool matchType = !mDeviceType.has_value() || ((mDeviceType.value() == GPU && device.type() == boost::compute::device::type::gpu) ||
                                                          (mDeviceType.value() == CPU && device.type() == boost::compute::device::type::cpu) ||
                                                          (mDeviceType.value() == Accelerator && device.type() == boost::compute::device::type::accelerator));
            bool matchPlatform = !mPlatformName.has_value() || device.platform().name() == mPlatformName.value();
            if (matchType && matchPlatform) {
                devicesNames.push_back(device.name());
            }
        }
    }
    if (mPlatformName.has_value() && !platformExists) {
        RAISE_ERROR(DeviceNotPresent, ("No OpenCL platform found with name '" + mPlatformName.value() + "'").c_str());
    }
    if (mDeviceAmount.has_value() && devicesNames.size() > mDeviceAmount.value()) {
        devicesNames.resize(mDeviceAmount.value());
    }
    return devicesNames;
}

size_t spla::Library::Config::GetBlockSize() const {
    return mBlockSize;
}

const std::optional<spla::Filename> &spla::Library::Config::GetLogFilename() const {
    return mLogFilename;
}
