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
#include <core/configure.hpp>
#include <core/dispatcher.hpp>
#include <core/logger.hpp>
#include <core/registry.hpp>
#include <core/top.hpp>

#include <cpu/cpu_algo_registry.hpp>

#include <cstdlib>
#include <iostream>

#if defined(SPLA_BUILD_OPENCL)
    #include <opencl/cl_accelerator.hpp>
    #include <opencl/cl_algo_registry.hpp>
#endif

#include <profiling/time_profiler.hpp>

namespace spla {

    static Status to_status(ConfigStatus status) {
        switch (status) {
            case ConfigStatus::Ok:
            case ConfigStatus::HelpRequested:
            case ConfigStatus::VersionRequested:
                return Status::Ok;
            case ConfigStatus::CliOrEnvParseError:
            case ConfigStatus::ConfigurationNotFound:
            case ConfigStatus::ConfigurationCycle:
            case ConfigStatus::MissedParameters:
            case ConfigStatus::InvalidConfigParams:
                return Status::InvalidArgument;
            case ConfigStatus::PlatformNotFound:
                return Status::PlatformNotFound;
            case ConfigStatus::DeviceNotFound:
                return Status::DeviceNotFound;
            case ConfigStatus::FileNotExists:
            case ConfigStatus::OpenFileError:
            case ConfigStatus::ParseConfError:
            default:
                return Status::Error;
        }
    }

    static const char* to_string(Status status) {
#define STATUS_MAP(value) \
    case Status::value:   \
        return #value

        switch (status) {
            STATUS_MAP(Ok);
            STATUS_MAP(Error);
            STATUS_MAP(NoAcceleration);
            STATUS_MAP(PlatformNotFound);
            STATUS_MAP(DeviceNotFound);
            STATUS_MAP(InvalidState);
            STATUS_MAP(InvalidArgument);
            STATUS_MAP(NoValue);
            STATUS_MAP(NotImplemented);
            default:
                return "none";
        }
#undef STATUS_MAP
    }

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

    Status Library::configure(int argc, char** argv) {
        ConfigStatus config_status = spla::make_config_final(argc, argv);

        if (config_status == ConfigStatus::HelpRequested || config_status == ConfigStatus::VersionRequested) {
            return Status::Ok;
        }
        if (config_status != ConfigStatus::Ok) {
            return to_status(config_status);
        }

        set_verbosity(spla::config_final.verbosity.value_or(2));
        set_default_callback();

        const std::string backend = spla::config_final.backend.value_or("opencl_gpu");
         
        if (backend == "cpu") {
            return set_accelerator(AcceleratorType::None);
        }

#if defined(SPLA_BUILD_OPENCL)
        Status status = set_accelerator(AcceleratorType::OpenCL);

        if (status != Status::Ok) {
            const std::string if_unavail = spla::config_final.if_opencl_unavailable.value_or("abort");
            if (if_unavail == "use_cpu") {
                LOG_MSG(Status::NoAcceleration,
                        "OpenCL acceleration is unavailable (status: "
                                << static_cast<int>(status)
                                << "), falling back to pure CPU computations");
                set_accelerator(AcceleratorType::None);
                return Status::Ok;
            }
            LOG_MSG(status,
                    "OpenCL acceleration is unavailable (status: "
                            << static_cast<int>(status)
                            << "), configuration requested abort");
            return status;
        }
        return Status::Ok;
#else
        const std::string if_unavail = spla::config_final.if_opencl_unavailable.value_or("abort");
        if (if_unavail == "use_cpu") {
            set_accelerator(AcceleratorType::None);
            return Status::Ok;
        }
        LOG_MSG(Status::Error,
                "OpenCL support not built, but OpenCL backend requested");
        return Status::Error;
#endif
    }

    Status Library::set_accelerator(AcceleratorType accelerator) {
#if defined(SPLA_BUILD_OPENCL)
        if (accelerator == AcceleratorType::OpenCL) {
            m_accelerator = std::make_unique<CLAccelerator>();

            Status status = m_accelerator->init();
            if (status != Status::Ok) {
                m_accelerator.reset();
                return status;
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

    Status Library::set_verbosity(int verbosity) {
        m_logger->set_verbosity(verbosity);
        LOG_MSG(Status::Ok, "set verbosity: " << verbosity);
        return Status::Ok;
    }

    Status Library::set_default_callback() {
        auto callback = [](spla::Status                        status,
                           const std::string&                  msg,
                           [[maybe_unused]] const std::string& file,
                           [[maybe_unused]] const std::string& function,
                           [[maybe_unused]] int                line) {
            std::ostream& out = (status == Status::Ok) ? std::cout : std::cerr;
            out << to_string(status) << ": " << msg << std::endl;
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

    Status Library::get_accelerator_info(std::string& info) {
        if (!m_accelerator) {
            info = "none";
            return Status::NoAcceleration;
        }
        info = m_accelerator->get_description();
        return Status::Ok;
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

    Library* Library::get(int argc, char** argv) {
        static std::unique_ptr<Library> g_library;

        if (!g_library) {
            g_library = std::make_unique<Library>();

            if (g_library->configure(argc, argv) != Status::Ok) {
                std::cerr << "failed to apply SPLA configuration; continuing with pure CPU computations" << std::endl;
            }
        }

        return g_library.get();
    }

}// namespace spla
