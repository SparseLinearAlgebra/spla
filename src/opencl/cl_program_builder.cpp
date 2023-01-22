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

#include "cl_program_builder.hpp"

#include <spla/timer.hpp>

namespace spla {

    CLProgramBuilder& CLProgramBuilder::set_name(const char* name) {
        m_name = name;
        return *this;
    }
    CLProgramBuilder& CLProgramBuilder::add_define(const char* define, int value) {
        m_defines.emplace_back(define + std::string(" ") + std::to_string(value));
        return *this;
    }
    CLProgramBuilder& CLProgramBuilder::add_type(const char* define, const ref_ptr<Type>& type) {
        m_defines.emplace_back(define + std::string(" ") + type->get_cpp());
        return *this;
    }
    CLProgramBuilder& CLProgramBuilder::add_op(const char* name, const ref_ptr<OpBinary>& op) {
        m_functions.emplace_back(op->get_type_res()->get_cpp() + " " + name + " " + op->get_source());
        return *this;
    }
    CLProgramBuilder& CLProgramBuilder::add_op(const char* name, const ref_ptr<OpSelect>& op) {
        m_functions.emplace_back(std::string("bool ") + name + " " + op->get_source());
        return *this;
    }
    CLProgramBuilder& CLProgramBuilder::set_source(const char* source) {
        m_source = source;
        return *this;
    }
    bool CLProgramBuilder::build() {
        CLAccelerator*  acc   = get_acc_cl();
        CLProgramCache* cache = acc->get_cache();

        std::stringstream cache_key;
        cache_key << m_name;
        for (auto& entry : m_defines) cache_key << entry;
        for (auto& entry : m_functions) cache_key << entry;

        m_program = cache->get_program(cache_key.str());

        if (m_program) {
            LOG_MSG(Status::Ok, "found program '" << m_name << "' in cache");
            return true;
        }

        std::stringstream builder;

        for (const auto& define : m_defines) {
            builder << "#define " << define << "\n";
        }
        for (const auto& function : m_functions) {
            builder << function << "\n";
        }
        builder << m_source;

        m_program_code       = builder.str();
        m_program            = std::make_shared<CLProgram>();
        m_program->m_program = cl::Program(acc->get_context(), m_program_code);

        Timer t;
        t.start();
        auto status = m_program->m_program.build("-cl-std=CL1.2");
        t.stop();

        if (status != CL_SUCCESS) {
            LOG_MSG(Status::Error, "failed to build program: " << m_program->m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(acc->get_device()));
            LOG_MSG(Status::Error, "src\n" << m_source);
            return false;
        }

        m_program->m_sources.emplace_back(m_source);
        m_program->m_defines   = std::move(m_defines);
        m_program->m_functions = std::move(m_functions);
        m_program->m_source    = std::move(m_program_code);
        m_program->m_name      = std::move(m_name);
        m_program->m_key       = cache_key.str();
        LOG_MSG(Status::Ok, "build program '" << m_program->m_name << "' in " << t.get_elapsed_sec() << "sec");

        cache->add_program(m_program);

        return true;
    }

}// namespace spla