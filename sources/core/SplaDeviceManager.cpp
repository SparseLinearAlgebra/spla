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
#include <spla-cpp/SplaExpression.hpp>

namespace spla {
    namespace {
        bool FetchSpecifiedDevice(Descriptor &desc, DeviceManager::DeviceId &id) {
            return desc.GetParamT(Descriptor::Param::DeviceId, id);
        }
        bool FetchSpecifiedDevice(const RefPtr<ExpressionNode> &node, DeviceManager::DeviceId &id) {
            return FetchSpecifiedDevice(*node->GetDescriptor(), id) ||
                   FetchSpecifiedDevice(*node->GetParent()->GetDescriptor(), id);
        }
        bool FixedAllocationStrategy(const RefPtr<ExpressionNode> &node) {
            return node->GetDescriptor()->IsParamSet(Descriptor::Param::DeviceFixedStrategy) ||
                   node->GetParent()->GetDescriptor()->IsParamSet(Descriptor::Param::DeviceFixedStrategy);
        }
    }// namespace
}// namespace spla

spla::DeviceManager::DeviceId spla::DeviceManager::FetchDevice(const spla::RefPtr<spla::ExpressionNode> &node) {
    std::lock_guard<std::mutex> lockGuard(mMutex);

    assert(node);
    auto desc = node->GetDescriptor();
    assert(desc);

    DeviceId id;

    auto fixedAllocationStrategy = FixedAllocationStrategy(node);// Try to get allocation strategy
    auto fetched = FetchSpecifiedDevice(node, id);               // Try to fetch device id from desc

    if (fetched && id < mDevices.size())
        return id;
    else if (fixedAllocationStrategy)
        return 0;
    else
        return NextDevice();
}

std::vector<spla::DeviceManager::DeviceId> spla::DeviceManager::FetchDevices(std::size_t required, const spla::RefPtr<spla::ExpressionNode> &node) {
    std::lock_guard<std::mutex> lockGuard(mMutex);

    if (!required)
        return {};

    assert(node);
    auto desc = node->GetDescriptor();
    assert(desc);

    DeviceId id;
    std::vector<DeviceId> result;

    auto fixedAllocationStrategy = FixedAllocationStrategy(node);// Try to get allocation strategy
    auto fetched = FetchSpecifiedDevice(node, id);               // Try to fetch device id from desc

    if (fetched && id < mDevices.size())
        result.resize(required, id);
    else if (fixedAllocationStrategy) {
        // FIll devices starting from 0
        result.reserve(required);
        for (std::size_t i = 0; i < required; i++)
            result.push_back(i % mDevices.size());
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