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

#include "cl_algo_registry.hpp"

#include <core/registry.hpp>
#include <core/top.hpp>

#include <opencl/cl_vector_reduce.hpp>

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
    }

}// namespace spla
