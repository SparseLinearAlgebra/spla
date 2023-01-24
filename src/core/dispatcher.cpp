/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#include "dispatcher.hpp"

#include <core/accelerator.hpp>
#include <core/logger.hpp>
#include <core/registry.hpp>

#include <cstdlib>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
#endif

namespace spla {

    Status Dispatcher::dispatch(const DispatchContext& ctx) {
        Library*     g_lib        = get_library();
        Registry*    g_reg        = g_lib->get_registry();
        Accelerator* g_acc        = g_lib->get_accelerator();
        bool         force_no_acc = g_lib->is_set_force_no_acceleration();

        std::shared_ptr<RegistryAlgo> algo;
        std::string                   key = ctx.task->get_key();

        if (g_acc && !force_no_acc) {
            std::string key_acc = key + g_acc->get_suffix();
            algo                = g_reg->find(key_acc);
        }

        if (!algo) {
            std::string key_cpu = key + CPU_SUFFIX;
            algo                = g_reg->find(key_cpu);
        }

        if (algo) {
            try {
                return algo->execute(ctx);
            }
#if defined(SPLA_BUILD_OPENCL) && defined(CL_HPP_ENABLE_EXCEPTIONS)
            catch (const cl::BuildError& cl_ex) {
                LOG_MSG(Status::Error, "not handled cl exception thrown: " << cl_ex.getBuildLog().front().second);
    #ifndef SPLA_RELEASE
                std::abort();
    #endif
                return Status::Error;
            }
#endif
            catch (const std::exception& ex) {
                LOG_MSG(Status::Error, "not handled exception thrown: " << ex.what());
#ifndef SPLA_RELEASE
                std::abort();
#endif
                return Status::Error;
            }
        }

        LOG_MSG(Status::NotImplemented, "failed to find suitable algo for key " << key);
        return Status::NotImplemented;
    }

}// namespace spla