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

#include <spla/spla.hpp>


int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = make_options("bfs", "bfs (breadth first search) algorithm with spla library");
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

    spla::Library* library = spla::get_library();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);

    const spla::uint                N     = loader.get_n_rows();
    const spla::uint                s     = args[OPT_SOURCE].as<int>();
    spla::ref_ptr<spla::Vector>     v_cpu = spla::make_vector(N, spla::INT);
    spla::ref_ptr<spla::Vector>     v_acc = spla::make_vector(N, spla::INT);
    spla::ref_ptr<spla::Matrix>     A     = spla::make_matrix(N, N, spla::INT);
    spla::ref_ptr<spla::Descriptor> desc  = spla::make_desc();

    desc->set_push_only(args[OPT_PUSH].as<bool>());
    desc->set_pull_only(args[OPT_PULL].as<bool>());
    desc->set_push_pull(args[OPT_PUSH_PULL].as<bool>());
    desc->set_front_factor(args[OPT_FRONT_FACTOR].as<float>());
    desc->set_discovered_factor(args[OPT_DISCOVERED_FACTOR].as<float>());

    const auto& Ai = loader.get_Ai();
    const auto& Aj = loader.get_Aj();

    for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
        A->set_int(Ai[k], Aj[k], 1);
    }

    const int n_iters = args[OPT_NITERS].as<int>();

    if (args[OPT_RUN_CPU].as<bool>()) {
        library->set_force_no_acceleration(true);

        for (int i = 0; i < n_iters; ++i) {
            v_cpu->clear();

            timer_cpu.lap_begin();
            spla::bfs(v_cpu, A, s, desc);
            timer_cpu.lap_end();
        }
    }

    if (args[OPT_RUN_GPU].as<bool>()) {
        library->set_force_no_acceleration(false);

        for (int i = 0; i < n_iters; ++i) {
            v_acc->clear();

            timer_gpu.lap_begin();
            spla::bfs(v_acc, A, s, desc);
            timer_gpu.lap_end();
        }
    }

    if (args[OPT_RUN_REF].as<bool>()) {
        std::vector<int>                     ref_v(N);
        std::vector<std::vector<spla::uint>> ref_A(N, std::vector<spla::uint>());

        for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
            ref_A[Ai[k]].push_back(Aj[k]);
        }

        timer_ref.lap_begin();
        spla::bfs_naive(ref_v, ref_A, s, spla::ref_ptr<spla::Descriptor>());
        timer_ref.lap_end();

        if (args[OPT_RUN_CPU].as<bool>()) verify_exact(v_cpu, ref_v);
        if (args[OPT_RUN_GPU].as<bool>()) verify_exact(v_acc, ref_v);
    }

    spla::get_library()->finalize();

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