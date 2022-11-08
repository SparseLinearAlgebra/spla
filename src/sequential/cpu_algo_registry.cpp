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

#include "cpu_algo_registry.hpp"

#include <core/registry.hpp>
#include <core/top.hpp>

#include <sequential/cpu_algo_callback.hpp>
#include <sequential/cpu_mxv.hpp>
#include <sequential/cpu_vector_assign.hpp>
#include <sequential/cpu_vector_reduce.hpp>
#include <sequential/cpu_vxm.hpp>

namespace spla {

    void register_algo_cpu(Registry* g_registry) {
        g_registry->add("callback" CPU_SUFFIX, std::make_shared<Algo_callback>());

        g_registry->add(MAKE_KEY_CPU_1("v_reduce", PLUS_INT), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", PLUS_UINT), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", PLUS_FLOAT), std::make_shared<Algo_v_reduce_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", MULT_INT), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", MULT_UINT), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", MULT_FLOAT), std::make_shared<Algo_v_reduce_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", BOR_INT), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", BOR_UINT), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", BAND_INT), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_1("v_reduce", BAND_UINT), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());

        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", PLUS_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", PLUS_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", PLUS_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MINUS_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MINUS_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MINUS_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MULT_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MULT_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", MULT_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", DIV_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", DIV_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", DIV_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", SECOND_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", SECOND_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", SECOND_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", ONE_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", ONE_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", ONE_FLOAT, NQZERO_FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BOR_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BOR_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BAND_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BAND_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BXOR_INT, NQZERO_INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", BXOR_UINT, NQZERO_UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());

        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", BAND_INT, BOR_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", BAND_UINT, BOR_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MULT_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MULT_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MULT_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_mxv_masked<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MIN_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_mxv_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MIN_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_mxv_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("mxv_masked", MIN_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_mxv_masked<T_FLOAT>>());

        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", BAND_INT, BOR_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", BAND_UINT, BOR_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MULT_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MULT_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MULT_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_vxm_masked<T_FLOAT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MIN_INT, PLUS_INT, EQZERO_INT), std::make_shared<Algo_vxm_masked<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MIN_UINT, PLUS_UINT, EQZERO_UINT), std::make_shared<Algo_vxm_masked<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_3("vxm_masked", MIN_FLOAT, PLUS_FLOAT, EQZERO_FLOAT), std::make_shared<Algo_vxm_masked<T_FLOAT>>());
    }

}// namespace spla