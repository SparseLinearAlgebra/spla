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

#include <cxxopts.hpp>

#include <spla-cpp/Spla.hpp>

int main(int argc, const char *const *argv) {
    cxxopts::Options options("spla-data-tool", "Graph data processing tool");
    options.add_option("", cxxopts::Option("h,help", "display help info", cxxopts::value<bool>()->default_value("false")));
    options.add_option("", cxxopts::Option("mtxpath", "path to matrix file", cxxopts::value<std::string>()));
    options.add_option("", cxxopts::Option("undirected", "force graph to be undirected", cxxopts::value<bool>()->default_value("true")));
    options.add_option("", cxxopts::Option("remove-loops", "remove self loops", cxxopts::value<bool>()->default_value("true")));
    options.add_option("", cxxopts::Option("verbose", "verbose std output", cxxopts::value<bool>()->default_value("true")));
    cxxopts::ParseResult args;

    try {
        args = options.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "Failed parse input arguments: " << e.what();
        return 1;
    }

    if (args["help"].as<bool>()) {
        std::cout << options.help();
        return 0;
    }

    std::string mtxpath;
    bool undirected;
    bool removeLoops;
    bool ignoreValues = true;
    bool verbose;

    try {
        mtxpath = args["mtxpath"].as<std::string>();
        undirected = args["undirected"].as<bool>();
        removeLoops = args["remove-loops"].as<bool>();
        verbose = args["verbose"].as<bool>();
    } catch (const std::exception &e) {
        std::cerr << "Invalid input arguments: " << e.what();
        return 1;
    }

    // Load data
    spla::MatrixLoader<std::int32_t> loader;

    try {
        loader.Load(mtxpath, undirected, removeLoops, ignoreValues, verbose);
    } catch (const std::exception &e) {
        std::cerr << "Failed load matrix: " << e.what();
        return 1;
    }

    std::string output = mtxpath + ".dt" + (undirected ? ".un" : "") + (removeLoops ? ".rl" : "");
    std::fstream file(output, std::ios::out);
    loader.Save(file);

    return 0;
}