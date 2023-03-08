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


int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = make_options("tc", "tc (triangles counting) algorithm with spla library");
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

    spla::Library* library = spla::Library::get();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);

    const spla::uint                N          = loader.get_n_rows();
    int                             ntrins_cpu = -1;
    int                             ntrins_acc = -1;
    spla::ref_ptr<spla::Matrix>     B_cpu      = spla::Matrix::make(N, N, spla::INT);
    spla::ref_ptr<spla::Matrix>     B_acc      = spla::Matrix::make(N, N, spla::INT);
    spla::ref_ptr<spla::Matrix>     A          = spla::Matrix::make(N, N, spla::INT);
    spla::ref_ptr<spla::Descriptor> desc       = spla::Descriptor::make();

    desc->set_traversal_mode(static_cast<spla::Descriptor::TraversalMode>(args[OPT_PUSH_PULL].as<int>() - 1));
    desc->set_front_factor(args[OPT_FRONT_FACTOR].as<float>());

    const auto& Ai = loader.get_Ai();
    const auto& Aj = loader.get_Aj();

    for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
        if (Ai[k] > Aj[k]) {
            A->set_int(Ai[k], Aj[k], 1);
        }
    }

    const int n_iters = args[OPT_NITERS].as<int>();

    if (args[OPT_RUN_CPU].as<bool>()) {
        library->set_force_no_acceleration(true);

        for (int i = 0; i < n_iters; ++i) {
            B_cpu->clear();

            timer_cpu.lap_begin();
            spla::tc(ntrins_cpu, A, B_cpu, desc);
            timer_cpu.lap_end();
        }
    }

    if (args[OPT_RUN_GPU].as<bool>()) {
        library->set_force_no_acceleration(false);

        for (int i = 0; i < n_iters; ++i) {
            B_acc->clear();

            timer_gpu.lap_begin();
            spla::tc(ntrins_acc, A, B_acc, desc);
            timer_gpu.lap_end();
        }
    }

    if (args[OPT_RUN_REF].as<bool>()) {
        std::vector<int>                     ref_v(N);
        std::vector<std::vector<spla::uint>> ref_A(N, std::vector<spla::uint>());

        for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
            if (Ai[k] > Aj[k]) {
                ref_A[Ai[k]].push_back(Aj[k]);
            }
        }

        int ntrins_ref = -1;

        timer_ref.lap_begin();
        spla::tc_naive(ntrins_ref, ref_A, desc);
        timer_ref.lap_end();

        if (args[OPT_RUN_CPU].as<bool>()) verify_exact(ntrins_ref, ntrins_cpu);
        if (args[OPT_RUN_GPU].as<bool>()) verify_exact(ntrins_ref, ntrins_acc);
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