#include "cl_configure.hpp"

namespace spla {

    Config config_user_and_system;
    Config config_cli_and_env;
    Config config_final;

    void Config::merge(const Config& source) {
        if (source.platform.has_value()) platform = source.platform;
        if (source.device.has_value()) device = source.device;
        if (source.queues.has_value()) queues = source.queues;
        if (source.verbosity.has_value()) verbosity = source.verbosity;
        if (source.allocator.has_value()) allocator = source.allocator;
        if (source.allocator_size.has_value()) allocator_size = source.allocator_size;
        if (source.profiling.has_value()) profiling = source.profiling;
        if (source.system_config_path.has_value()) system_config_path = source.system_config_path;
        if (source.user_config_path.has_value()) user_config_path = source.user_config_path;
        if (source.help.has_value()) help = source.help;
        if (source.version.has_value()) version = source.version;
    }

    void Config::reset() {
        *this = Config{};
    }

    bool Config::has_all_required() const {
        return platform.has_value() &&
               device.has_value() &&
               queues.has_value() &&
               profiling.has_value() &&
               allocator.has_value() &&
               allocator_size.has_value() &&
               verbosity.has_value();
    }

    std::string get_spla_version() {
        return "SPLA version: 0.0.0";
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

    ConfigStatus load_from_file(const std::string& path) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << path << std::endl;
                return ConfigStatus::FileError;
            }

            nlohmann::json config_data = nlohmann::json::parse(file);

            if (config_data.contains("platform")) {
                config_user_and_system.platform = config_data["platform"].get<int>();
            }
            if (config_data.contains("device")) {
                config_user_and_system.device = config_data["device"].get<int>();
            }
            if (config_data.contains("queues")) {
                config_user_and_system.queues = config_data["queues"].get<int>();
            }
            if (config_data.contains("verbosity")) {
                config_user_and_system.verbosity = config_data["verbosity"].get<int>();
            }
            if (config_data.contains("profiling")) {
                config_user_and_system.profiling = config_data["profiling"].get<bool>();
            }
            if (config_data.contains("allocator")) {
                config_user_and_system.allocator = config_data["allocator"].get<std::string>();
            }
            if (config_data.contains("allocator_size")) {
                config_user_and_system.allocator_size = config_data["allocator_size"].get<size_t>();
            }

            return ConfigStatus::Ok;

        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Error parsing JSON file '" << path << "': " << e.what() << std::endl;
            return ConfigStatus::UserOrSystemConfParseError;
        }
    }

    ConfigStatus parse_system_and_user_conf() {
        std::string sys_dir  = config_cli_and_env.system_config_path.value_or(get_default_system_config_path());
        std::string user_dir = config_cli_and_env.user_config_path.value_or(get_default_user_config_path());

        bool file_loaded = false;

        if (sys_dir != "") {
            std::string sys_file = find_first_json_file(sys_dir);
            if (sys_file != "") {
                std::cout << "Loading system config: " << sys_file << std::endl;
                load_from_file(sys_file);
                file_loaded = true;
            }
        }

        if (user_dir != "") {
            std::string user_file = find_first_json_file(user_dir);
            if (user_file != "") {
                std::cout << "Loading user config: " << user_file << std::endl;
                load_from_file(user_file);
                file_loaded = true;
            }
        }

        if (!file_loaded) {
            std::cerr << "Warning: No JSON configuration files found." << std::endl;
        }

        return ConfigStatus::Ok;
    }

    ConfigStatus parse_cli_and_env(int argc, char** argv) {
        CLI::App app{"SPLA configuration"};

        app.add_flag("-sh,--spla-help", config_cli_and_env.help, "Show help and exit");
        app.add_flag("-sv,--spla-version", config_cli_and_env.version, "Show version and exit");

        app.add_option("-ss,--spla-sconf", config_cli_and_env.system_config_path, "Path to system configuration file")
                ->envname("SPLA_SYSTEM_CONFIG_PATH");

        app.add_option("-su,--spla-uconf", config_cli_and_env.user_config_path, "Path to user configuration file")
                ->envname("SPLA_USER_CONFIG_PATH");

        app.add_option("-sp,--spla-platform", config_cli_and_env.platform, "OpenCL platform index")
                ->envname("SPLA_OPENCL_PLATFORM")
                ->check(CLI::PositiveNumber);

        app.add_option("-sd,--spla-device", config_cli_and_env.device, "OpenCL device index")
                ->envname("SPLA_OPENCL_DEVICE")
                ->check(CLI::PositiveNumber);

        app.add_option("-sq,--spla-queues", config_cli_and_env.queues, "Number of command queues")
                ->envname("SPLA_QUEUES")
                ->check(CLI::PositiveNumber);

        app.add_flag("-pr,--spla-profiling", config_cli_and_env.profiling, "Enable profiling of command queues")
                ->envname("SPLA_PROFILING");

        app.add_option("-sa,--spla-allocator", config_cli_and_env.allocator, "Allocator type: linear or general")
                ->envname("SPLA_ALLOCATOR")
                ->check(CLI::IsMember({"linear", "general"}));

        app.add_option("-sS,--spla-allocator-size", config_cli_and_env.allocator_size, "Linear allocator size in bytes")
                ->envname("SPLA_ALLOCATOR_SIZE")
                ->check(CLI::PositiveNumber);

        app.add_option("-sV,--spla-verbosity", config_cli_and_env.verbosity, "Verbosity level (0-3)")
                ->envname("SPLA_VERBOSITY")
                ->check(CLI::Range(0, 3));

        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError& e) {
            app.exit(e);
            return ConfigStatus::CliOrEnvParseError;
        }

        if (config_cli_and_env.help) {
            std::cout << app.help() << std::endl;
            return ConfigStatus::HelpRequested;
        }

        if (config_cli_and_env.version) {
            std::cout << get_spla_version() << std::endl;
            return ConfigStatus::VersionRequested;
        }

        return ConfigStatus::Ok;
    }

    ConfigStatus validate() {

        if (!config_final.has_all_required()) {
            std::cerr << "Error: Missing required configuration parameters" << std::endl;
            std::exit(1);
        }

        return ConfigStatus::Ok;
    }

    ConfigStatus apply() {
        return ConfigStatus::Ok;
    }

    ConfigStatus configure(int argc, char** argv) {
        config_user_and_system.reset();
        config_cli_and_env.reset();
        config_final.reset();

        ConfigStatus status = parse_cli_and_env(argc, argv);

        if (status == ConfigStatus::HelpRequested || status == ConfigStatus::VersionRequested) {
            return status;
        }

        if (status != ConfigStatus::Ok) {
            return status;
        }

        status = parse_system_and_user_conf();
        if (status != ConfigStatus::Ok) {
            return status;
        }

        config_final = config_user_and_system;
        config_final.merge(config_cli_and_env);

        status = validate();
        if (status != ConfigStatus::Ok) {
            return status;
        }

        status = apply();
        if (status != ConfigStatus::Ok) {
            return status;
        }

        return ConfigStatus::Ok;
    }
}// namespace spla