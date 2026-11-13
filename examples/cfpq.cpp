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
#include <sstream>

namespace {

    constexpr const char* NT_PREFIX = "NON_TERMINAL#";
    constexpr const char* NT_EPS    = "NON_TERMINAL#EPS";

    struct SymbolTable {
        std::vector<std::string>   names;
        std::vector<bool>          nonterm;
        std::map<std::string, int> index;

        int add(const std::string& name, bool nt) {
            auto it = index.find(name);
            if (it != index.end()) {
                if (nt) nonterm[it->second] = true;
                return it->second;
            }
            const int id = static_cast<int>(names.size());
            names.push_back(name);
            nonterm.push_back(nt);
            index.emplace(name, id);
            return id;
        }

        bool is_nonterm(int id) const { return nonterm[id]; }
    };

    struct Rule {
        int first;
        int second;
        int third;
    };

    // Parse a grammar in linear CFG form (one production per line: "A", "A t",
    // "A B C") followed by a "Count:" line naming the start nonterminal, and
    // convert it to weak Chomsky normal form. Ported from CFG_bench grammar_to_WCNF.
    void load_grammar(const std::string&                              path,
                      std::vector<spla::uint>&                        nonterms,
                      std::vector<spla::uint>&                        terms,
                      std::vector<std::array<spla::uint, 3>>&         binary,
                      std::vector<std::pair<spla::uint, spla::uint>>& terminal,
                      std::vector<spla::uint>&                        epsilon) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open grammar file " << path << std::endl;
            exit(1);
        }

        SymbolTable       symbols;
        std::vector<Rule> rules;
        int               start = -1;

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string       a, b, c;
            ss >> a >> b >> c;

            if (a == "Count:") {
                std::string start_name;
                in >> start_name;
                start = symbols.add(start_name, true);
                break;
            }

            const int fa = symbols.add(a, true);
            const int fb = b.empty() ? -1 : symbols.add(b, false);
            const int fc = c.empty() ? -1 : symbols.add(c, false);
            rules.push_back({fa, fb, fc});
        }

        // determine terminals: symbols that never appear as a left-hand side
        for (const auto& r : rules) {
            if (r.second != -1 && !symbols.is_nonterm(r.second)) {
                // keep as terminal; will be enumerated later
            }
            if (r.third != -1 && !symbols.is_nonterm(r.third)) {
                // keep as terminal
            }
        }

        // convert to WCNF
        bool             eps_added = false;
        std::vector<int> promoted_terms;

        for (auto& r : rules) {
            if (r.second == -1 && r.third == -1) {
                continue;// A -> eps
            }
            if (r.second != -1 && r.third == -1) {
                if (!symbols.is_nonterm(r.second)) {
                    continue;// A -> t
                }
                // unit rule A -> N  =>  A -> N EPS, EPS -> eps
                eps_added         = true;
                const int eps_idx = symbols.add(NT_EPS, true);
                r.third           = eps_idx;
                continue;
            }
            // A -> B C
            const bool b_nt = symbols.is_nonterm(r.second);
            const bool c_nt = symbols.is_nonterm(r.third);
            if (b_nt && c_nt) {
                continue;// A -> B C (already binary)
            }
            if (!b_nt && !c_nt) {
                // A -> t1 t2  =>  A -> NT#t1 NT#t2, NT#t1 -> t1, NT#t2 -> t2
                const int nt1 = symbols.add(std::string(NT_PREFIX) + symbols.names[r.second], true);
                const int nt2 = symbols.add(std::string(NT_PREFIX) + symbols.names[r.third], true);
                promoted_terms.push_back(r.second);
                promoted_terms.push_back(r.third);
                r.second = nt1;
                r.third  = nt2;
                continue;
            }
            // A -> N t  or  A -> t N
            int*      term_slot = !b_nt ? &r.second : &r.third;
            const int nt        = symbols.add(std::string(NT_PREFIX) + symbols.names[*term_slot], true);
            promoted_terms.push_back(*term_slot);
            *term_slot = nt;
        }

        if (eps_added) {
            const int eps_idx = symbols.add(NT_EPS, true);
            rules.push_back({eps_idx, -1, -1});
        }

        for (int term : promoted_terms) {
            const int nt = symbols.add(std::string(NT_PREFIX) + symbols.names[term], true);
            rules.push_back({nt, term, -1});
        }

        // enumerate terminals and nonterminals
        std::map<int, int> nonterm_ids;
        std::map<int, int> term_ids;
        for (int i = 0; i < static_cast<int>(symbols.names.size()); ++i) {
            if (symbols.is_nonterm(i)) {
                nonterm_ids[i] = static_cast<int>(nonterm_ids.size());
            } else {
                term_ids[i] = static_cast<int>(term_ids.size());
            }
        }

        for (const auto& r : rules) {
            const int lhs = nonterm_ids[r.first];
            if (r.second == -1 && r.third == -1) {
                epsilon.push_back(static_cast<spla::uint>(lhs));
            } else if (r.third == -1) {
                terminal.push_back({static_cast<spla::uint>(lhs), static_cast<spla::uint>(term_ids[r.second])});
            } else {
                binary.push_back({static_cast<spla::uint>(lhs),
                                  static_cast<spla::uint>(nonterm_ids[r.second]),
                                  static_cast<spla::uint>(nonterm_ids[r.third])});
            }
        }

        nonterms.assign(nonterm_ids.size(), 0);
        terms.assign(term_ids.size(), 0);
        (void) start;
    }

    // Parse a labeled edge list "src dst label" into per-label adjacency matrices.
    void load_graph(const std::string&                        path,
                    std::vector<spla::ref_ptr<spla::Matrix>>& adj_matrices,
                    std::vector<std::string>&                 labels,
                    spla::uint&                               n) {
        std::ifstream in(path);
        if (!in) {
            std::cerr << "failed to open graph file " << path << std::endl;
            exit(1);
        }

        std::map<std::string, int>                                  label_ids;
        std::vector<std::vector<std::pair<spla::uint, spla::uint>>> edges;

        spla::uint  src, dst;
        std::string label;
        while (in >> src >> dst >> label) {
            n       = std::max(n, std::max(src, dst) + 1);
            int lid = label_ids.emplace(label, static_cast<int>(label_ids.size())).first->second;
            if (static_cast<int>(edges.size()) <= lid) edges.resize(lid + 1);
            edges[lid].push_back({src, dst});
        }

        labels.resize(label_ids.size());
        for (const auto& [lbl, id] : label_ids) labels[id] = lbl;

        adj_matrices.resize(edges.size());
        for (std::size_t l = 0; l < edges.size(); ++l) {
            adj_matrices[l] = spla::Matrix::make(n, n, spla::INT);
            for (const auto& [s, d] : edges[l]) {
                adj_matrices[l]->set_int(s, d, 1);
            }
        }
    }

}// namespace

int main(int argc, const char* const* argv) {
    std::shared_ptr<cxxopts::Options> options = make_options("cfpq", "cfpq (single-path context-free path querying) algorithm with spla library");
    cxxopts::ParseResult              args;
    int                               ret;

    if (parse_options(argc, argv, options, args, ret)) {
        std::cerr << "failed to parse options";
        return ret;
    }

    const std::string graph_path   = args[OPT_GRAPH].as<std::string>();
    const std::string grammar_path = args[OPT_GRAMMAR].as<std::string>();

    if (graph_path.empty() || grammar_path.empty()) {
        std::cerr << "provide --graph and --grammar" << std::endl;
        return 1;
    }

    spla::Timer timer;
    timer.start();

    std::vector<spla::ref_ptr<spla::Matrix>> adj_matrices;
    std::vector<std::string>                 labels;
    spla::uint                               n = 0;
    load_graph(graph_path, adj_matrices, labels, n);

    std::vector<spla::uint>                        nonterms;
    std::vector<spla::uint>                        terms;
    std::vector<std::array<spla::uint, 3>>         binary;
    std::vector<std::pair<spla::uint, spla::uint>> terminal;
    std::vector<spla::uint>                        epsilon;
    load_grammar(grammar_path, nonterms, terms, binary, terminal, epsilon);

    std::string acc_info;

    spla::Library* library = spla::Library::get();
    library->set_platform(args[OPT_PLATFORM].as<int>());
    library->set_device(args[OPT_DEVICE].as<int>());
    library->set_queues_count(1);
    library->get_accelerator_info(acc_info);
    std::cout << "env: " << acc_info << std::endl;
    std::cout << "graph: " << n << " vertices, " << labels.size() << " labels" << std::endl;
    std::cout << "grammar: " << nonterms.size() << " nonterminals, " << terms.size() << " terminals, " << binary.size() << " binary rules" << std::endl;

    const int n_iters = args[OPT_NITERS].as<int>();

    std::vector<spla::ref_ptr<spla::Matrix>> T;
    spla::Timer                              timer_gpu;

    for (int i = 0; i < n_iters; ++i) {
        T.clear();
        timer_gpu.lap_begin();
        spla::single_path_cfpq(T, adj_matrices, binary, terminal, epsilon, static_cast<spla::uint>(nonterms.size()));
        timer_gpu.lap_end();
    }

    // count reachable pairs for the start nonterminal (index 0)
    std::size_t reachable = 0;
    if (!T.empty()) {
        for (spla::uint i = 0; i < n; ++i) {
            for (spla::uint j = 0; j < n; ++j) {
                spla::PathIndex v;
                T[0]->get_path_index(i, j, v);
                if (v.middle != 0 || v.height != 0) ++reachable;
            }
        }
    }
    std::cout << "reachable pairs (start nonterminal): " << reachable << std::endl;

    const std::string bench_path = args[OPT_BENCH].as<std::string>();
    if (!bench_path.empty()) {
        const std::string dataset = std::filesystem::path(graph_path).stem();
        write_bench_entry(bench_path, "CFPQ", dataset, timer_gpu);
    }

    spla::Library::get()->finalize();

    timer.stop();

    std::cout << "total(ms): " << timer.get_elapsed_ms() << std::endl;
    std::cout << "gpu(ms): ";
    timer_gpu.print();
    std::cout << std::endl;

    return 0;
}
