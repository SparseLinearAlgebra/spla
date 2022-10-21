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

#ifndef SPLA_REGISTRY_HPP
#define SPLA_REGISTRY_HPP

#include <spla/config.hpp>
#include <spla/schedule.hpp>

#include <string>
#include <unordered_map>

namespace spla {

#define CPU_SUFFIX                             "__cpu"
#define GPU_SUFFIX                             "__gpu"
#define GPU_CL_SUFFIX                          "__cl"
#define MASK_COMPLEMENT                        "_mc"
#define MASK_DIRECT                            "_md"
#define MASK_KEY(complement)                   ((complement) ? MASK_COMPLEMENT : MASK_DIRECT)
#define OP_KEY(op)                             "_" + (op)->get_key()
#define MAKE_KEY_1(name, op)                   std::string(name) + OP_KEY(op)
#define MAKE_KEY_2(name, op1, op2)             std::string(name) + OP_KEY(op1) + OP_KEY(op2)
#define MAKE_KEY_2_M(name, op1, op2, comp)     std::string(name) + OP_KEY(op1) + OP_KEY(op2) + MASK_KEY(comp)
#define MAKE_KEY_CPU_1(name, op)               MAKE_KEY_1(name, op) + CPU_SUFFIX
#define MAKE_KEY_CPU_2(name, op1, op2)         MAKE_KEY_2(name, op1, op2) + CPU_SUFFIX
#define MAKE_KEY_CPU_2_M(name, op1, op2, comp) MAKE_KEY_2_M(name, op1, op2, comp) + CPU_SUFFIX
#define MAKE_KEY_CL_1(name, op)                MAKE_KEY_1(name, op) + GPU_CL_SUFFIX
#define MAKE_KEY_CL_2(name, op1, op2)          MAKE_KEY_2(name, op1, op2) + GPU_CL_SUFFIX
#define MAKE_KEY_CL_2_M(name, op1, op2, comp)  MAKE_KEY_2_M(name, op1, op2, comp) + GPU_CL_SUFFIX

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class RegistryAlgo
     * @brief Algorithm suitable to process schedule task based on task string key
     */
    class RegistryAlgo {
    public:
        virtual ~RegistryAlgo()                                       = default;
        virtual std::string get_name()                                = 0;
        virtual std::string get_description()                         = 0;
        virtual Status      execute(const class DispatchContext& ctx) = 0;
    };

    /**
     * @class Registry
     * @brief Registry with key-algo mapping of stored algo implementations
     */
    class Registry {
    public:
        virtual ~Registry() = default;
        virtual void                          add(const std::string& key, std::shared_ptr<RegistryAlgo> algo);
        virtual bool                          has(const std::string& key);
        virtual std::shared_ptr<RegistryAlgo> find(const std::string& key);

    private:
        std::unordered_map<std::string, std::shared_ptr<RegistryAlgo>> m_registry;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_REGISTRY_HPP