#include "cl_configure.hpp"
#include "cl_accelerator.hpp"
#include "opencl.hpp"

namespace spla {

    Config config_user_and_system;
    Config config_cli_and_env;
    Config config_final;

    void Config::merge(const Config& source) {
        if (source.system_config_path.has_value()) system_config_path = source.system_config_path;
        if (source.user_config_path.has_value()) user_config_path = source.user_config_path;
        if (source.platform.has_value()) platform = source.platform;
        if (source.device.has_value()) device = source.device;
        if (source.queues.has_value()) queues = source.queues;
        if (source.profiling.has_value()) profiling = source.profiling;
        if (source.allocator.has_value()) allocator = source.allocator;
        if (source.allocator_size.has_value()) allocator_size = source.allocator_size;
        if (source.verbosity.has_value()) verbosity = source.verbosity;
    }

    void Config::reset() {
        *this = Config{};
    }

    std::string get_home_directory() {
#ifdef _WIN32
        const char* home = std::getenv("USERPROFILE");
        if (home) return std::string(home);

        const char* drive = std::getenv("HOMEDRIVE");
        const char* path  = std::getenv("HOMEPATH");
        if (drive && path) return std::string(drive) + std::string(path);
        return "";

#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
        const char* home = std::getenv("HOME");
        if (home) return std::string(home);

        struct passwd* pw = getpwuid(getuid());
        if (pw) return std::string(pw->pw_dir);
        return "";

#else
    #error "Unsupported operating system"
#endif
    }

    std::string get_default_system_config_path() {
#ifdef _WIN32
        const char* program_data = std::getenv("PROGRAMDATA");
        if (program_data) {
            return std::string(program_data) + "\\spla\\";
        }
        return "C:\\ProgramData\\spla\\";

#elif defined(__APPLE__)
        return "/Library/Application Support/spla/";

#elif defined(__linux__) || defined(__unix__)
        return "/etc/spla/";

#else
    #error "Unsupported operating system"
#endif
    }

    std::string get_default_user_config_path() {
        std::string home = get_home_directory();
        if (home.empty()) {
            return "";
        }

#ifdef _WIN32
        const char* app_data = std::getenv("APPDATA");
        if (app_data) return std::string(app_data) + "\\spla\\";
        return home + "\\AppData\\Roaming\\spla\\";

#elif defined(__APPLE__)
        return home + "/Library/Application Support/spla/";

#elif defined(__linux__) || defined(__unix__)
        return home + "/.config/spla/";

#else
    #error "Unsupported operating system"
#endif
    }

    std::string find_first_json_file(const std::string& directory) {
        if (directory.empty() || !std::filesystem::exists(directory)) {
            return "";
        }

        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if (path.size() >= 5 &&
                    path.compare(path.size() - 5, 5, ".json") == 0) {
                    return path;
                }
            }
        }
        return "";
    }

    ConfigStatus load_from_file(const std::string& path, Config& cfg) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << path << std::endl;
                return ConfigStatus::OpenFileError;
            }

            nlohmann::json config_data = nlohmann::json::parse(file);

            if (config_data.contains("platform")) {
                cfg.platform = config_data["platform"].get<int>();
            }
            if (config_data.contains("device")) {
                cfg.device = config_data["device"].get<int>();
            }
            if (config_data.contains("queues")) {
                cfg.queues = config_data["queues"].get<int>();
            }
            if (config_data.contains("profiling")) {
                cfg.profiling = config_data["profiling"].get<bool>();
            }
            if (config_data.contains("allocator")) {
                cfg.allocator = config_data["allocator"].get<std::string>();
            }
            if (config_data.contains("allocator_size")) {
                cfg.allocator_size = config_data["allocator_size"].get<size_t>();
            }
            if (config_data.contains("verbosity")) {
                cfg.verbosity = config_data["verbosity"].get<int>();
            }

            return ConfigStatus::Ok;

        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Failed to parse config files" << std::endl;
            std::cerr << "Error parsing JSON file '" << path << "': " << e.what() << std::endl;
            return ConfigStatus::UserOrSystemConfParseError;
        }
    }

    ConfigStatus parse_system_and_user_conf(const Config& cli_env_config, Config& file_config) {
        std::string sys_dir  = cli_env_config.system_config_path.value_or(get_default_system_config_path());
        std::string user_dir = cli_env_config.user_config_path.value_or(get_default_user_config_path());

        ConfigStatus status = ConfigStatus::Ok;

        if (sys_dir != "") {
            std::string sys_file = find_first_json_file(sys_dir);
            if (sys_file != "") {
                std::cout << "Loading system config: " << sys_file << std::endl;
                status = load_from_file(sys_file, file_config);
                if (status == ConfigStatus::UserOrSystemConfParseError) return status;
            } else
                std::cout << "No system config found in: " << sys_dir << std::endl;
        } else
            std::cout << "System config directory is not set. Skipping." << std::endl;

        if (user_dir != "") {
            std::string user_file = find_first_json_file(user_dir);
            if (user_file != "") {
                std::cout << "Loading user config: " << user_file << std::endl;
                status = load_from_file(user_file, file_config);
                if (status == ConfigStatus::UserOrSystemConfParseError) return status;
            } else
                std::cout << "No user config found in: " << user_dir << std::endl;
        } else
            std::cout << "User config directory is not set. Skipping." << std::endl;

        if (status == ConfigStatus::OpenFileError) return status;

        return ConfigStatus::Ok;
    }

    std::string get_spla_version() {
        return "SPLA version: 0.0.0";
    }

    ConfigStatus parse_cli_and_env(int argc, char** argv, Config& cfg) {
        CLI::App app{"SPLA configuration"};

        app.add_flag("-sh,--spla-help", cfg.help, "Show help and exit");
        app.add_flag("-sv,--spla-version", cfg.version, "Show version and exit");

        app.add_option("-ss,--spla-sconf", cfg.system_config_path,
                       "Path to system configuration file\n"
                       "Config key: (not used in config file)")
                ->envname("SPLA_SYSTEM_CONFIG_PATH");

        app.add_option("-su,--spla-uconf", cfg.user_config_path,
                       "Path to user configuration file\n"
                       "Config key: (not used in config file)")
                ->envname("SPLA_USER_CONFIG_PATH");

        app.add_option("-sp,--spla-platform", cfg.platform,
                       "OpenCL platform index\n"
                       "Config key: platform")
                ->envname("SPLA_OPENCL_PLATFORM");

        app.add_option("-sd,--spla-device", cfg.device,
                       "OpenCL device index\n"
                       "Config key: device")
                ->envname("SPLA_OPENCL_DEVICE");

        app.add_option("-sq,--spla-queues", cfg.queues,
                       "Number of command queues\n"
                       "Config key: queues")
                ->envname("SPLA_QUEUES");

        app.add_flag("-pr,--spla-profiling", cfg.profiling,
                     "Enable profiling of command queues\n"
                     "Config key: profiling\n")
                ->envname("SPLA_PROFILING");

        app.add_option("-sa,--spla-allocator", cfg.allocator,
                       "Allocator type: linear or general\n"
                       "Config key: allocator")
                ->envname("SPLA_ALLOCATOR");

        app.add_option("-as,--spla-allocator-size", cfg.allocator_size,
                       "Linear allocator size in bytes\n"
                       "Required for 'linear' allocator. Ignored for 'general'.\n"
                       "Config key: allocator_size")
                ->envname("SPLA_ALLOCATOR_SIZE");

        app.add_option("-sV,--spla-verbosity", cfg.verbosity,
                       "Verbosity level:\n"
                       "  0: No output\n"
                       "  1: Errors only\n"
                       "  2: Errors + warnings\n"
                       "  3: All messages (info, warnings, errors)")
                ->envname("SPLA_VERBOSITY");

        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            std::cerr << "Failed to parse CLI/ENV" << std::endl;
            app.exit(e);
            return ConfigStatus::CliOrEnvParseError;
        }

        if (cfg.help) {
            std::cout << app.help() << std::endl;
            return ConfigStatus::HelpRequested;
        }

        if (cfg.version) {
            std::cout << get_spla_version() << std::endl;
            return ConfigStatus::VersionRequested;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus check_platform_and_device(int platform_index, int device_index) {

        if (platform_index < 0) {
            std::cerr << "Error: platform must be >= 0 (got " << platform_index << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.empty()) {
            std::cerr << "Error: no platform to select for OpenCL acceleration" << std::endl;
            return ConfigStatus::PlatformNotFound;
        }

        if (static_cast<size_t>(platform_index) >= platforms.size()) {
            std::cerr << "Error: platform index out of range (got " << platform_index << ", max " << platforms.size() - 1 << ")" << std::endl;
            return ConfigStatus::PlatformNotFound;
        }


        if (device_index < 0) {
            std::cerr << "Error: device must be >= 0 (got " << device_index << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        std::vector<cl::Device> devices;
        platforms[platform_index].getDevices(CL_DEVICE_TYPE_ALL, &devices);

        if (devices.empty()) {
            std::cerr << "Error: no device to select for OpenCL acceleration" << std::endl;
            return ConfigStatus::DeviceNotFound;
        }

        if (static_cast<size_t>(device_index) >= devices.size()) {
            std::cerr << "Error: device index out of range (got " << device_index << ", max " << devices.size() - 1 << ")" << std::endl;
            return ConfigStatus::DeviceNotFound;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus validate(const Config& cfg) {

        if (!cfg.platform.has_value()) {
            std::cerr << "Error: platform is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (!cfg.device.has_value()) {
            std::cerr << "Error: device is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (!cfg.queues.has_value()) {
            std::cerr << "Error: queues is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (!cfg.profiling.has_value()) {
            std::cerr << "Error: profiling is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (!cfg.allocator.has_value()) {
            std::cerr << "Error: allocator is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (cfg.allocator.value() == "linear" && !cfg.allocator_size.has_value()) {
            std::cerr << "Error: allocator_size is required for linear allocator" << std::endl;
            return ConfigStatus::MissedParametrs;
        }
        if (!cfg.verbosity.has_value()) {
            std::cerr << "Error: verbosity is required" << std::endl;
            return ConfigStatus::MissedParametrs;
        }


        ConfigStatus status;

        status = check_platform_and_device(*cfg.platform, *cfg.device);
        if (status != ConfigStatus::Ok) {
            return status;
        }


        if (*cfg.queues <= 0) {
            std::cerr << "Error: queues must be > 0 (got " << *cfg.queues << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }


        if (*cfg.allocator != "linear" && *cfg.allocator != "general") {
            std::cerr << "Error: allocator must be 'linear' or 'general' (got '" << *cfg.allocator << "')" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        if (*cfg.allocator == "linear") {
            if (*cfg.allocator_size <= 0) {
                std::cerr << "Error: allocator_size must be > 0 for linear allocator (got " << *cfg.allocator_size << ")" << std::endl;
                return ConfigStatus::InvalidConfigParams;
            }
        }


        if (*cfg.verbosity < 0 || *cfg.verbosity > 3) {
            std::cerr << "Error: verbosity must be between 0 and 3 (got " << *cfg.verbosity << ")" << std::endl;
            return ConfigStatus::InvalidConfigParams;
        }

        return ConfigStatus::Ok;
    }


    ConfigStatus configure(int argc, char** argv) {
        config_user_and_system.reset();
        config_cli_and_env.reset();
        config_final.reset();

        ConfigStatus config_status;

        config_status = parse_cli_and_env(argc, argv, config_cli_and_env);
        if (config_status != ConfigStatus::CliOrEnvParseError) return config_status;
        else {
            std::exit(1);
        }

        config_status = parse_system_and_user_conf(config_cli_and_env, config_user_and_system);
        if (config_status != ConfigStatus::UserOrSystemConfParseError) return config_status;
        else {
            std::exit(1);
        }

        config_final = config_user_and_system;
        config_final.merge(config_cli_and_env);

        auto* acc = get_acc_cl();
        if (!acc) {
            std::exit(1);
        }

        Status status = acc->init();
        if (status != Status::Ok) {
            std::exit(1);
        }

        return ConfigStatus::Ok;
    }
}// namespace spla
