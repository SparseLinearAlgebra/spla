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

#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "config_status.hpp"
#include <nlohmann/json.hpp>

#ifdef _WIN32
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    #include <pwd.h>
    #include <unistd.h>
#endif

namespace spla {

    struct Configuration {
        std::optional<std::vector<std::string>> extends;

        std::optional<std::string> backend;
        std::optional<int>         platform_index;
        std::optional<int>         device_index;
        std::optional<std::string> if_opencl_unavailable;

        std::optional<int>         queues_count;
        std::optional<bool>        profiling;
        std::optional<std::string> allocator_type;
        std::optional<size_t>      linear_alloc_size;
        std::optional<int>         verbosity;

        void merge(const Configuration& source);
        void reset();
    };

    struct ConfigFile {
        std::optional<std::map<std::string, Configuration>> configs;

        void merge(const ConfigFile& source);
        void reset();
    };

    struct CliMeta {
        std::optional<bool>        version;
        std::optional<std::string> configuration;
        std::optional<std::string> path_to_conf;
        void                       reset();
    };

    extern ConfigFile    config_default;
    extern ConfigFile    config_system;
    extern ConfigFile    config_user;
    extern ConfigFile    config_explicit;
    extern CliMeta       config_cli_meta;
    extern Configuration config_cli_and_env;
    extern Configuration config_final;
    extern bool          config_initialized;
    extern bool          config_explicitly_configured;

    std::string get_spla_version();

    std::string get_default_config_path();
    std::string get_system_config_path();
    std::string get_home_directory();
    std::string get_user_config_path();

    ConfigStatus parse_cli_and_env(int argc, char** argv, CliMeta& meta, Configuration& cfg);
    ConfigStatus parse_file(const std::string& path, ConfigFile& cfg, bool allow_default = false);

    Configuration apply_extends(const std::map<std::string, Configuration>& configs,
                                const std::string&                          name,
                                std::set<std::string>&                      stack);
    ConfigStatus  apply_configuration(const ConfigFile& file, const std::string& name, Configuration& cfg);

    ConfigStatus validation(const Configuration& cfg);

    ConfigStatus configure(int argc, char** argv, bool explicit_request = true);

}// namespace spla
