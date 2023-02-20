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
#include <opencl/cl_vector_count_mf.hpp>
#include <opencl/cl_vector_eadd_fdb.hpp>
#include <opencl/cl_vector_map.hpp>
#include <opencl/cl_vector_reduce.hpp>
#include <opencl/cl_vxm.hpp>

namespace spla {

    void register_algo_cl(class Registry* g_registry) {
        // algorthm v_count_mf
        g_registry->add(MAKE_KEY_CL_0("v_count_mf", INT), std::make_shared<Algo_v_count_mf_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("v_count_mf", UINT), std::make_shared<Algo_v_count_mf_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("v_count_mf", FLOAT), std::make_shared<Algo_v_count_mf_cl<T_FLOAT>>());

        // algorthm v_map
        g_registry->add(MAKE_KEY_CL_0("v_map", INT), std::make_shared<Algo_v_map_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("v_map", UINT), std::make_shared<Algo_v_map_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("v_map", FLOAT), std::make_shared<Algo_v_map_cl<T_FLOAT>>());

        // algorthm v_reduce
        g_registry->add(MAKE_KEY_CL_0("v_reduce", INT), std::make_shared<Algo_v_reduce_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("v_reduce", UINT), std::make_shared<Algo_v_reduce_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("v_reduce", FLOAT), std::make_shared<Algo_v_reduce_cl<T_FLOAT>>());

        // algorthm v_eadd_fdb
        g_registry->add(MAKE_KEY_CL_0("v_eadd_fdb", INT), std::make_shared<Algo_v_eadd_fdb_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("v_eadd_fdb", UINT), std::make_shared<Algo_v_eadd_fdb_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("v_eadd_fdb", FLOAT), std::make_shared<Algo_v_eadd_fdb_cl<T_FLOAT>>());

        // algorthm v_assign_masked
        g_registry->add(MAKE_KEY_CL_0("v_assign_masked", INT), std::make_shared<Algo_v_assign_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("v_assign_masked", UINT), std::make_shared<Algo_v_assign_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("v_assign_masked", FLOAT), std::make_shared<Algo_v_assign_masked_cl<T_FLOAT>>());

        // algorthm mxv_masked
        g_registry->add(MAKE_KEY_CL_0("mxv_masked", INT), std::make_shared<Algo_mxv_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("mxv_masked", UINT), std::make_shared<Algo_mxv_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("mxv_masked", FLOAT), std::make_shared<Algo_mxv_masked_cl<T_FLOAT>>());

        // algorthm vxm_masked
        g_registry->add(MAKE_KEY_CL_0("vxm_masked", INT), std::make_shared<Algo_vxm_masked_cl<T_INT>>());
        g_registry->add(MAKE_KEY_CL_0("vxm_masked", UINT), std::make_shared<Algo_vxm_masked_cl<T_UINT>>());
        g_registry->add(MAKE_KEY_CL_0("vxm_masked", FLOAT), std::make_shared<Algo_vxm_masked_cl<T_FLOAT>>());
    }

}// namespace spla
