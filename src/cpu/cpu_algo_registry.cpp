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
#include <cpu/cpu_mxv.hpp>
#include <cpu/cpu_vector_assign.hpp>
#include <cpu/cpu_vector_reduce.hpp>
#include <cpu/cpu_vxm.hpp>

namespace spla {

    void register_algo_cpu(Registry* g_registry) {
        g_registry->add("callback" CPU_SUFFIX, std::make_shared<Algo_callback_cpu>());

        // algorthm v_reduce
        for (const auto& op0 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
            g_registry->add(MAKE_KEY_CPU_1("v_reduce", op0), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        }
        for (const auto& op0 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
            g_registry->add(MAKE_KEY_CPU_1("v_reduce", op0), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        }
        for (const auto& op0 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
            g_registry->add(MAKE_KEY_CPU_1("v_reduce", op0), std::make_shared<Algo_v_reduce_cpu<T_FLOAT>>());
        }
        for (const auto& op0 : {BOR_INT, BAND_INT, BXOR_INT}) {
            g_registry->add(MAKE_KEY_CPU_1("v_reduce", op0), std::make_shared<Algo_v_reduce_cpu<T_INT>>());
        }
        for (const auto& op0 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
            g_registry->add(MAKE_KEY_CPU_1("v_reduce", op0), std::make_shared<Algo_v_reduce_cpu<T_UINT>>());
        }

        // algorthm v_assign_masked
        for (const auto& op0 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
            for (const auto& op1 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", op0, op1), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
            }
        }
        for (const auto& op0 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
            for (const auto& op1 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", op0, op1), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
            }
        }
        for (const auto& op0 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
            for (const auto& op1 : {EQZERO_FLOAT, NQZERO_FLOAT, GTZERO_FLOAT, GEZERO_FLOAT, LTZERO_FLOAT, LEZERO_FLOAT, ALWAYS_FLOAT, NEVER_FLOAT}) {
                g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", op0, op1), std::make_shared<Algo_v_assign_masked_cpu<T_FLOAT>>());
            }
        }
        for (const auto& op0 : {BOR_INT, BAND_INT, BXOR_INT}) {
            for (const auto& op1 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", op0, op1), std::make_shared<Algo_v_assign_masked_cpu<T_INT>>());
            }
        }
        for (const auto& op0 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
            for (const auto& op1 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                g_registry->add(MAKE_KEY_CPU_2("v_assign_masked", op0, op1), std::make_shared<Algo_v_assign_masked_cpu<T_UINT>>());
            }
        }

        // algorthm mxv_masked
        for (const auto& op0 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
            for (const auto& op1 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
                for (const auto& op2 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                    g_registry->add(MAKE_KEY_CPU_3("mxv_masked", op0, op1, op2), std::make_shared<Algo_mxv_masked_cpu<T_INT>>());
                }
            }
        }
        for (const auto& op0 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
            for (const auto& op1 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
                for (const auto& op2 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                    g_registry->add(MAKE_KEY_CPU_3("mxv_masked", op0, op1, op2), std::make_shared<Algo_mxv_masked_cpu<T_UINT>>());
                }
            }
        }
        for (const auto& op0 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
            for (const auto& op1 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
                for (const auto& op2 : {EQZERO_FLOAT, NQZERO_FLOAT, GTZERO_FLOAT, GEZERO_FLOAT, LTZERO_FLOAT, LEZERO_FLOAT, ALWAYS_FLOAT, NEVER_FLOAT}) {
                    g_registry->add(MAKE_KEY_CPU_3("mxv_masked", op0, op1, op2), std::make_shared<Algo_mxv_masked_cpu<T_FLOAT>>());
                }
            }
        }
        for (const auto& op0 : {BOR_INT, BAND_INT, BXOR_INT}) {
            for (const auto& op1 : {BOR_INT, BAND_INT, BXOR_INT}) {
                for (const auto& op2 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                    g_registry->add(MAKE_KEY_CPU_3("mxv_masked", op0, op1, op2), std::make_shared<Algo_mxv_masked_cpu<T_INT>>());
                }
            }
        }
        for (const auto& op0 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
            for (const auto& op1 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
                for (const auto& op2 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                    g_registry->add(MAKE_KEY_CPU_3("mxv_masked", op0, op1, op2), std::make_shared<Algo_mxv_masked_cpu<T_UINT>>());
                }
            }
        }

        // algorthm vxm_masked
        for (const auto& op0 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
            for (const auto& op1 : {PLUS_INT, MINUS_INT, MULT_INT, DIV_INT, FIRST_INT, SECOND_INT, ONE_INT, MIN_INT, MAX_INT}) {
                for (const auto& op2 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                    g_registry->add(MAKE_KEY_CPU_3("vxm_masked", op0, op1, op2), std::make_shared<Algo_vxm_masked_cpu<T_INT>>());
                }
            }
        }
        for (const auto& op0 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
            for (const auto& op1 : {PLUS_UINT, MINUS_UINT, MULT_UINT, DIV_UINT, FIRST_UINT, SECOND_UINT, ONE_UINT, MIN_UINT, MAX_UINT}) {
                for (const auto& op2 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                    g_registry->add(MAKE_KEY_CPU_3("vxm_masked", op0, op1, op2), std::make_shared<Algo_vxm_masked_cpu<T_UINT>>());
                }
            }
        }
        for (const auto& op0 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
            for (const auto& op1 : {PLUS_FLOAT, MINUS_FLOAT, MULT_FLOAT, DIV_FLOAT, FIRST_FLOAT, SECOND_FLOAT, ONE_FLOAT, MIN_FLOAT, MAX_FLOAT}) {
                for (const auto& op2 : {EQZERO_FLOAT, NQZERO_FLOAT, GTZERO_FLOAT, GEZERO_FLOAT, LTZERO_FLOAT, LEZERO_FLOAT, ALWAYS_FLOAT, NEVER_FLOAT}) {
                    g_registry->add(MAKE_KEY_CPU_3("vxm_masked", op0, op1, op2), std::make_shared<Algo_vxm_masked_cpu<T_FLOAT>>());
                }
            }
        }
        for (const auto& op0 : {BOR_INT, BAND_INT, BXOR_INT}) {
            for (const auto& op1 : {BOR_INT, BAND_INT, BXOR_INT}) {
                for (const auto& op2 : {EQZERO_INT, NQZERO_INT, GTZERO_INT, GEZERO_INT, LTZERO_INT, LEZERO_INT, ALWAYS_INT, NEVER_INT}) {
                    g_registry->add(MAKE_KEY_CPU_3("vxm_masked", op0, op1, op2), std::make_shared<Algo_vxm_masked_cpu<T_INT>>());
                }
            }
        }
        for (const auto& op0 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
            for (const auto& op1 : {BOR_UINT, BAND_UINT, BXOR_UINT}) {
                for (const auto& op2 : {EQZERO_UINT, NQZERO_UINT, GTZERO_UINT, GEZERO_UINT, LTZERO_UINT, LEZERO_UINT, ALWAYS_UINT, NEVER_UINT}) {
                    g_registry->add(MAKE_KEY_CPU_3("vxm_masked", op0, op1, op2), std::make_shared<Algo_vxm_masked_cpu<T_UINT>>());
                }
            }
        }
    }

}// namespace spla