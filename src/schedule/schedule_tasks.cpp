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

#include "schedule_tasks.hpp"

#include <core/registry.hpp>

#include <sstream>

namespace spla {

    void ScheduleTaskBase::set_label(std::string new_label) {
        label = std::move(new_label);
    }
    const std::string& ScheduleTaskBase::get_label() const {
        return label;
    }

    ref_ptr<Descriptor> ScheduleTaskBase::get_desc() {
        return desc;
    }

    ref_ptr<Descriptor> ScheduleTaskBase::get_desc_or_default() {
        static ref_ptr<Descriptor> default_desc(new Descriptor);
        return desc.is_not_null() ? desc : default_desc;
    }

    std::string ScheduleTask_callback::get_name() {
        return "callback";
    }
    std::string ScheduleTask_callback::get_key() {
        return "callback";
    }
    std::string ScheduleTask_callback::get_key_full() {
        return "callback";
    }
    std::vector<ref_ptr<Object>> ScheduleTask_callback::get_args() {
        return {};
    }

    std::string ScheduleTask_mxv_masked::get_name() {
        return "mxv_masked";
    }
    std::string ScheduleTask_mxv_masked::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(r->get_type());

        return key.str();
    }
    std::string ScheduleTask_mxv_masked::get_key_full() {
        std::stringstream key;
        key << get_name()
            << OP_KEY(op_multiply)
            << OP_KEY(op_add)
            << OP_KEY(op_select);

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_mxv_masked::get_args() {
        return {r.as<Object>(), mask.as<Object>(), M.as<Object>(), v.as<Object>(), op_multiply.as<Object>(), op_add.as<Object>(), op_select.as<Object>(), init.as<Object>()};
    }

    std::string ScheduleTask_vxm_masked::get_name() {
        return "vxm_masked";
    }
    std::string ScheduleTask_vxm_masked::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(r->get_type());

        return key.str();
    }
    std::string ScheduleTask_vxm_masked::get_key_full() {
        std::stringstream key;
        key << get_name()
            << OP_KEY(op_multiply)
            << OP_KEY(op_add)
            << OP_KEY(op_select);

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_vxm_masked::get_args() {
        return {r.as<Object>(), mask.as<Object>(), v.as<Object>(), M.as<Object>(), op_multiply.as<Object>(), op_add.as<Object>(), op_select.as<Object>(), init.as<Object>()};
    }

    std::string ScheduleTask_v_eadd_fdb::get_name() {
        return "v_eadd_fdb";
    }
    std::string ScheduleTask_v_eadd_fdb::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(r->get_type());

        return key.str();
    }
    std::string ScheduleTask_v_eadd_fdb::get_key_full() {
        std::stringstream key;
        key << get_name()
            << OP_KEY(op);

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_v_eadd_fdb::get_args() {
        return {r.as<Object>(), v.as<Object>(), fdb.as<Object>(), op.as<Object>()};
    }

    std::string ScheduleTask_v_assign_masked::get_name() {
        return "v_assign_masked";
    }
    std::string ScheduleTask_v_assign_masked::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(r->get_type());

        return key.str();
    }
    std::string ScheduleTask_v_assign_masked::get_key_full() {
        std::stringstream key;
        key << get_name()
            << OP_KEY(op_assign)
            << OP_KEY(op_select);

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_v_assign_masked::get_args() {
        return {r.as<Object>(), mask.as<Object>(), value.as<Object>(), op_assign.as<Object>(), op_select.as<Object>()};
    }

    std::string ScheduleTask_v_reduce::get_name() {
        return "v_reduce";
    }
    std::string ScheduleTask_v_reduce::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(r->get_type());

        return key.str();
    }
    std::string ScheduleTask_v_reduce::get_key_full() {
        std::stringstream key;
        key << get_name()
            << OP_KEY(op_reduce);

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_v_reduce::get_args() {
        return {r.as<Object>(), s.as<Object>(), v.as<Object>(), op_reduce.as<Object>()};
    }

    std::string ScheduleTask_v_count_mf::get_name() {
        return "v_count_mf";
    }
    std::string ScheduleTask_v_count_mf::get_key() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(v->get_type());

        return key.str();
    }
    std::string ScheduleTask_v_count_mf::get_key_full() {
        std::stringstream key;
        key << get_name()
            << TYPE_KEY(v->get_type());

        return key.str();
    }
    std::vector<ref_ptr<Object>> ScheduleTask_v_count_mf::get_args() {
        return {r.as<Object>(), v.as<Object>()};
    }

}// namespace spla
