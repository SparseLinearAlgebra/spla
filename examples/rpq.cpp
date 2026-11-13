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

#include <filesystem>
#include <fstream>
#include <set>

namespace {

    // Sorted list of .mtx files in a directory.
    std::vector<std::string> list_mtx(const std::string& dir) {
        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".mtx") {
                files.push_back(entry.path().filename().string());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    // Load a Matrix-Market adjacency matrix (1-based -> 0-based, directed) into an INT matrix.
    spla::ref_ptr<spla::Matrix> load_adjacency(const std::string& path) {
        spla::MtxLoader loader;
        if (!loader.load(path, true, false, false)) {
            std::cerr << "failed to load " << path << std::endl;
            exit(1);
        }

        const spla::uint n = loader.get_n_rows();
        auto             M = spla::Matrix::make(n, n, spla::INT);
        for (std::size_t k = 0; k < loader.get_n_values(); ++k) {
            M->set_int(loader.get_Ai()[k], loader.get_Aj()[k], 1);
        }
        return M;
    }

    // Parse the NFA meta file: "nqs QS.. nqf QF.." (1-based states).
    void load_meta(const std::string& path, std::vector<spla::uint>& start_states, std::vector<spla::uint>& final_states) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open " << path << std::endl;
            exit(1);
        }

        std::size_t nqs, nqf;
        in >> nqs;
        start_states.resize(nqs);
        for (std::size_t i = 0; i < nqs; ++i) in >> start_states[i];

        in >> nqf;
        final_states.resize(nqf);
        for (std::size_t i = 0; i < nqf; ++i) in >> final_states[i];

        for (auto& s : start_states) --s;
        for (auto& f : final_states) --f;
    }

    // Parse the sources file: whitespace-separated 1-based source vertex ids.
    std::vector<spla::uint> load_sources(const std::string& path) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open " << path << std::endl;
            exit(1);
        }

        std::vector<spla::uint> sources;
        spla::uint              s;
        while (in >> s) sources.push_back(s - 1);
        return sources;
    }

}// namespace

int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = make_options("rpq", "rpq (single-source regular path querying) algorithm with spla library");
    cxxopts::ParseResult              args;
    int                               ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options";
        return ret;
    }

    const std::string graph_dir    = args[OPT_GRAPH].as<std::string>();
    const std::string query_dir    = args[OPT_QUERY].as<std::string>();
    const std::string meta_path    = args[OPT_QUERY_META].as<std::string>();
    const std::string sources_path = args[OPT_QUERY_SOURCES].as<std::string>();

    if (graph_dir.empty() || query_dir.empty() || meta_path.empty() || sources_path.empty()) {
        std::cerr << "provide --graph, --query, --query-meta and --query-sources" << std::endl;
        return 1;
    }

    spla::Timer timer;
    timer.start();

    const std::vector<std::string> graph_files = list_mtx(graph_dir);
    const std::vector<std::string> nfa_files   = list_mtx(query_dir);

    if (graph_files != nfa_files) {
        std::cerr << "graph and NFA decompositions must have the same labels" << std::endl;
        return 1;
    }

    std::vector<spla::ref_ptr<spla::Matrix>> A;
    std::vector<spla::ref_ptr<spla::Matrix>> R;
    A.reserve(graph_files.size());
    R.reserve(nfa_files.size());

    for (const auto& f : graph_files) A.push_back(load_adjacency(graph_dir + "/" + f));
    for (const auto& f : nfa_files) R.push_back(load_adjacency(query_dir + "/" + f));

    std::vector<spla::uint> start_states, final_states;
    load_meta(meta_path, start_states, final_states);

    const std::vector<spla::uint> sources = load_sources(sources_path);

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);
    std::cout << "env: " << acc_info << std::endl;
    std::cout << "labels: " << A.size() << ", sources: " << sources.size() << std::endl;

    const spla::uint ng = A[0]->get_n_rows();

    const int n_iters = args[OPT_NITERS].as<int>();

    spla::Timer          timer_gpu;
    std::set<spla::uint> reachable;

    for (int i = 0; i < n_iters; ++i) {
        reachable.clear();
        timer_gpu.lap_begin();

        for (spla::uint source : sources) {
            spla::ref_ptr<spla::Vector> v = spla::Vector::make(ng, spla::INT);
            spla::reach_rpq(v, A, R, source, start_states, final_states);

            int x;
            for (spla::uint j = 0; j < ng; ++j) {
                v->get_int(j, x);
                if (x != 0) reachable.insert(j);
            }
        }

        timer_gpu.lap_end();
    }

    std::cout << "reachable vertices (" << reachable.size() << "):";
    for (spla::uint v : reachable) std::cout << " " << v;
    std::cout << std::endl;

    const std::string bench_path = args[OPT_BENCH].as<std::string>();
    if (!bench_path.empty()) {
        const std::string dataset = std::filesystem::path(graph_dir).filename();
        write_bench_entry(bench_path, "RPQ", dataset, timer_gpu);
    }

    spla::Library::get()->finalize();

    timer.stop();

    std::cout << "total(ms): " << timer.get_elapsed_ms() << std::endl;
    std::cout << "gpu(ms): ";
    timer_gpu.print();
    std::cout << std::endl;

    return 0;
}
