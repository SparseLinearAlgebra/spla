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

#include "dispatcher.hpp"

#include <core/accelerator.hpp>
#include <core/logger.hpp>
#include <core/registry.hpp>

namespace spla {

    Status Dispatcher::dispatch(const DispatchContext& ctx) {
        Registry*    g_reg = get_registry();
        Accelerator* g_acc = get_accelerator();

        std::shared_ptr<RegistryAlgo> algo;
        std::string                   key = ctx.task->get_key();

        if (g_acc) {
            std::string key_acc = key + g_acc->get_suffix();
            algo                = g_reg->find(key_acc);

            if (algo) {
                LOG_MSG(Status::Ok, "found acc algo " << algo->get_name());
                return algo->execute(ctx);
            }
        }

        std::string key_cpu = key + CPU_SUFFIX;
        algo                = g_reg->find(key_cpu);

        if (algo) {
            LOG_MSG(Status::Ok, "found cpu algo " << algo->get_name());
            return algo->execute(ctx);
        }

        LOG_MSG(Status::NotImplemented, "failed to find suitable algo for key " << key);

        return Status::NotImplemented;
    }

}// namespace spla