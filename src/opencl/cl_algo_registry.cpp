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

#include "cl_algo_registry.hpp"

#include <core/registry.hpp>
#include <core/top.hpp>

#include <opencl/cl_mxv.hpp>
#include <opencl/cl_vector_assign.hpp>
#include <opencl/cl_vector_reduce.hpp>
#include <opencl/cl_vector_select_count.hpp>
#include <opencl/cl_vxm.hpp>

namespace spla {

    void register_algo_cl(class Registry* g_registry) {
        g_registry->add(MAKE_KEY_CL_1("v_reduce", PLUS_INT), std::make_shared<Algo_v_reduce_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", PLUS_UINT), std::make_shared<Algo_v_reduce_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", PLUS_FLOAT), std::make_shared<Algo_v_reduce_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", MULT_INT), std::make_shared<Algo_v_reduce_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", MULT_UINT), std::make_shared<Algo_v_reduce_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", MULT_FLOAT), std::make_shared<Algo_v_reduce_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", BOR_INT), std::make_shared<Algo_v_reduce_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", BOR_UINT), std::make_shared<Algo_v_reduce_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", BAND_INT), std::make_shared<Algo_v_reduce_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_reduce", BAND_UINT), std::make_shared<Algo_v_reduce_cl<T_UINT>>());

        g_registry->add(MAKE_KEY_CL_1("v_select_count", NQZERO_INT), std::make_shared<Algo_v_select_count_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_select_count", NQZERO_UINT), std::make_shared<Algo_v_select_count_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_1("v_select_count", NQZERO_FLOAT), std::make_shared<Algo_v_select_count_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_1("v_select_count", EQZERO_INT), std::make_shared<Algo_v_select_count_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_1("v_select_count", EQZERO_UINT), std::make_shared<Algo_v_select_count_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_1("v_select_count", EQZERO_FLOAT), std::make_shared<Algo_v_select_count_cl<T_FLOAT>>());

        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", PLUS_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", PLUS_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", PLUS_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MINUS_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MINUS_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MINUS_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MULT_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MULT_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", MULT_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", DIV_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", DIV_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", DIV_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", SECOND_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", SECOND_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", SECOND_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", ONE_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", ONE_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", ONE_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BOR_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BOR_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BAND_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BAND_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BXOR_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_2("v_assign_masked", BXOR_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());

        g_registry->add(MAKE_KEY_CL_3("mxv_masked", BAND_INT, BOR_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", BAND_UINT, BOR_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MULT_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MULT_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MULT_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_mxv_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MIN_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MIN_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("mxv_masked", MIN_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_mxv_masked_cl<T_FLOAT>>());

        g_registry->add(MAKE_KEY_CL_3("vxm_masked", BAND_INT, BOR_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", BAND_UINT, BOR_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MULT_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MULT_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MULT_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_vxm_masked_cl<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MIN_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MIN_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_3("vxm_masked", MIN_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_vxm_masked_cl<T_FLOAT>>());
    }

}// namespace spla
