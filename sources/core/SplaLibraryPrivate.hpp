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

#ifndef SPLA_SPLALIBRARYPRIVATE_HPP
#define SPLA_SPLALIBRARYPRIVATE_HPP

#include <algo/SplaAlgorithmManager.hpp>
#include <boost/compute/device.hpp>
#include <boost/compute/system.hpp>
#include <core/SplaDeviceManager.hpp>
#include <expression/SplaExpressionManager.hpp>
#include <spdlog/spdlog.h>
#include <spla-cpp/SplaDescriptor.hpp>
#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaType.hpp>
#include <spla-cpp/SplaTypes.hpp>
#include <taskflow/taskflow.hpp>
#include <unordered_map>
#include <vector>


namespace spla {
    /**
     * @class LibraryPrivate
     * Private library state, accessible for all objects within library.
     */
    class LibraryPrivate {
    public:
        explicit LibraryPrivate(Library &library, Library::Config config);

        tf::Executor &GetTaskFlowExecutor() noexcept;

        const RefPtr<Descriptor> &GetDefaultDesc() const noexcept;

        const RefPtr<ExpressionManager> &GetExprManager() const noexcept;

        const RefPtr<AlgorithmManager> &GetAlgoManager() const noexcept;

        // todo: #44 remove this method
        const std::vector<boost::compute::device> &GetDevices() const noexcept;

        DeviceManager &GetDeviceManager() noexcept;

        const boost::compute::platform &GetPlatform() const noexcept;

        const boost::compute::context &GetContext() const noexcept;

        const Library::Config &GetContextConfig() const noexcept;

        const std::shared_ptr<spdlog::logger> &GetLogger() const noexcept;

        std::unordered_map<std::string, RefPtr<Type>> &GetTypeCache() noexcept;

        std::size_t GetBlockSize() const noexcept;

    private:
        tf::Executor mExecutor;
        RefPtr<Descriptor> mDefaultDesc;
        RefPtr<ExpressionManager> mExprManager;
        RefPtr<AlgorithmManager> mAlgoManager;
        DeviceManager mDeviceManager;
        boost::compute::platform mPlatform;
        boost::compute::context mContext;
        Library::Config mContextConfig;
        std::shared_ptr<spdlog::logger> mLogger;
        std::unordered_map<std::string, RefPtr<Type>> mTypeCache;
    };

}// namespace spla

#endif//SPLA_SPLALIBRARYPRIVATE_HPP