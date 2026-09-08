#pragma once

#include <optional>
#include <string>

#include "CLI/CLI.hpp"
#include <nlohmann/json.hpp>

#ifdef _WIN32
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    #include <pwd.h>
    #include <unistd.h>
#endif

namespace spla {

    struct Config {
        std::optional<bool>        help;
        std::optional<bool>        version;
        std::optional<std::string> system_config_path;
        std::optional<std::string> user_config_path;
        std::optional<int>         platform;
        std::optional<int>         device;
        std::optional<int>         queues;
        std::optional<bool>        profiling;
        std::optional<std::string> allocator;
        std::optional<size_t>      allocator_size;
        std::optional<int>         verbosity;

        void merge(const Config& source);
        void reset();
    };

    enum ConfigStatus {
        Ok,
        HelpRequested,
        VersionRequested,

        CliOrEnvParseError,
        UserOrSystemConfParseError,
        OpenFileError,

        MissedParametrs,
        PlatformNotFound,
        DeviceNotFound,
        InvalidConfigParams
    };

    extern Config config_user_and_system;
    extern Config config_cli_and_env;
    extern Config config_final;

    std::string  get_home_directory();
    std::string  get_default_system_config_path();
    std::string  get_default_user_config_path();
    std::string  find_first_json_file(const std::string& directory);
    ConfigStatus load_from_file(const std::string& path, Config& cfg);

    ConfigStatus parse_system_and_user_conf(const Config& cli_env_config, Config& file_config);

    std::string  get_spla_version();
    ConfigStatus parse_cli_and_env(int argc, char** argv, Config& cfg);

    ConfigStatus check_platform_and_device(int platform_index, int device_index);
    ConfigStatus validate(const Config& cfg);

    ConfigStatus configure(int argc, char** argv);

}// namespace spla