#include "test_common.hpp"
#include <limits>

#include "spla.hpp"

TEST(pair, struct_creation) {
    spla::T_PAIR p(2.5f, 2);  
    EXPECT_EQ(p.weight, 2.5f);
    EXPECT_EQ(p.vertex, 2);
}
TEST(pair, basic_operations) {
    spla::T_PAIR p1(2.5f, 2);
    spla::T_PAIR p2(1.5f, 5);
    
    EXPECT_EQ(p1.weight, 2.5f);
    EXPECT_EQ(p1.vertex, 2);
    EXPECT_TRUE(p2.weight < p1.weight);
}

TEST(pair, type_registration) {
        auto type = spla::PAIR;
        ASSERT_TRUE(type);
        EXPECT_EQ(type->get_name(), "PAIR");
        EXPECT_EQ(type->get_code(), "P");
        EXPECT_EQ(type->get_cpp(), "struct Pair");
        EXPECT_EQ(type->get_description(), "weight-vertex pair float-int");
        EXPECT_EQ(type->get_size(), sizeof(spla::Pair));
        EXPECT_EQ(type->get_id(), 5);
}

TEST(pair, op_registration) {
        spla::Library::get();
        EXPECT_EQ(spla::MIN_PAIR->get_name(), "MIN_PAIR");
}
TEST(pair, get_pair_matrix) {
        auto S = spla::Matrix::make(2, 2, spla::PAIR);
        S->set_pair(0, 1, spla::T_PAIR(7.0f, 1));
        spla::T_PAIR pair;
        S->get_pair(0, 1, pair);
        EXPECT_EQ(pair.vertex, 1);
        EXPECT_EQ(pair.weight, 7.0f);

}
TEST(pair, mxv_pair) {
        int32_t n = 7;
        auto S = spla::Matrix::make(n, n, spla::PAIR);
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
        //parent = [(0, 0), (0, 1), (0, 2), (0, 3),(0, 4),(0, 5), (0, 6)] к какой компоненты принадлежит вершина
        for (int32_t i = 0; i < n; i++) {
            parent->set_pair(i, spla::T_PAIR(0.0f, i));
        }
        auto edge = spla::Vector::make(n, spla::PAIR);

       
        auto mask = spla::Vector::make(n, spla::PAIR); //любые значения тк select выберет все пары
        for (int32_t i = 0; i < n; i++) {
            mask->set_pair(i, spla::T_PAIR(0.0f, 0));
        }
        auto init_inf = spla::Scalar::make(spla::PAIR);//нулевое значение по сложению (+inf, -1)
        spla::T_PAIR init_val(1e9f, -1);  // большой вес и -1 (нет вершины)
        init_inf->set_pair(init_val);        
        spla::exec_mxv_masked(edge, mask, S, parent, spla::MUL_PAIR, spla::MIN_PAIR, spla::ALWAYS_PAIR, init_inf);

        for (int32_t i = 0; i < n; i++) {
            spla::T_PAIR p;
            edge->get_pair(i, p);
            std::cout << "edge[" << i << "] = (" << p.weight << ", " << p.vertex << ")" << std::endl;
        }
        spla::T_PAIR expected[] = {
            spla::T_PAIR(4.0f, 4),
            spla::T_PAIR(7.0f, 0),
            spla::T_PAIR(5.0f, 3),
            spla::T_PAIR(5.0f, 2),
            spla::T_PAIR(4.0f, 0),
            spla::T_PAIR(6.0f, 4),
            spla::T_PAIR(8.0f, 3)
        };
        
        for (int32_t i = 0; i < n; i++) {
            spla::T_PAIR p;
            edge->get_pair(i, p);
            EXPECT_FLOAT_EQ(p.weight, expected[i].weight);
            EXPECT_EQ(p.vertex, expected[i].vertex);
        }
}
TEST(pair, extract_row) {
    int32_t n = 3;
    auto S = spla::Matrix::make(n, n, spla::PAIR);
    S->set_pair(0, 1, spla::T_PAIR(7.0f, 1));
    S->set_pair(1, 0, spla::T_PAIR(4.0f, 4));
    S->set_pair(1, 2, spla::T_PAIR(7.0f, 0));
       
    auto row1 = spla::Vector::make(n, spla::PAIR);
    spla::exec_m_extract_row(row1, S, 1, spla::IDENTITY_PAIR);
    for (int v = 0; v < n; v++) {
        spla::T_PAIR p;
        row1->get_pair(v, p);
        if (p.weight < 1e8) {
            std::cout << "  S[1][" << v << "] = (" << p.weight << "," << p.vertex << ")\n";
        }
    }
}

SPLA_GTEST_MAIN
