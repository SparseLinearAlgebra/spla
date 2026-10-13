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

#include <spla.h>
#include <spla.hpp>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

    constexpr const char* CHECK_TAG = "[spla:check]";

    bool has_flag(int argc, char** argv, const char* flag) {
        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], flag) == 0) return true;
        }
        return false;
    }

    spla_Status configure_without(int argc, char** argv, const char* flag) {
        std::vector<char*> args;
        args.reserve(static_cast<size_t>(argc));

        for (int i = 0; i < argc; ++i) {
            if (i > 0 && std::strcmp(argv[i], flag) == 0) continue;
            args.push_back(argv[i]);
        }

        return spla_Configure(static_cast<int>(args.size()), args.data());
    }

    void print_usage() {
        std::cout << "spla - sparse linear algebra library\n\n"
                  << "Usage:\n"
                  << "  spla --softcheck              check that the environment is ready to work\n"
                  << "  spla [options]                apply configuration and exit\n\n"
                  << "Configuration options:\n"
                  << "  --spla-config <name>          configuration to apply (default: \"default\")\n"
                  << "  --spla-config-file <path>     explicitly specified configuration file\n"
                  << "  --spla-backend <type>         gpu | cpu | any | by_index\n"
                  << "  --spla-help                   show configuration help\n"
                  << "  --spla-version                show library version\n";
    }

    /**
     * Runs a sanity check of the installed environment:
     *   1. applies configuration (default config file, system/user files, env, cli);
     *   2. initializes OpenCL and selects a platform and device;
     *   3. runs a simple test task (2 + 3) and verifies the result.
     *
     * @return Exit code: 0 if everything is ready, 1 otherwise.
     */
    int softcheck(int argc, char** argv) {
        using namespace spla;

        std::cout << CHECK_TAG << " Starting sanity check..." << std::endl;

        if (configure_without(argc, argv, "--softcheck") != SPLA_STATUS_OK) {
            std::cout << CHECK_TAG << " FAILED - invalid configuration" << std::endl;
            return 1;
        }

        std::cout << CHECK_TAG << " Initializing OpenCL..." << std::endl;

        Library::get()->set_default_callback();

        if (Library::get()->set_accelerator(AcceleratorType::OpenCL) != Status::Ok) {
            std::cout << CHECK_TAG << " FAILED - no OpenCL platform/device available" << std::endl;
            return 1;
        }

        std::string info;
        Library::get()->get_accelerator_info(info);
        std::cout << CHECK_TAG << " Selected accelerator: " << info << std::endl;

        auto a = Vector::make(1, INT);
        auto b = Vector::make(1, INT);
        auto c = Vector::make(1, INT);

        if (!a || !b || !c) {
            std::cout << CHECK_TAG << " FAILED - cannot allocate test vectors" << std::endl;
            return 1;
        }

        a->set_int(0, 2);
        b->set_int(0, 3);

        std::cout << CHECK_TAG << " Running test task: 2 + 3 = ..." << std::endl;

        if (exec_v_eadd(c, a, b, PLUS_INT) != Status::Ok) {
            std::cout << CHECK_TAG << " FAILED - test task execution error" << std::endl;
            return 1;
        }

        T_INT result = 0;
        c->get_int(0, result);

        std::cout << CHECK_TAG << " Result: " << result << " (expected 5)" << std::endl;

        if (result != 5) {
            std::cout << CHECK_TAG << " FAILED - unexpected result" << std::endl;
            return 1;
        }

        Library::get()->finalize();
        std::cout << CHECK_TAG << " SUCCESS - spla works correctly" << std::endl;
        return 0;
    }

}// namespace

int main(int argc, char** argv) {
    if (has_flag(argc, argv, "--softcheck")) {
        return softcheck(argc, argv);
    }

    const bool wants_help    = has_flag(argc, argv, "--spla-help");
    const bool wants_version = has_flag(argc, argv, "--spla-version");

    const spla_Status status = spla_Configure(argc, argv);

    if (wants_help || wants_version) return 0;
    if (status != SPLA_STATUS_OK) return 1;

    print_usage();
    return 0;
}
