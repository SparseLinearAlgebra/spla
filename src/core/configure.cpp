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

#include "configure.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace spla {

    ConfigFile    config_default;
    ConfigFile    config_system;
    ConfigFile    config_user;
    ConfigFile    config_explicit;
    CliMeta       config_cli_meta;
    Configuration config_cli_and_env;
    Configuration config_final;
    bool          config_initialized           = false;
    bool          config_explicitly_configured = false;


    void Configuration::merge(const Configuration& source) {
        if (source.extends.has_value()) extends = source.extends;
        if (source.backend.has_value()) backend = source.backend;
        if (source.platform_index.has_value()) platform_index = source.platform_index;
        if (source.device_index.has_value()) device_index = source.device_index;
        if (source.if_opencl_unavailable.has_value()) if_opencl_unavailable = source.if_opencl_unavailable;
        if (source.queues_count.has_value()) queues_count = source.queues_count;
        if (source.profiling.has_value()) profiling = source.profiling;
        if (source.allocator_type.has_value()) allocator_type = source.allocator_type;
        if (source.linear_alloc_size.has_value()) linear_alloc_size = source.linear_alloc_size;
        if (source.verbosity.has_value()) verbosity = source.verbosity;
    }

    void Configuration::reset() {
        *this = Configuration{};
    }


    void ConfigFile::merge(const ConfigFile& source) {
        if (!source.configs.has_value()) return;
        if (!configs.has_value()) configs.emplace();

        for (const auto& [name, source_config] : *source.configs) {
            (*configs)[name].merge(source_config);
        }
    }

    void ConfigFile::reset() {
        configs.reset();
    }

    void CliMeta::reset() {
        *this = CliMeta{};
    }


    std::string get_spla_version() {
#ifdef SPLA_VERSION
        return "SPLA version: " SPLA_VERSION;
#else
        return "SPLA version: unknown (SPLA_VERSION not defined at build time)";
#endif
    }


    std::string get_default_config_path() {
#ifdef _WIN32
        HMODULE hModule = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<LPCSTR>(&get_default_config_path), &hModule);
        char path[MAX_PATH];
        GetModuleFileNameA(hModule, path, MAX_PATH);
        std::string dir = path;
        dir             = dir.substr(0, dir.find_last_of("\\/"));
        return dir + "\\spla_conf.json";
#elif defined(__APPLE__) || defined(__linux__)
        Dl_info info;
        dladdr((void*) &get_default_config_path, &info);
        std::string path = info.dli_fname;
        std::string dir  = path.substr(0, path.find_last_of("/"));
        return dir + "/spla_conf.json";
#else
    #error "spla supports only Windows, macOS and Linux"
#endif
    }

    std::string get_system_config_path() {
#ifdef _WIN32
        const char* program_data = std::getenv("PROGRAMDATA");
        if (program_data) {
            return std::string(program_data) + "\\spla\\spla_conf.json";
        }
        return "C:\\ProgramData\\spla\\spla_conf.json";

#elif defined(__APPLE__)
        return "/Library/Application Support/spla/spla_conf.json";

#elif defined(__linux__)
        return "/etc/spla/spla_conf.json";

#else
    #error "spla supports only Windows, macOS and Linux"
#endif
    }


    std::string get_home_directory() {
#ifdef _WIN32
        const char* home = std::getenv("USERPROFILE");
        if (home) return std::string(home);

        const char* drive = std::getenv("HOMEDRIVE");
        const char* path  = std::getenv("HOMEPATH");
        if (drive && path) return std::string(drive) + std::string(path);
        throw std::runtime_error("Cannot determine home directory");

#elif defined(__APPLE__) || defined(__linux__)
        const char* home = std::getenv("HOME");
        if (home) return std::string(home);

        struct passwd* pw = getpwuid(getuid());
        if (pw) return std::string(pw->pw_dir);
        throw std::runtime_error("Cannot determine home directory");

#else
    #error "spla supports only Windows, macOS and Linux"
#endif
    }


    std::string get_user_config_path() {
        std::string home = get_home_directory();

#ifdef _WIN32
        const char* app_data = std::getenv("APPDATA");
        if (app_data) return std::string(app_data) + "\\spla\\spla_conf.json";
        return home + "\\AppData\\Roaming\\spla\\spla_conf.json";

#elif defined(__APPLE__)
        return home + "/Library/Application Support/spla/spla_conf.json";

#elif defined(__linux__) || defined(__unix__)
        return home + "/.config/spla/spla_conf.json";

#else
    #error "spla supports only Windows, macOS and Linux"
#endif
    }


    ConfigStatus parse_cli_and_env(int argc, char** argv, CliMeta& meta, Configuration& cfg) {
        CLI::App app{"SPLA configuration"};
        app.name("spla");

        app.allow_extras();

        app.set_help_flag("--spla-help", "Show help and exit");
        app.add_flag("--spla-version", meta.version, "Show version and exit");

        app.add_option("--spla-config", meta.configuration,
                       "Name of the configuration to apply.\n"
                       "If not specified, the configuration named 'default' is used.\n"
                       "Config key: configuration")
                ->envname("SPLA_CONFIG");

        app.add_option("--spla-config-file", meta.path_to_conf,
                       "Path to a configuration file to load explicitly.\n"
                       "Has higher priority than system and user configuration files,\n"
                       "but lower than command-line arguments and environment variables.\n"
                       "Config key: path_to_conf")
                ->envname("SPLA_CONFIG_FILE");

        app.add_option("--spla-backend", cfg.backend,
                       "Device type to use:\n"
                       "  cpu           - pure native CPU (no OpenCL)\n"
                       "  opencl_gpu    - only GPU devices via OpenCL\n"
                       "  by_index      - select by platform_index and device_index\n"
                       "Config key: backend")
                ->envname("SPLA_BACKEND");

        app.add_option("--spla-platform-index", cfg.platform_index,
                       "OpenCL platform index\n"
                       "Used only with backend=by_index.\n"
                       "Config key: platform_index")
                ->envname("SPLA_PLATFORM_INDEX");

        app.add_option("--spla-device-index", cfg.device_index,
                       "OpenCL device index\n"
                       "Used only with backend=by_index.\n"
                       "Config key: device_index")
                ->envname("SPLA_DEVICE_INDEX");

        app.add_option("--spla-if-opencl-unavailable", cfg.if_opencl_unavailable,
                       "Behavior when OpenCL acceleration is unavailable:\n"
                       "  use_cpu - fall back to pure CPU\n"
                       "  abort   - return error\n"
                       "Config key: if_opencl_unavailable")
                ->envname("SPLA_IF_OPENCL_UNAVAILABLE");

        app.add_option("--spla-queues-count", cfg.queues_count,
                       "Number of command queues\n"
                       "Config key: queues_count")
                ->envname("SPLA_QUEUES_COUNT");

        app.add_option("--spla-profiling", cfg.profiling,
                       "Enable profiling of command queues: true or false\n"
                       "Config key: profiling")
                ->envname("SPLA_PROFILING");

        app.add_option("--spla-allocator-type", cfg.allocator_type,
                       "Allocator type:\n"
                       "  general\n"
                       "  linear\n"
                       "Config key: allocator_type")
                ->envname("SPLA_ALLOCATOR_TYPE");

        app.add_option("--spla-linear-alloc-size", cfg.linear_alloc_size,
                       "Linear allocator size in bytes\n"
                       "Required for allocator_type=linear. Ignored for general.\n"
                       "Config key: linear_alloc_size")
                ->envname("SPLA_LINEAR_ALLOC_SIZE");

        app.add_option("--spla-verbosity", cfg.verbosity,
                       "Verbosity level:\n"
                       "  0 - no output\n"
                       "  1 - errors only\n"
                       "  2 - errors and warnings\n"
                       "  3 - all messages\n"
                       "Config key: verbosity")
                ->envname("SPLA_VERBOSITY");

        std::vector<std::string> args;
        if (argv != nullptr && argc > 0) {
            args.reserve(static_cast<size_t>(argc));
            for (int i = argc - 1; i >= 0; --i) {
                args.emplace_back(argv[i]);
            }
        }

        try {
            app.parse(std::move(args));
        } catch (const CLI::CallForHelp& e) {
            std::cout << app.help() << std::endl;
            return ConfigStatus::HelpRequested;
        } catch (const CLI::ParseError& e) {
            std::cerr << "[spla:cli_env] ERROR: failed to parse: " << e.what() << std::endl;
            return ConfigStatus::CliOrEnvParseError;
        }

        if (meta.version.value_or(false)) {
            std::cout << get_spla_version() << std::endl;
            return ConfigStatus::VersionRequested;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus parse_file(const std::string& path, ConfigFile& cfg, bool allow_default) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[spla:config] ERROR: cannot open: " << path << std::endl;
            return ConfigStatus::OpenFileError;
        }

        try {
            nlohmann::json data = nlohmann::json::parse(file);
            if (!cfg.configs.has_value()) cfg.configs.emplace();

            for (auto& [name, value] : data.items()) {
                if (!allow_default && name == "default") {
                    std::cerr << "[spla:config] ERROR: '" << path << "': "
                              << "configuration 'default' can only be defined in the library configuration file"
                              << std::endl;
                    return ConfigStatus::ParseConfError;
                }

                Configuration config;
                if (value.contains("extends")) config.extends = value["extends"].get<std::vector<std::string>>();
                if (value.contains("backend")) config.backend = value["backend"].get<std::string>();
                if (value.contains("platform_index")) config.platform_index = value["platform_index"].get<int>();
                if (value.contains("device_index")) config.device_index = value["device_index"].get<int>();
                if (value.contains("if_opencl_unavailable")) config.if_opencl_unavailable = value["if_opencl_unavailable"].get<std::string>();
                if (value.contains("queues_count")) config.queues_count = value["queues_count"].get<int>();
                if (value.contains("profiling")) config.profiling = value["profiling"].get<bool>();
                if (value.contains("allocator_type")) config.allocator_type = value["allocator_type"].get<std::string>();
                if (value.contains("linear_alloc_size")) {
                    const std::int64_t size = value["linear_alloc_size"].get<std::int64_t>();
                    if (size <= 0) {
                        std::cerr << "[spla:config] ERROR: '" << path << "': "
                                  << "linear_alloc_size must be > 0 (got " << size << ")" << std::endl;
                        return ConfigStatus::InvalidConfigParams;
                    }
                    config.linear_alloc_size = static_cast<size_t>(size);
                }
                if (value.contains("verbosity")) config.verbosity = value["verbosity"].get<int>();

                (*cfg.configs)[name].merge(config);
            }

            return ConfigStatus::Ok;

        } catch (const nlohmann::json::exception& e) {
            std::cerr << "[spla:config] ERROR: failed to parse '"
                      << path << "': " << e.what() << std::endl;
            return ConfigStatus::ParseConfError;
        }
    }


    Configuration apply_extends(const std::map<std::string, Configuration>& configs,
                                const std::string&                          name,
                                std::set<std::string>&                      stack) {
        if (stack.count(name)) {
            throw std::runtime_error("extends cycle detected: " + name);
        }
        stack.insert(name);

        auto it = configs.find(name);
        if (it == configs.end()) {
            throw std::runtime_error("extends configuration not found: " + name);
        }

        const Configuration& source = it->second;
        Configuration        result;

        if (source.extends.has_value()) {
            for (const auto& parent_name : *source.extends) {
                result.merge(apply_extends(configs, parent_name, stack));
            }
        }
        result.merge(source);
        stack.erase(name);
        return result;
    }

    ConfigStatus apply_configuration(const ConfigFile& file, const std::string& name, Configuration& cfg) {

        std::set<std::string> stack;
        try {
            cfg = apply_extends(*file.configs, name, stack);
        } catch (const std::runtime_error& e) {
            const std::string msg = e.what();
            if (msg.find("cycle") != std::string::npos) {
                std::cerr << "[spla:configure] ERROR: extends cycle detected in configuration '"
                          << name << "'" << std::endl;
                return ConfigStatus::ConfigurationCycle;
            }
            std::cerr << "[spla:configure] ERROR: " << msg << std::endl;
            return ConfigStatus::ConfigurationNotFound;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus validation(const Configuration& cfg) {
        if (!cfg.backend.has_value()) {
            std::cerr << "[spla:validation] ERROR: required parameter missing: backend" << std::endl;
            return ConfigStatus::MissedParameters;
        }

        const std::string& backend = *cfg.backend;
        if (backend != "opencl_gpu" && backend != "cpu" && backend != "by_index") {
            std::cerr << "[spla:validation] ERROR: backend must be "
                      << "'opencl_gpu', 'cpu' or 'by_index' (got '" << backend << "')" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        if (backend == "by_index") {
            if (!cfg.platform_index.has_value()) {
                std::cerr << "[spla:validation] ERROR: platform_index is required "
                          << "when backend='by_index'" << std::endl;
                return ConfigStatus::MissedParameters;
            }
            if (!cfg.device_index.has_value()) {
                std::cerr << "[spla:validation] ERROR: device_index is required "
                          << "when backend='by_index'" << std::endl;
                return ConfigStatus::MissedParameters;
            }

            if (*cfg.platform_index < 0) {
                std::cerr << "[spla:validation] ERROR: platform_index must be >= 0 (got "
                          << *cfg.platform_index << ")" << std::endl;
                return ConfigStatus::InvalidConfigParams;
            }
            if (*cfg.device_index < 0) {
                std::cerr << "[spla:validation] ERROR: device_index must be >= 0 (got "
                          << *cfg.device_index << ")" << std::endl;
                return ConfigStatus::InvalidConfigParams;
            }
        } else if (cfg.platform_index.has_value()) {
            std::cerr << "[spla:validation] WARNING: platform_index: " << *cfg.platform_index
                      << " ignored for backend='" << backend << "'" << std::endl;

        } else if (cfg.device_index.has_value()) {
            std::cerr << "[spla:validation] WARNING: device_index " << *cfg.device_index
                      << "ignored for backend='" << backend << "'" << std::endl;
        }


        if (backend != "cpu" && !cfg.if_opencl_unavailable.has_value()) {
            std::cerr << "[spla:validation] ERROR: if_gpu_unavailable is required "
                      << "when backend=" << backend << std::endl;
            return ConfigStatus::MissedParameters;
        }
        if (cfg.if_opencl_unavailable.has_value()) {
            const std::string& behavior = *cfg.if_opencl_unavailable;
            if (behavior != "use_cpu" && behavior != "abort") {
                std::cerr << "[spla:validation] ERROR: if_opencl_unavailable must be "
                          << "'use_cpu' or 'abort' (got '" << behavior << "')" << std::endl;
                return ConfigStatus::InvalidConfigParams;
            }
            if (backend == "cpu") {
                std::cerr << "[spla:validation] WARNING: if_opencl_unavailable "
                          << "ignored for backend='cpu'" << std::endl;
            }
        }


        if (!cfg.queues_count.has_value()) {
            std::cerr << "[spla:validation] ERROR: required parameter missing: queues_count" << std::endl;
            return ConfigStatus::MissedParameters;
        }
        if (*cfg.queues_count < 1) {
            std::cerr << "[spla:validation] ERROR: queues_count must be >= 1 (got "
                      << *cfg.queues_count << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }


        if (!cfg.profiling.has_value()) {
            std::cerr << "[spla:validation] ERROR: required parameter missing: profiling" << std::endl;
            return ConfigStatus::MissedParameters;
        }


        if (!cfg.allocator_type.has_value()) {
            std::cerr << "[spla:validation] ERROR: required parameter missing: allocator_type" << std::endl;
            return ConfigStatus::MissedParameters;
        }
        if (*cfg.allocator_type != "general" && *cfg.allocator_type != "linear") {
            std::cerr << "[spla:validation] ERROR: allocator_type must be "
                      << "'general' or 'linear' (got '" << *cfg.allocator_type << "')" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        if (*cfg.allocator_type == "linear") {
            if (!cfg.linear_alloc_size.has_value()) {
                std::cerr << "[spla:validation] ERROR: linear_alloc_size is required "
                          << "when allocator_type='linear'" << std::endl;
                return ConfigStatus::MissedParameters;
            }
            if (*cfg.linear_alloc_size == 0) {
                std::cerr << "[spla:validation] ERROR: linear_alloc_size must be > 0" << std::endl;
                return ConfigStatus::InvalidConfigParams;
            }
        } else if (cfg.linear_alloc_size.has_value()) {
            std::cerr << "[spla:validation] WARNING: linear_alloc_size "
                      << "ignored for allocator_type='" << *cfg.allocator_type << "'" << std::endl;
        }


        if (!cfg.verbosity.has_value()) {
            std::cerr << "[spla:validation] ERROR: required parameter missing: verbosity" << std::endl;
            return ConfigStatus::MissedParameters;
        }
        if (*cfg.verbosity < 0 || *cfg.verbosity > 3) {
            std::cerr << "[spla:validation] ERROR: verbosity must be in [0, 3] (got "
                      << *cfg.verbosity << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus configure(int argc, char** argv, bool explicit_request) {
        config_default.reset();
        config_system.reset();
        config_user.reset();
        config_explicit.reset();
        config_cli_meta.reset();
        config_cli_and_env.reset();
        config_final.reset();
        config_initialized           = false;
        config_explicitly_configured = false;

        ConfigStatus status;

        status = parse_cli_and_env(argc, argv, config_cli_meta, config_cli_and_env);
        if (status != ConfigStatus::Ok) return status;

        std::string path;

        path = get_default_config_path();
        if (!std::filesystem::exists(path)) {
            std::cerr << "[spla:configure] ERROR: default configuration file not found: " << path << std::endl;
            return ConfigStatus::FileNotExists;
        }
        status = parse_file(path, config_default, true);
        if (status != ConfigStatus::Ok) {
            std::cerr << "[spla:configure] ERROR: failed to load default configuration file: " << path << std::endl;
            return status;
        }


        path = get_system_config_path();
        if (std::filesystem::exists(path)) {
            status = parse_file(path, config_system, false);
            if (status != ConfigStatus::Ok) {
                std::cerr << "[spla:configure] ERROR: failed to load system configuration file: " << path << std::endl;
                return status;
            }
        }


        path = get_user_config_path();
        if (std::filesystem::exists(path)) {
            status = parse_file(path, config_user, false);
            if (status != ConfigStatus::Ok) {
                std::cerr << "[spla:configure] ERROR: failed to load user configuration file: " << path << std::endl;
                return status;
            }
        }


        if (config_cli_meta.path_to_conf.has_value()) {
            path = *config_cli_meta.path_to_conf;
            if (!std::filesystem::exists(path)) {
                std::cerr << "[spla:configure] ERROR: configuration file not found: " << path << std::endl;
                return ConfigStatus::FileNotExists;
            }
            status = parse_file(path, config_explicit, false);
            if (status != ConfigStatus::Ok) {
                std::cerr << "[spla:configure] ERROR: failed to load configuration file: " << path << std::endl;
                return status;
            }
        }

        ConfigFile merged;
        merged.merge(config_default);
        merged.merge(config_system);
        merged.merge(config_user);
        merged.merge(config_explicit);

        const std::string active = config_cli_meta.configuration.value_or("default");

        status = apply_configuration(merged, active, config_final);
        if (status != ConfigStatus::Ok) return status;

        config_final.merge(config_cli_and_env);

        status = validation(config_final);
        if (status != ConfigStatus::Ok) return status;

        config_initialized           = true;
        config_explicitly_configured = explicit_request;
        return ConfigStatus::Ok;
    }

}// namespace spla
