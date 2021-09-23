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

#include <detail/SplaLibraryPrivate.hpp>
#include <detail/SplaError.hpp>
#include <expression/SplaExpressionManager.hpp>

namespace {
    std::vector<boost::compute::device> FindAllDevices(const std::vector<std::string> &names) {
        std::vector<boost::compute::device> foundDevices;
        for (const std::string &name : names) {
            try {
                foundDevices.push_back(boost::compute::system::find_device(name));
            } catch (const boost::compute::no_device_found&) {
                RAISE_ERROR(DeviceNotPresent, ("Device does not exist: '" + name + "'").c_str())
            }
        }
        return foundDevices;
    }

    boost::compute::platform GetDevicesPlatform(const std::vector<boost::compute::device> &devices) {
        if (devices.empty()) {
            RAISE_ERROR(DeviceNotPresent, "No device was found");
        }
        auto platformId = devices[0].platform().id();
        for (const auto &device : devices) {
            if (platformId != device.platform().id()) {
                RAISE_ERROR(DeviceError, "Could not initialize context when devices has different platforms");
            }
        }
        return devices[0].platform();
    }
}

tf::Executor &spla::LibraryPrivate::GetTaskFlowExecutor() {
    return mExecutor;
}

const spla::RefPtr<spla::Descriptor> &spla::LibraryPrivate::GetDefaultDesc() {
    return mDefaultDesc;
}

const spla::RefPtr<spla::ExpressionManager> &spla::LibraryPrivate::GetExprManager() {
    return mExprManager;
}

const std::vector<boost::compute::device> &spla::LibraryPrivate::GetDevices() const noexcept {
    return mDevices;
}

const boost::compute::platform &spla::LibraryPrivate::GetPlatform() const noexcept {
    return mPlatform;
}

const boost::compute::context &spla::LibraryPrivate::GetContext() const noexcept {
    return mContext;
}

spla::LibraryPrivate::LibraryPrivate(
    spla::Library &library,
    const spla::Library::Config &config
) : mDefaultDesc(Descriptor::Make(library)),
    mExprManager(RefPtr<ExpressionManager>(new ExpressionManager(library))),
    mDevices(FindAllDevices(config.GetDevicesNames())),
    mPlatform(GetDevicesPlatform(mDevices)),
    mContext(mDevices) {
}
