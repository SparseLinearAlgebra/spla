#include "common.hpp"
#include "options.hpp"

#include <spla.hpp>
#define INF std::numeric_limits<float>::infinity()
int main(int argc, const char* const* argv) {
        int32_t n = 7;
        int32_t len_S = n;
        auto S = spla::Matrix::make(n, n, spla::PAIR);
        auto T = spla::Matrix::make(n, n, spla::FLOAT);
        S->set_pair(0, 1, spla::T_PAIR(7.0f, 1));
        S->set_pair(0, 4, spla::T_PAIR(4.0f, 4));
        S->set_pair(1, 0, spla::T_PAIR(7.0f, 0));
        S->set_pair(1, 2, spla::T_PAIR(11.0f, 2));
        S->set_pair(1, 3, spla::T_PAIR(10.0f, 3));
        S->set_pair(1, 4, spla::T_PAIR(9.0f, 4));
        S->set_pair(2, 1, spla::T_PAIR(11.0f, 1));
        S->set_pair(2, 3, spla::T_PAIR(5.0f, 3));
        S->set_pair(3, 1, spla::T_PAIR(10.0f, 1));
        S->set_pair(3, 2, spla::T_PAIR(5.0f, 2));
        S->set_pair(3, 4, spla::T_PAIR(15.0f, 4));
        S->set_pair(3, 5, spla::T_PAIR(12.0f, 5));
        S->set_pair(3, 6, spla::T_PAIR(8.0f, 6));
        S->set_pair(4, 0, spla::T_PAIR(4.0f, 0));
        S->set_pair(4, 1, spla::T_PAIR(9.0f, 1));
        S->set_pair(4, 3, spla::T_PAIR(15.0f, 3));
        S->set_pair(4, 5, spla::T_PAIR(6.0f, 5));
        S->set_pair(5, 3, spla::T_PAIR(12.0f, 3));
        S->set_pair(5, 4, spla::T_PAIR(6.0f, 4));
        S->set_pair(5, 6, spla::T_PAIR(13.0f, 6));
        S->set_pair(6, 3, spla::T_PAIR(8.0f, 3));
        S->set_pair(6, 5, spla::T_PAIR(13.0f, 5));

        auto parent = spla::Vector::make(n, spla::PAIR);
        for (int32_t i = 0; i < n; i++) {
            parent->set_pair(i, spla::T_PAIR(0.0f, i));
        }

        auto edge = spla::Vector::make(n, spla::PAIR);
        auto cedge = spla::Vector::make(n, spla::PAIR);
        auto t = spla::Vector::make(n, spla::PAIR);

        auto mask = spla::Vector::make(n, spla::PAIR); 
        for (int32_t i = 0; i < n; i++) {
            mask->set_pair(i, spla::T_PAIR(0.0f, 0));
        }
        auto init_inf = spla::Scalar::make(spla::PAIR); //нулевой элемент по сложению в полукольце
        spla::T_PAIR init_val;
        init_inf->set_pair(init_val);

        while (len_S > 0) {
                // step 1, min edges for each vertices
                spla::exec_mxv_masked(edge, mask, S, parent, spla::MUL_PAIR, spla::MIN_PAIR, spla::ALWAYS_PAIR, init_inf);
                //////////////////
                std::cout << "edge = [";
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR p;
                        edge->get_pair(i, p);
                        std::cout << "(" << p.weight << ", " << p.vertex << "), ";
                }
                std::cout << "]\n";
                //////////////////
                // step 2, min edges for each component
                for (int32_t i = 0; i < n; i++) {
                        
                        cedge->set_pair(i, init_val);
                }
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR p;
                        spla::T_PAIR p1;
                        spla::T_PAIR p2;
                        parent->get_pair(i, p); 
                        auto p_i = p.vertex; // p_i = parent[i]
                        cedge->get_pair(p_i, p1); // p1 = cedge[parent[i]]
                        edge->get_pair(i, p2); // p2 = edge[i]
                        auto min_for_comp = p1.weight <= p2.weight? p1 : p2; // min(cedge[parent[i]], edge[i])
                        cedge->set_pair(p_i, min_for_comp);
                }
                ///////////////////
                std::cout << "cedge = [";
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR p;
                        cedge->get_pair(i, p);
                        std::cout << "(" << p.weight << ", " << p.vertex << "), ";
                }
                std::cout << "]\n";
                ///////////////////
                // step 3, когда нашли лучшее ребро компоненты распространяем его на все вершины
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR parent_v;
                        spla::T_PAIR cedge_v;
                        parent->get_pair(i, parent_v);
                        cedge->get_pair(parent_v.vertex, cedge_v);
                        t->set_pair(i, cedge_v); //t[i] = cedge[parent[i]]
                }
                //step 4 выбор представителя для каждой компоненты(когда лучшее ребро в edges совпадает с лучшим ребром компоненты t)
                auto index = spla::Vector::make(n, spla::INT);
                
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR edge_v, t_v;
                        edge->get_pair(i, edge_v);
                        t->get_pair(i, t_v);
                        if (edge_v == t_v) index->set_int(i, i);
                        else index->set_int(i, n);
                }
                auto temp = spla::Vector::make(n, spla::INT);
                for (int32_t i = 0; i < n; i++) temp->set_int(i, n);
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR parent_v;
                        parent->get_pair(i, parent_v);
                        auto p_i = parent_v.vertex;
                        spla::T_INT temp_v, ind_v;
                        temp->get_int(p_i, temp_v);
                        index->get_int(i, ind_v);
                        spla::T_INT min_v = temp_v < ind_v? temp_v : ind_v;
                        temp->set_int(p_i, min_v); //temp[parent[i]] = min(temp[parent[i]], index[i])
                }
                /////////////////
                std::cout << "t = [";
                for (int32_t i = 0; i < n; i++) {
                        spla::T_INT p;
                        temp->get_int(i, p);
                        std::cout << p << ", ";
                }
                std::cout << "]\n";
                ///////////////////
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR parent_v;
                        parent->get_pair(i, parent_v);
                        auto p_i = parent_v.vertex;
                        spla::T_INT temp_v;
                        temp->get_int(p_i, temp_v);
                        index->set_int(i, temp_v);
                }
                /////////////////
                std::cout << "index = [";
                for (int32_t i = 0; i < n; i++) {
                        spla::T_INT p;
                        index->get_int(i, p);
                        std::cout << p << ", ";
                }
                std::cout << "]\n";
                ///////////////////
                //step 5 добавляем найденные ребра в MST
                // это те у которых index[i] = i
                auto new_parent = spla::Vector::make(n, spla::PAIR);
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR p;
                        parent->get_pair(i, p);
                        new_parent->set_pair(i, p);
                }
                for (int32_t i = 0; i < n; i++) {
                        spla::T_INT ind_v;
                        index->get_int(i, ind_v);
                        if (i == ind_v) {
                                auto row = spla::Vector::make(n, spla::PAIR);
                                spla::exec_m_extract_row(row, S, i, spla::IDENTITY_PAIR);
                                int min_vertex = -1;
                                float min_weight = INF;

                                for (int32_t j = 0; j < n; j++) {
                                        spla::T_PAIR pair_row;
                                        row->get_pair(j, pair_row);
                                        auto pair_row_weight = pair_row.weight;
                                        auto pair_row_vertex = pair_row.vertex;
                                        if (pair_row_weight < INF) {
                                                spla::T_PAIR p1, p2;
                                                parent->get_pair(i, p1);
                                                parent->get_pair(pair_row_vertex, p2);
                                                if (p1.vertex != p2.vertex) { //разные компоненты
                                                        if (pair_row_weight < min_weight) {
                                                                min_weight = pair_row_weight;
                                                                min_vertex = j;
                                                        }

                                                }
                                        }
                

                                }
                                if (min_vertex == -1) continue;
                                T->set_float(i, min_vertex, min_weight);
                                T->set_float(min_vertex, i, min_weight);
                                std::cout << "T <- "  << i << " - " << min_vertex << " (" << min_weight << ")\n";
                                if (i < min_vertex) {
                                        spla::T_PAIR p;
                                        spla::T_PAIR old_p;
                                        new_parent->get_pair(i, p);
                                        new_parent->get_pair(min_vertex, old_p);
                                        new_parent->set_pair(min_vertex, spla::T_PAIR(0.0f, p.vertex));
                                        for (int k = 0; k < n; k++) {
                                                spla::T_PAIR p1;
                                                new_parent->get_pair(k, p1);
                                                if (p1.vertex == old_p.vertex) new_parent->set_pair(k, spla::T_PAIR(0.0f, p.vertex));
                                        }
                                }
                                else {
                                        spla::T_PAIR p;
                                        spla::T_PAIR old_p;
                                        new_parent->get_pair(min_vertex, p);
                                        new_parent->get_pair(i, old_p);
                                        new_parent->set_pair(i, spla::T_PAIR(0.0f, p.vertex));
                                        for (int k = 0; k < n; k++) {
                                                spla::T_PAIR p1;
                                                new_parent->get_pair(k, p1);
                                                if (p1.vertex == old_p.vertex) new_parent->set_pair(k, spla::T_PAIR(0.0f, p.vertex));
                                        }
                                }
                                
                        }
                }
                parent = new_parent;
                ///////////////////
                std::cout << "parent = [";
                for (int32_t i = 0; i < n; i++) {
                        spla::T_PAIR p;
                        parent->get_pair(i, p);
                        std::cout << p.vertex << ", ";
                }
                std::cout << "]\n";
                ///////////////////
                //обновляем матрицу смежности
                len_S = 0;
                auto filtered_S = spla::Matrix::make(n, n, spla::PAIR);
                for (int32_t i = 0; i < n; i++) {
                        for (int32_t j = 0; j < n; j++) {
                                spla::T_PAIR val;
                                S->get_pair(i, j, val);
                                if (val.weight != std::numeric_limits<float>::infinity()) {
                                        spla::T_PAIR parent_i, parent_j;
                                        parent->get_pair(i, parent_i);
                                        parent->get_pair(j, parent_j);
                                        if ((parent_i.vertex != parent_j.vertex) && (val.weight != std::numeric_limits<float>::infinity())) {
                                                filtered_S->set_pair(i, j, val);
                                                len_S++;
                                        }

                                }
                        }
                }
                S = filtered_S;

        }
        std::cout << "=========================MST========================\n";
        // Вывод результирующего дерева MST
        long sum = 0;

        for (int i = 0; i < n; i++) {
                std::cout << i << ": ";
                for (int j = 0; j < n; j++) {
                        spla::T_FLOAT f;
                        T->get_float(i, j, f);
                        std::cout << f << "\t";
                        sum += f;
                }
                std::cout <<"\n";
        }
        std::cout << "Сумма ребер: " << sum / 2 << "\n";

}
