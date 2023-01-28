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

#include <spla/library.hpp>

#include <core/accelerator.hpp>
#include <core/dispatcher.hpp>
#include <core/logger.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>

#include <cpu/cpu_algo_registry.hpp>

#include <iostream>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_algo_registry.hpp>
#endif

#include <profiling/time_profiler.hpp>

namespace spla {

    Library::Library() {
        // Setup logger (always present in the system)
        m_logger = std::make_unique<Logger>();
        // Setup registry (always available)
        m_registry = std::make_unique<Registry>();
        // Setup dispatcher (always available)
        m_dispatcher = std::make_unique<Dispatcher>();

        // Register build-in bin ops (id's done here, since registration depend on types)
        register_ops();

        // Register cpu algo version
        register_algo_cpu(m_registry.get());

#ifdef SPLA_BUILD_OPENCL
        // Register cl algo version
        register_algo_cl(m_registry.get());
#endif

#ifndef SPLA_RELEASE
        // Setup profiler (do not have only in release builds)
        m_time_profiler = std::make_unique<TimeProfiler>();
#endif
    }

    Library::~Library() = default;

    void Library::finalize() {
        LOG_MSG(Status::Ok, "finalize library state");

        if (m_accelerator) {
            LOG_MSG(Status::Ok, "release accelerator: " << m_accelerator->get_name());
            m_accelerator.reset();
        }
    }

    Status Library::set_accelerator(AcceleratorType accelerator) {
#if defined(SPLA_BUILD_OPENCL)
        if (accelerator == AcceleratorType::OpenCL) {
            m_accelerator = std::make_unique<CLAccelerator>();

            if (m_accelerator->init() != Status::Ok) {
                m_accelerator.reset();
                return Status::NoAcceleration;
            }

            return Status::Ok;
        }
#endif
        if (accelerator == AcceleratorType::None) {
            LOG_MSG(Status::Ok, "disable acceleration");
            m_accelerator.reset();
            return Status::Ok;
        }

        return Status::NoAcceleration;
    }

    Status Library::set_platform(int index) {
        return m_accelerator ? m_accelerator->set_platform(index) : Status::NoAcceleration;
    }

    Status Library::set_device(int index) {
        return m_accelerator ? m_accelerator->set_device(index) : Status::NoAcceleration;
    }

    Status Library::set_queues_count(int count) {
        return m_accelerator ? m_accelerator->set_queues_count(count) : Status::NoAcceleration;
    }

    Status Library::set_message_callback(MessageCallback callback) {
        m_logger->set_msg_callback(std::move(callback));
        LOG_MSG(Status::Ok, "set new message callback");
        return Status::Ok;
    }

    Status Library::set_default_callback() {
        auto callback = [](spla::Status       status,
                           const std::string& msg,
                           const std::string& file,
                           const std::string& function,
                           int                line) {
            std::stringstream to_output;

            to_output << "[" << file << ":" << line << "] "
                      << to_string(status) << ": " << msg << std::endl;

            if (status == Status::Ok)
                std::cout << to_output.str();
            else
                std::cerr << to_output.str();
        };
        return set_message_callback(callback);
    }

    Status Library::set_force_no_acceleration(bool value) {
        LOG_MSG(Status::Ok, "force no acc: " << value);
        m_force_no_acc = value;
        return Status::Ok;
    }

    bool Library::is_set_force_no_acceleration() {
        return m_force_no_acc;
    }

    Status Library::time_profile_dump() {
#ifndef SPLA_RELEASE
        m_time_profiler->dump(std::cout);
#endif
        return Status::Ok;
    }

    Status Library::time_profile_reset() {
#ifndef SPLA_RELEASE
        m_time_profiler->reset();
#endif
        return Status::Ok;
    }

    class Accelerator* Library::get_accelerator() {
        return m_accelerator.get();
    }

    class Registry* Library::get_registry() {
        return m_registry.get();
    }

    class Dispatcher* Library::get_dispatcher() {
        return m_dispatcher.get();
    }

    class Logger* Library::get_logger() {
        return m_logger.get();
    }

    class TimeProfiler* Library::get_time_profiler() {
        return m_time_profiler.get();
    }

    Library* get_library() {
        static std::unique_ptr<Library> g_library;

        if (!g_library) {
            g_library = std::make_unique<Library>();

#ifndef SPLA_RELEASE
            // In debug mode we automatically set default callback
            // to get log (error) messages from library start-up
            g_library->set_default_callback();
#endif

            // On init we by default attempt to setup OpenCL runtime
            // If setup is failed error is ignored and library fallbacks to CPU computations only
            g_library->set_accelerator(AcceleratorType::OpenCL);
        }

        return g_library.get();
    }

}// namespace spla