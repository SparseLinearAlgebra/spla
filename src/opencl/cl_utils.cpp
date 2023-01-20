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

#include "cl_utils.hpp"

#include <opencl/cl_program_builder.hpp>
#include <opencl/cl_program_cache.hpp>
#include <opencl/generated/auto_fill.hpp>
#include <opencl/generated/auto_sort_bitonic.hpp>
#include <opencl/generated/auto_vector_formats.hpp>

namespace spla {

    CLUtils::CLUtils() {
        acquire_state<T_INT>().type   = get_ttype<T_INT>().as<Type>();
        acquire_state<T_UINT>().type  = get_ttype<T_UINT>().as<Type>();
        acquire_state<T_FLOAT>().type = get_ttype<T_FLOAT>().as<Type>();
    }

    bool CLUtils::CachedState::SortBitonic::ensure(const ref_ptr<Type>& type) {
        if (program) return true;

        CLProgramBuilder builder;
        builder.set_key("sort_bitonic")
                .add_type("TYPE", type)
                .add_define("BITONIC_SORT_LOCAL_BUFFER_SIZE", local_size)
                .add_code(source_sort_bitonic);

        if (!builder.build()) return false;

        program = builder.get_program();
        local   = program->make_kernel("bitonic_sort_local");
        global  = program->make_kernel("bitonic_sort_global");
        return true;
    }
    bool CLUtils::CachedState::VectorFormats::ensure(const ref_ptr<Type>& type) {
        if (program) return true;

        CLProgramBuilder builder;
        builder.set_key("vector_format")
                .add_type("TYPE", type)
                .add_code(source_vector_formats);

        if (!builder.build()) return false;

        program         = builder.get_program();
        sparse_to_dense = program->make_kernel("sparse_to_dense");
        dense_to_sparse = program->make_kernel("dense_to_sparse");

        return true;
    }
    bool CLUtils::CachedState::Fill::ensure(const ref_ptr<Type>& type) {
        if (program) return true;

        CLProgramBuilder builder;
        builder.set_key("fill")
                .add_type("TYPE", type)
                .add_code(source_fill);

        if (!builder.build()) return false;

        program    = builder.get_program();
        fill_zero  = program->make_kernel("fill_zero");
        fill_value = program->make_kernel("fill_value");

        return true;
    }

}// namespace spla
