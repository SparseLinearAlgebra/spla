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

#include <ctime>
#include <random>

int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = std::make_shared<cxxopts::Options>("pi", "compute Pi number using Monte-Carlo algorithm with spla library");
    options->add_option("", cxxopts::Option("h,help", "display help info", cxxopts::value<bool>()->default_value("false")));
    options->add_option("", cxxopts::Option("n", "number of samples to generate", cxxopts::value<int>()->default_value("1000000")));
    options->add_option("", cxxopts::Option("cpu", "force cpu for computations", cxxopts::value<bool>()->default_value("false")));
    cxxopts::ParseResult args;
    int                  ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options";
        return ret;
    }

    const int n = args["n"].as<int>();

    auto gen     = spla::Vector::make(n, spla::INT);
    auto samples = spla::Vector::make(n, spla::INT);
    auto count   = spla::Scalar::make(spla::INT);
    auto seed    = std::time(nullptr);

    auto is_in_unit_circle = spla::OpUnary::make_int(
            "is_in_unit_circle",
            "(int seed) { "
            "   const float x = random_float(seed + 0) * 2.0f - 1.0f;"
            "   const float y = random_float(seed + 1) * 2.0f - 1.0f;"
            "   return x * x + y * y <= 1.0f ? 1: 0;"
            "}",
            [engine = std::default_random_engine(seed),
             dist   = std::uniform_real_distribution<float>(-1.0f, 1.0f)](int) mutable -> int {
                const float x = dist(engine);
                const float y = dist(engine);
                return x * x + y * y <= 1.0f ? 1 : 0;
            });

    gen->fill_noize(seed);

    spla::Library::get()->set_force_no_acceleration(args["cpu"].as<bool>());
    spla::exec_v_map(samples, gen, is_in_unit_circle);
    spla::exec_v_reduce(count, spla::Scalar::make_int(0), samples, spla::PLUS_INT);

    std::cout << "Pi is roughly " << (4.0f * count->as_float()) / float(n) << std::endl;

    return 0;
}