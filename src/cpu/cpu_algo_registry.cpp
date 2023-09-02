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

#include "cpu_algo_registry.hpp"

#include <core/registry.hpp>
#include <core/top.hpp>

#include <cpu/cpu_algo_callback.hpp>
#include <cpu/cpu_kron.hpp>
#include <cpu/cpu_m_eadd.hpp>
#include <cpu/cpu_m_emult.hpp>
#include <cpu/cpu_m_reduce.hpp>
#include <cpu/cpu_m_reduce_by_column.hpp>
#include <cpu/cpu_m_reduce_by_row.hpp>
#include <cpu/cpu_m_transpose.hpp>
#include <cpu/cpu_mxm.hpp>
#include <cpu/cpu_mxmT_masked.hpp>
#include <cpu/cpu_mxv.hpp>
#include <cpu/cpu_v_assign.hpp>
#include <cpu/cpu_v_count_mf.hpp>
#include <cpu/cpu_v_eadd.hpp>
#include <cpu/cpu_v_eadd_fdb.hpp>
#include <cpu/cpu_v_emult.hpp>
#include <cpu/cpu_v_map.hpp>
#include <cpu/cpu_v_reduce.hpp>
#include <cpu/cpu_vxm.hpp>

namespace spla {

    void register_algo_cpu(Registry* g_registry) {
        // algorthm callback
        g_registry->add("callback" CPU_SUFFIX, std::make_shared<Algo_callback_cpu>());

        // algorthm v_count_mf
        g_registry->add(MAKE_KEY_CPU_0("v_count_mf", INT), std::make_shared<Algo_v_count_mf_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_count_mf", UINT), std::make_shared<Algo_v_count_mf_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_count_mf", FLOAT), std::make_shared<Algo_v_count_mf_cpu<T_FLOAT>>());

        // algorthm v_map
        g_registry->add(MAKE_KEY_CPU_0("v_map", INT), std::make_shared<Algo_v_map_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_map", UINT), std::make_shared<Algo_v_map_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_map", FLOAT), std::make_shared<Algo_v_map_cpu<T_FLOAT>>());

        // algorthm v_reduce
        g_registry->add(MAKE_KEY_CPU_0("v_reduce", INT), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_reduce", UINT), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_reduce", FLOAT), std::make_shared<Algo_v_reduce_cpu<T_FLOAT>>());

        // algorthm v_eadd
        g_registry->add(MAKE_KEY_CPU_0("v_eadd", INT), std::make_shared<Algo_v_eadd_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_eadd", UINT), std::make_shared<Algo_v_eadd_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_eadd", FLOAT), std::make_shared<Algo_v_eadd_cpu<T_FLOAT>>());

        // algorthm v_emult
        g_registry->add(MAKE_KEY_CPU_0("v_emult", INT), std::make_shared<Algo_v_emult_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_emult", UINT), std::make_shared<Algo_v_emult_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_emult", FLOAT), std::make_shared<Algo_v_emult_cpu<T_FLOAT>>());

        // algorthm v_eadd_fdb
        g_registry->add(MAKE_KEY_CPU_0("v_eadd_fdb", INT), std::make_shared<Algo_v_eadd_fdb_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_eadd_fdb", UINT), std::make_shared<Algo_v_eadd_fdb_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_eadd_fdb", FLOAT), std::make_shared<Algo_v_eadd_fdb_cpu<T_FLOAT>>());

        // algorthm v_assign_masked
        g_registry->add(MAKE_KEY_CPU_0("v_assign_masked", INT), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_assign_masked", UINT), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("v_assign_masked", FLOAT), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());

        // algorthm m_reduce_by_row
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_row", INT), std::make_shared<Algo_m_reduce_by_row_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_row", UINT), std::make_shared<Algo_m_reduce_by_row_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_row", FLOAT), std::make_shared<Algo_m_reduce_by_row_cpu<T_FLOAT>>());

        // algorthm m_reduce_by_column
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_column", INT), std::make_shared<Algo_m_reduce_by_column_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_column", UINT), std::make_shared<Algo_m_reduce_by_column_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce_by_column", FLOAT), std::make_shared<Algo_m_reduce_by_column_cpu<T_FLOAT>>());

        // algorthm m_reduce
        g_registry->add(MAKE_KEY_CPU_0("m_reduce", INT), std::make_shared<Algo_m_reduce_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce", UINT), std::make_shared<Algo_m_reduce_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_reduce", FLOAT), std::make_shared<Algo_m_reduce_cpu<T_FLOAT>>());

        // algorthm m_eadd
        g_registry->add(MAKE_KEY_CPU_0("m_eadd", INT), std::make_shared<Algo_m_eadd_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_eadd", UINT), std::make_shared<Algo_m_eadd_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_eadd", FLOAT), std::make_shared<Algo_m_eadd_cpu<T_FLOAT>>());

        // algorthm m_emult
        g_registry->add(MAKE_KEY_CPU_0("m_emult", INT), std::make_shared<Algo_m_emult_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_emult", UINT), std::make_shared<Algo_m_emult_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_emult", FLOAT), std::make_shared<Algo_m_emult_cpu<T_FLOAT>>());

        // algorthm m_transpose
        g_registry->add(MAKE_KEY_CPU_0("m_transpose", INT), std::make_shared<Algo_m_transpose_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_transpose", UINT), std::make_shared<Algo_m_transpose_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("m_transpose", FLOAT), std::make_shared<Algo_m_transpose_cpu<T_FLOAT>>());

        // algorthm mxv_masked
        g_registry->add(MAKE_KEY_CPU_0("mxv_masked", INT), std::make_shared<Algo_mxv_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxv_masked", UINT), std::make_shared<Algo_mxv_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxv_masked", FLOAT), std::make_shared<Algo_mxv_masked_cpu<T_FLOAT>>());

        // algorthm vxm_masked
        g_registry->add(MAKE_KEY_CPU_0("vxm_masked", INT), std::make_shared<Algo_vxm_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("vxm_masked", UINT), std::make_shared<Algo_vxm_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("vxm_masked", FLOAT), std::make_shared<Algo_vxm_masked_cpu<T_FLOAT>>());

        // algorthm kron
        g_registry->add(MAKE_KEY_CPU_0("kron", INT), std::make_shared<Algo_kron_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("kron", UINT), std::make_shared<Algo_kron_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("kron", FLOAT), std::make_shared<Algo_kron_cpu<T_FLOAT>>());

        // algorthm mxmT_masked
        g_registry->add(MAKE_KEY_CPU_0("mxmT_masked", INT), std::make_shared<Algo_mxmT_masked_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxmT_masked", UINT), std::make_shared<Algo_mxmT_masked_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxmT_masked", FLOAT), std::make_shared<Algo_mxmT_masked_cpu<T_FLOAT>>());

        // algorthm mxm
        g_registry->add(MAKE_KEY_CPU_0("mxm", INT), std::make_shared<Algo_mxm_cpu<T_INT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxm", UINT), std::make_shared<Algo_mxm_cpu<T_UINT>>());
        g_registry->add(MAKE_KEY_CPU_0("mxm", FLOAT), std::make_shared<Algo_mxm_cpu<T_FLOAT>>());
    }

}// namespace spla