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

#ifndef SPLA_CL_PROGRAM_HPP
#define SPLA_CL_PROGRAM_HPP

#include <opencl/cl_accelerator.hpp>

#include <svector.hpp>

#include <string>
#include <vector>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CLProgram
     * @brief Compiled opencl program from library sources
     */
    class CLProgram {
    public:
        cl::Kernel make_kernel(const char* name);

        [[nodiscard]] const ankerl::svector<std::string, 8>& get_defines() const { return m_defines; }
        [[nodiscard]] const ankerl::svector<std::string, 8>& get_functions() const { return m_functions; }
        [[nodiscard]] const ankerl::svector<std::string, 2>& get_sources() const { return m_sources; }
        [[nodiscard]] const std::string&                     get_source() const { return m_source; }
        [[nodiscard]] const std::string&                     get_name() const { return m_name; }
        [[nodiscard]] const std::string&                     get_key() const { return m_key; }
        [[nodiscard]] const cl::Program&                     get_program() const { return m_program; }

    private:
        friend class CLProgramBuilder;

        ankerl::svector<std::string, 8> m_defines;
        ankerl::svector<std::string, 8> m_functions;
        ankerl::svector<std::string, 2> m_sources;
        std::string                     m_source;
        std::string                     m_name;
        std::string                     m_key;
        cl::Program                     m_program;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_PROGRAM_HPP
