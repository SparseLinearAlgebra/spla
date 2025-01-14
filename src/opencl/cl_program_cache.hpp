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

#ifndef SPLA_CL_PROGRAM_CACHE_HPP
#define SPLA_CL_PROGRAM_CACHE_HPP

#include <opencl/cl_accelerator.hpp>
#include <opencl/cl_program.hpp>

#include <robin_hood.hpp>

#include <string>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class CLProgramCache
     * @brief Runtime cache for compiled opencl programs
     */
    class CLProgramCache {
    public:
        void                       add_program(const std::shared_ptr<CLProgram>& program);
        std::shared_ptr<CLProgram> get_program(const std::string& source);

    private:
        robin_hood::unordered_flat_map<std::string, std::shared_ptr<CLProgram>> m_programs;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_CL_PROGRAM_CACHE_HPP
