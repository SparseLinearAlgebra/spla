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

#include <CLI/CLI.hpp>
#include <spla.h>
#include <spla.hpp>

#include <iostream>
#include <string>

namespace {

    const char* CHECK_TAG = "[spla:check]";

    /**
     * Runs a sanity check of the already configured environment:
     *   1. verifies the accelerator initialization status;
     *   2. runs a simple test task (2 + 3);
     *   3. verifies the result.
     *
     * @return Exit code: 0 if everything is ready, 1 otherwise.
     */
    int softcheck() {
        using namespace spla;

        std::cout << CHECK_TAG << " Initializing..." << std::endl;

        Library::get()->set_default_callback();

        if (spla_Library_initialize() != SPLA_STATUS_OK) {
            std::cout << CHECK_TAG << " FAILED - no OpenCL platform/device available" << std::endl;
            return 1;
        }

        std::string info;
        Library::get()->get_accelerator_info(info);
        std::cout << CHECK_TAG << " Selected accelerator: " << info << std::endl;

        auto a = Vector::make(1, spla::INT);
        auto b = Vector::make(1, spla::INT);
        auto c = Vector::make(1, spla::INT);

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

        spla::T_INT result = 0;
        c->get_int(0, result);
        std::cout << CHECK_TAG << " Result: " << result << " (expected 5)" << std::endl;

        if (result != 5) {
            std::cout << CHECK_TAG << " FAILED - wrong result" << std::endl;
            return 1;
        }

        std::cout << CHECK_TAG << " SUCCESS - spla works correctly" << std::endl;
        return 0;
    }

}// namespace

int main(int argc, char** argv) {

    bool do_softcheck      = false;

    CLI::App app{"spla - sparse linear algebra library"};
    app.description("Command-line tool for SPLA configuration and environment checks.");
    app.add_flag("--softcheck", do_softcheck, "Run environment sanity check");

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp& e) {
        std::cout << app.help() << std::endl;
        return 0;
    } catch (const CLI::ParseError& e) {
        std::cerr << "[spla] ERROR: " << e.what() << std::endl
                  << std::endl;
        std::cout << app.help() << std::endl;
        return 1;
    }

    if (do_softcheck) {
        std::cout << CHECK_TAG << " Starting sanity check..." << std::endl;
    }

    if (spla_Configure(0, nullptr) != SPLA_STATUS_OK) {
        std::cout << CHECK_TAG << " FAILED - invalid configuration" << std::endl;
        return 1;
    }

    if (do_softcheck) {
        return softcheck();
    }

    std::cout << CHECK_TAG << " Configuration applied successfully" << std::endl;
    return 0;
}
