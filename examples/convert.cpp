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

#include "options.hpp"

#include <spla.hpp>

int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = std::make_shared<cxxopts::Options>("convert", "aux tool to convert .mtx graph in desired format");
    options->add_option("", cxxopts::Option("h,help", "display help info", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option("in", "path to matrix file", cxxopts::value<std::string>()));
    options->add_option("", cxxopts::Option("out", "path to save matrix file", cxxopts::value<std::string>()));
    options->add_option("", cxxopts::Option("offset_indices", "offset indices to start from 0", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option("make_undirected", "make graph undirected adding backward edges", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option("remove_loops", "remove self-loops", cxxopts::value<bool>()->default_value("true")));
    options->add_option("", cxxopts::Option("stats_only", "collect only graphs stats", cxxopts::value<bool>()->default_value("false")));
    cxxopts::ParseResult args;
    int                  ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options";
        return ret;
    }

    spla::Timer     timer;
    spla::MtxLoader loader;

    timer.start();

    bool offset_indices  = args["offset_indices"].as<bool>();
    bool make_undirected = args["make_undirected"].as<bool>();
    bool remove_loops    = args["remove_loops"].as<bool>();
    bool stats_only      = args["stats_only"].as<bool>();

    if (!loader.load(args["in"].as<std::string>(), offset_indices, make_undirected, remove_loops)) {
        std::cerr << "failed to load graph";
        return 1;
    }

    if (!loader.save(args["out"].as<std::string>(), stats_only)) {
        std::cerr << "failed to save graph";
        return 1;
    }

    timer.stop();

    std::cout << "total(ms): " << timer.get_elapsed_ms() << std::endl;

    return 0;
}