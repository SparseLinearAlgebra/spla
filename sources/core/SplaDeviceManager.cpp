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

#include <core/SplaDeviceManager.hpp>

namespace spla {
    namespace {
        bool FetchSpecifiedDevice(Descriptor &desc, DeviceManager::DeviceId &id) {
            if (desc.IsParamSet(Descriptor::Param::DeviceId0))
                id = 0;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId1))
                id = 1;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId2))
                id = 2;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId3))
                id = 3;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId4))
                id = 4;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId5))
                id = 5;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId6))
                id = 6;
            else if (desc.IsParamSet(Descriptor::Param::DeviceId7))
                id = 7;
            else
                return false;

            return true;
        }
    }// namespace
}// namespace spla

spla::DeviceManager::DeviceId spla::DeviceManager::FetchDevice(const spla::RefPtr<spla::ExpressionNode> &node) {
    std::lock_guard<std::mutex> lockGuard(mMutex);

    assert(node);
    auto desc = node->GetDescriptor();
    assert(desc);

    DeviceId id;

    // Try to fetch device id from desc
    auto fetched = FetchSpecifiedDevice(*desc, id);

    // If it does not fetched or specified id is too large, we can simply use next suitable
    if (!fetched || id >= mDevices.size())
        id = NextDevice();

    return id;
}

std::vector<spla::DeviceManager::DeviceId> spla::DeviceManager::FetchDevices(std::size_t required, const spla::RefPtr<spla::ExpressionNode> &node) {
    std::lock_guard<std::mutex> lockGuard(mMutex);

    if (!required)
        return std::vector<DeviceId>();

    assert(node);
    auto desc = node->GetDescriptor();
    assert(desc);

    DeviceId id;
    std::vector<DeviceId> result;

    // Try to fetch device id from desc
    auto fetched = FetchSpecifiedDevice(*desc, id);

    // If fetched, single device is used for all required tasks
    if (fetched) {
        // If specified id is too large, we can simply use next suitable
        if (id >= mDevices.size())
            id = NextDevice();
        result.resize(required, id);
    } else {
        // Use even work distribution for all tasks
        result.reserve(required);
        for (std::size_t i = 0; i < required; i++)
            result.push_back(NextDevice());
    }

    return result;
}

const spla::DeviceManager::Device &spla::DeviceManager::GetDevice(spla::DeviceManager::DeviceId id) const {
    assert(id < mDevices.size());
    return mDevices[id];
}

const std::vector<spla::DeviceManager::Device> &spla::DeviceManager::GetDevices() const {
    return mDevices;
}

spla::DeviceManager::DeviceManager(std::vector<Device> devices) : mDevices(std::move(devices)) {
    assert(!mDevices.empty());
}

spla::DeviceManager::DeviceId spla::DeviceManager::NextDevice() {
    auto next = mNextDevice;
    mNextDevice = (mNextDevice + 1) % mDevices.size();
    return next;
}