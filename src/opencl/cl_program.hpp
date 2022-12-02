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

#ifndef SPLA_CL_PROGRAM_HPP
#define SPLA_CL_PROGRAM_HPP

#include <opencl/cl_accelerator.hpp>

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

        [[nodiscard]] const std::vector<std::string>& get_defines() const { return m_defines; }
        [[nodiscard]] const std::vector<std::string>& get_functions() const { return m_functions; }
        [[nodiscard]] const std::vector<std::string>& get_sources() const { return m_sources; }
        [[nodiscard]] const std::string&              get_source() const { return m_source; }
        [[nodiscard]] const std::string&              get_key() const { return m_key; }
        [[nodiscard]] const cl::Program&              get_program() const { return m_program; }

    private:
        friend class CLProgramBuilder;

        std::vector<std::string> m_defines;
        std::vector<std::string> m_functions;
        std::vector<std::string> m_sources;
        std::string              m_source;
        std::string              m_key;
        cl::Program              m_program;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_PROGRAM_HPP
