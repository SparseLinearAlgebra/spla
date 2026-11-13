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

#include <spla.hpp>

#include <sstream>

static std::vector<spla::uint> parse_sources(const std::string& s) {
    std::vector<spla::uint> sources;
    std::stringstream       ss(s);
    std::string             token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) sources.push_back(static_cast<spla::uint>(std::stoi(token)));
    }
    return sources;
}

int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = make_options("msl_bfs", "msl_bfs (multi-source bfs levels) algorithm with spla library");
    cxxopts::ParseResult              args;
    int                               ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options";
        return ret;
    }

    spla::Timer     timer;
    spla::Timer     timer_cpu;
    spla::Timer     timer_gpu;
    spla::Timer     timer_ref;
    spla::MtxLoader loader;

    timer.start();

    if (!loader.load(args[OPT_MTXPATH].as<std::string>())) {
        std::cerr << "failed to load graph";
        return 1;
    }

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);
    std::cout << "env: " << acc_info << std::endl;

    const spla::uint              N       = loader.get_n_rows();
    const std::vector<spla::uint> sources = parse_sources(args[OPT_SOURCES].as<std::string>());
    const spla::uint              K       = static_cast<spla::uint>(sources.size());

    spla::ref_ptr<spla::Matrix> S_cpu = spla::Matrix::make(K, N, spla::INT);
    spla::ref_ptr<spla::Matrix> S_acc = spla::Matrix::make(K, N, spla::INT);
    spla::ref_ptr<spla::Matrix> A     = spla::Matrix::make(N, N, spla::INT);

    const auto& Ai = loader.get_Ai();
    const auto& Aj = loader.get_Aj();

    for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
        A->set_int(Ai[k], Aj[k], 1);
    }

    const int n_iters = args[OPT_NITERS].as<int>();

    if (args[OPT_RUN_CPU].as<bool>()) {
        library->set_force_no_acceleration(true);

        for (int i = 0; i < n_iters; ++i) {
            S_cpu->clear();

            timer_cpu.lap_begin();
            spla::msl_bfs(S_cpu, A, sources);
            timer_cpu.lap_end();
        }
    }

    if (args[OPT_RUN_GPU].as<bool>()) {
        library->set_force_no_acceleration(false);

        for (int i = 0; i < n_iters; ++i) {
            S_acc->clear();

            timer_gpu.lap_begin();
            spla::msl_bfs(S_acc, A, sources);
            timer_gpu.lap_end();
        }
    }

    const std::string bench_path = args[OPT_BENCH].as<std::string>();
    if (!bench_path.empty()) {
        const std::string dataset = std::filesystem::path(args[OPT_MTXPATH].as<std::string>()).stem();
        write_bench_entry(bench_path, "MSL_BFS", dataset, timer_gpu);
    }

    if (args[OPT_RUN_REF].as<bool>()) {
        std::vector<std::vector<spla::uint>> ref_A(N, std::vector<spla::uint>());
        for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
            ref_A[Ai[k]].push_back(Aj[k]);
        }

        std::vector<std::vector<int>> ref_S;

        timer_ref.lap_begin();
        spla::msl_bfs_naive(ref_S, ref_A, sources, spla::ref_ptr<spla::Descriptor>());
        timer_ref.lap_end();

        auto verify = [&](const char* where, const spla::ref_ptr<spla::Matrix>& S) {
            for (spla::uint t = 0; t < K; ++t) {
                for (spla::uint j = 0; j < N; ++j) {
                    int actual;
                    S->get_int(t, j, actual);
                    if (actual != ref_S[t][j]) {
                        std::cerr << " VERIFY " << where << ": [" << t << "][" << j << "] expected " << ref_S[t][j] << " actual " << actual << std::endl;
                        assert(false);
                    }
                }
            }
        };

        if (args[OPT_RUN_CPU].as<bool>()) verify("cpu", S_cpu);
        if (args[OPT_RUN_GPU].as<bool>()) verify("acc", S_acc);
    }

    spla::Library::get()->finalize();

    timer.stop();

    std::cout << "total(ms): " << timer.get_elapsed_ms() << std::endl;
    std::cout << "cpu(ms): ";
    timer_cpu.print();
    std::cout << std::endl;
    std::cout << "gpu(ms): ";
    timer_gpu.print();
    std::cout << std::endl;
    std::cout << "ref(ms): ";
    timer_ref.print();
    std::cout << std::endl;

    return 0;
}
