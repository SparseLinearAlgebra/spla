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

#ifndef SPLA_CL_KERNEL_BUILDER_HPP
#define SPLA_CL_KERNEL_BUILDER_HPP

#include <core/top.hpp>
#include <core/ttype.hpp>
#include <opencl/cl_accelerator.hpp>

#include <sstream>
#include <string>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CLKernelBuilder
     * @brief Runtime opencl program builder
     */
    class CLKernelBuilder final {
    public:
        CLKernelBuilder& add_define(const char* define, int value);
        CLKernelBuilder& add_type(const char* alias, const ref_ptr<Type>& type);
        CLKernelBuilder& add_op(const char* name, const ref_ptr<OpBinary>& op);
        CLKernelBuilder& add_op(const char* name, const ref_ptr<OpSelect>& op);
        CLKernelBuilder& add_code(const char* source);

        bool build();

        const std::string& get_source() { return m_source; };
        const cl::Program& get_program() { return m_program; };

    private:
        std::vector<std::string> m_defines;
        std::vector<std::string> m_functions;
        std::vector<std::string> m_sources;
        std::string              m_source;
        cl::Program              m_program;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_KERNEL_BUILDER_HPP
