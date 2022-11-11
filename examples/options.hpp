/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
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

#ifndef SPLA_OPTIONS_HPP
#define SPLA_OPTIONS_HPP

#include <cxxopts.hpp>

#define OPT_MTXPATH           "mtxpath"
#define OPT_NITERS            "niters"
#define OPT_SOURCE            "source"
#define OPT_RUN_REF           "run-ref"
#define OPT_RUN_CPU           "run-cpu"
#define OPT_RUN_GPU           "run-gpu"
#define OPT_PLATFORM          "platform"
#define OPT_DEVICE            "device"
#define OPT_PUSH              "push"
#define OPT_PULL              "pull"
#define OPT_PUSH_PULL         "push-pull"
#define OPT_FRONT_FACTOR      "front-factor"
#define OPT_DISCOVERED_FACTOR "discovered-factor"

std::shared_ptr<cxxopts::Options> make_options(const std::string& name, const std::string& desc) {
    std::shared_ptr<cxxopts::Options> options = std::make_shared<cxxopts::Options>(name, desc);
    options->add_option("", cxxopts::Option("h,help", "display help info", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option(OPT_MTXPATH, "path to matrix file", cxxopts::value<std::string>()));
    options->add_option("", cxxopts::Option(OPT_NITERS, "number of iterations to run", cxxopts::value<int>()->default_value("4")));
    options->add_option("", cxxopts::Option(OPT_SOURCE, "source vertex to run", cxxopts::value<int>()->default_value("0")));
    options->add_option("", cxxopts::Option(OPT_RUN_REF, "check validity running naive version", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option(OPT_RUN_CPU, "run algo with cpu backend", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option(OPT_RUN_GPU, "run algo with gpu (acc) backend", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option("undirected", "force graph to be undirected", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option(OPT_PLATFORM, "id of platform to run", cxxopts::value<int>()->default_value("0")));
    options->add_option("", cxxopts::Option(OPT_DEVICE, "id of device to run", cxxopts::value<int>()->default_value("0")));
    options->add_option("", cxxopts::Option("verbose", "verbose std output", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option("debug-timing", "timing for each iteration of algorithm", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option(OPT_PUSH, "run traversal in push only", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option(OPT_PULL, "run traversal in pull only", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option(OPT_PUSH_PULL, "run traversal in adaptive push-pull", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option(OPT_FRONT_FACTOR, "adaptive push-pull front sparsity factor", cxxopts::value<float>()->default_value("0.05")));
    options->add_option("", cxxopts::Option(OPT_DISCOVERED_FACTOR, "adaptive push-pull discovered sparsity factor", cxxopts::value<float>()->default_value("0.7")));
    return options;
}

bool parse_options(int argc, const char* const* argv, const std::shared_ptr<cxxopts::Options>& options, cxxopts::ParseResult& args, int& ret) {
    ret = 0;

    try {
        args = options->parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "failed parse input arguments: " << e.what();
        ret = 1;
        return true;
    }

    if (args["help"].as<bool>()) {
        std::cout << options->help();
        return true;
    }

    return false;
}

#endif//SPLA_OPTIONS_HPP
