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

#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace {

    // Loads a Matrix-Market "coordinate pattern general" file into a square INT matrix.
    spla::ref_ptr<spla::Matrix> load_mtx(const std::string& path) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open " << path << std::endl;
            exit(1);
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '%') continue;
            break;
        }

        std::stringstream ss(line);
        std::size_t       rows, cols, nnz;
        ss >> rows >> cols >> nnz;

        const spla::uint size = static_cast<spla::uint>(std::max(rows, cols));

        spla::ref_ptr<spla::Matrix> M = spla::Matrix::make(size, size, spla::INT);

        for (std::size_t k = 0; k < nnz; ++k) {
            std::size_t src, dst;
            in >> src >> dst;
            // la-rpq matrices are 1-based
            M->set_int(static_cast<spla::uint>(src - 1), static_cast<spla::uint>(dst - 1), 1);
        }

        return M;
    }

    // Parses a meta.txt file: "source target", start states, final states, labels.
    void load_query(const std::string&       path,
                    spla::uint&              source,
                    std::vector<spla::uint>& start_states,
                    std::vector<spla::uint>& final_states,
                    std::vector<int>&        labels) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open " << path << std::endl;
            exit(1);
        }

        std::string line;

        std::getline(in, line);
        std::stringstream ss(line);
        spla::uint        source_n, target_n;
        ss >> source_n >> target_n;
        source = source_n - 1;

        auto read_list = [&](std::vector<spla::uint>& out) {
            std::getline(in, line);
            std::stringstream s(line);
            std::size_t       count;
            s >> count;
            out.resize(count);
            for (std::size_t i = 0; i < count; ++i) s >> out[i];
        };

        read_list(start_states);
        read_list(final_states);

        std::getline(in, line);
        std::stringstream sl(line);
        std::size_t       label_count;
        sl >> label_count;
        labels.resize(label_count);
        for (std::size_t i = 0; i < label_count; ++i) sl >> labels[i];

        // NFA states in la-rpq are 1-based
        for (auto& s : start_states) --s;
        for (auto& f : final_states) --f;
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

    const std::string graph_dir = args[OPT_GRAPH].as<std::string>();
    const std::string query_dir = args[OPT_QUERY].as<std::string>();

    if (graph_dir.empty() || query_dir.empty()) {
        std::cerr << "provide --graph and --query" << std::endl;
        return 1;
    }

    spla::Timer timer;
    timer.start();

    // number of graph vertices = max vertex number in vertices.txt
    spla::uint n = 0;
    {
        std::ifstream in(graph_dir + "/vertices.txt");
        if (!in) {
            std::cerr << "failed to open " << graph_dir << "/vertices.txt" << std::endl;
            return 1;
        }
        std::string name;
        spla::uint  num;
        while (in >> name >> num) n = std::max(n, num);
    }

    spla::uint              source;
    std::vector<spla::uint> start_states, final_states;
    std::vector<int>        labels;
    load_query(query_dir + "/meta.txt", source, start_states, final_states, labels);

    std::vector<spla::ref_ptr<spla::Matrix>> A;
    std::vector<spla::ref_ptr<spla::Matrix>> R;

    for (int label : labels) {
        const int abs_label = std::abs(label);

        spla::ref_ptr<spla::Matrix> graph_mat = load_mtx(graph_dir + "/" + std::to_string(abs_label) + ".txt");

        if (label < 0) {
            // inverse label: use the transposed graph adjacency
            spla::ref_ptr<spla::Matrix> T = spla::Matrix::make(graph_mat->get_n_rows(), graph_mat->get_n_cols(), spla::INT);
            spla::exec_m_transpose(T, graph_mat, spla::IDENTITY_INT);
            graph_mat = T;
        }

        spla::ref_ptr<spla::Matrix> nfa_mat = load_mtx(query_dir + "/" + std::to_string(label) + ".txt");

        A.push_back(graph_mat);
        R.push_back(nfa_mat);
    }

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);
    std::cout << "env: " << acc_info << std::endl;
    std::cout << "graph: " << n << " vertices, " << A.size() << " labels" << std::endl;
    std::cout << "query: source " << source << ", " << start_states.size() << " start states, " << final_states.size() << " final states" << std::endl;

    const int n_iters = args[OPT_NITERS].as<int>();

    spla::ref_ptr<spla::Vector> v = spla::Vector::make(n, spla::INT);
    spla::Timer                 timer_gpu;

    for (int i = 0; i < n_iters; ++i) {
        v->clear();
        timer_gpu.lap_begin();
        spla::reach_rpq(v, A, R, source, start_states, final_states);
        timer_gpu.lap_end();
    }

    std::size_t reachable = 0;
    for (spla::uint i = 0; i < n; ++i) {
        int x;
        v->get_int(i, x);
        if (x != 0) ++reachable;
    }
    std::cout << "reachable vertices: " << reachable << std::endl;

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
