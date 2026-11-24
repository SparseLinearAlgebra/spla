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

#include "common.hpp"
#include "options.hpp"

#include <iostream>
#include <spla.hpp>

int main(int argc, const char* const* argv) {
    auto options = make_options(
            "mst", "Boruvka's Minimum Spanning Tree algorithm with spla library");

    cxxopts::ParseResult args;
    int                  ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options" << std::endl;
        return ret;
    }

    spla::Timer     timer_total;
    spla::Timer     timer_gpu;
    spla::Timer     timer_cpu;
    spla::Timer     timer_ref;
    spla::MtxLoader loader;

    timer_total.start();

    if (!loader.load(args["mtxpath"].as<std::string>())) {
        std::cerr << "failed to load graph";
        return 1;
    }

    std::string    acc_info;
    spla::Library* library = spla::Library::get();

    library->set_platform(args["platform"].as<int>());
    library->set_device(args["device"].as<int>());
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);
    std::cout << "env: " << acc_info << std::endl;

    const spla::uint N = loader.get_n_rows();
    auto             S = spla::Matrix::make(N, N, spla::PAIR);

    const auto& Ai = loader.get_Ai();
    const auto& Aj = loader.get_Aj();
    const auto& Aw = loader.get_Aw();

    for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
        S->set_pair(Ai[k], Aj[k], spla::T_PAIR(Aw[k], Aj[k]));
    }

    auto T_gpu = spla::Matrix::make(N, N, spla::FLOAT);
    auto T_cpu = spla::Matrix::make(N, N, spla::FLOAT);

    auto desc = spla::Descriptor::make();

    const int n_iters = args["niters"].as<int>();

    double total_weight_gpu = 0.0;
    double total_weight_cpu = 0.0;

    if (args["run-cpu"].as<bool>()) {
        library->set_force_no_acceleration(true);

        for (int i = 0; i < n_iters; ++i) {
            T_cpu->clear();
            S = spla::Matrix::make(N, N, spla::PAIR);
            for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
                S->set_pair(Ai[k], Aj[k], spla::T_PAIR(Aw[k], Aj[k]));
            }
            timer_cpu.lap_begin();
            spla::mst(T_cpu, S, desc, nullptr);
            timer_cpu.lap_end();
        }

        total_weight_cpu = 0;
        for (spla::uint i = 0; i < N; ++i) {
            for (spla::uint j = i + 1; j < N; ++j) {
                float w;
                T_cpu->get_float(i, j, w);
                if (w != 0.0) {
                    total_weight_cpu += w;
                }
            }
        }

        std::cout << "CPU MST total weight: " << total_weight_cpu << std::endl;
    }

    if (args["run-gpu"].as<bool>()) {
        library->set_force_no_acceleration(false);

        for (int i = 0; i < n_iters; ++i) {
            T_gpu->clear();
            S = spla::Matrix::make(N, N, spla::PAIR);
            for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
                S->set_pair(Ai[k], Aj[k], spla::T_PAIR(Aw[k], Aj[k]));
            }
            timer_gpu.lap_begin();
            spla::mst(T_gpu, S, desc, nullptr);
            timer_gpu.lap_end();
        }

        total_weight_gpu = 0;
        for (spla::uint i = 0; i < N; ++i) {
            for (spla::uint j = i + 1; j < N; ++j) {
                float w;
                T_gpu->get_float(i, j, w);
                if (w != 0.0) {
                    total_weight_gpu += w;
                }
            }
        }

        std::cout << "GPU MST total weight: " << total_weight_gpu << std::endl;
    }

    spla::Library::get()->finalize();

    timer_total.stop();

    std::cout << "\n=== Timing Results ===" << std::endl;
    std::cout << "total(ms):" << timer_total.get_elapsed_ms() << std::endl;
    std::cout << "cpu(ms): ";
    timer_cpu.print();
    std::cout << std::endl;
    std::cout << "gpu(ms): ";
    timer_gpu.print();
    std::cout << std::endl;

    return 0;
}