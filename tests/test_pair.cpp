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
        EXPECT_EQ(type->get_cpp(), "pair");
        EXPECT_EQ(type->get_description(), "weight-vertex pair float-int");
        EXPECT_EQ(type->get_size(), sizeof(spla::Pair));
        EXPECT_EQ(type->get_id(), 5);
}

TEST(pair, op_registration) {
        spla::Library::get();
        EXPECT_EQ(spla::MIN_PAIR->get_name(), "MIN_PAIR");
}

TEST(pair, operation_make_pair_from_int_float) {
        spla::Library::get();
        spla::T_PAIR res;
        EXPECT_EQ(res.vertex, -1);
        EXPECT_EQ(res.weight, std::numeric_limits<float>::infinity());
        auto op = spla::MAKE_PAIR_FROM_INT_FLOAT;
        ASSERT_TRUE(op);

}

TEST(pair, mxv_pair) {
        int32_t n = 7;
        auto S = spla::Matrix::make(n, n, spla::FLOAT);
        S->set_float(0, 1, 7.0f);
        S->set_float(0, 4, 4.0f);
        S->set_float(1, 0, 7.0f);
        S->set_float(1, 2, 11.0f);
        S->set_float(1, 3, 10.0f);
        S->set_float(1, 4, 9.0f);
        S->set_float(2, 1, 11.0f);
        S->set_float(2, 3, 5.0f);
        S->set_float(3, 1, 10.0f);
        S->set_float(3, 2, 5.0f);
        S->set_float(3, 4, 15.0f);
        S->set_float(3, 5, 12.0f);
        S->set_float(3, 6, 8.0f);
        S->set_float(4, 0, 4.0f);
        S->set_float(4, 1, 9.0f);
        S->set_float(4, 3, 15.0f);
        S->set_float(4, 5, 6.0f);
        S->set_float(5, 3, 12.0f);
        S->set_float(5, 4, 6.0f);
        S->set_float(5, 6, 13.0f);
        S->set_float(6, 3, 8.0f);
        S->set_float(6, 5, 13.0f);

        auto parent = spla::Vector::make(n, spla::INT);
        //parent = [0, 1, 2, 3, 4, 5, 6, 7] к какой компоненты принадлежит вершина
        for (int32_t i = 0; i < n; i++) {
            parent->set_int(i, i);
        }
        auto edge = spla::Vector::make(n, spla::PAIR);

       
        auto mask = spla::Vector::make(n, spla::INT); //mask = [1, 1, 1, 1, 1, 1, 1]
        for (int32_t i = 0; i < n; i++) {
            mask->set_int(i, 1);
        }
        auto init_inf = spla::Scalar::make(spla::PAIR);

        spla::exec_mxv_masked(edge, mask, S, parent, spla::MAKE_PAIR_FROM_INT_FLOAT, spla::MIN_PAIR, spla::ALWAYS_PAIR, init_inf);

        std::cout << "edge after mxv:" << std::endl;
        for (int32_t i = 0; i < n; i++) {
            spla::T_PAIR p;
            edge->get_pair(i, p);
            std::cout << "edge[" << i << "] = (" << p.weight << ", " << p.vertex << ")" << std::endl;
        }

        // Должны получить
        // edge[0] = (4, 4)
        // edge[1] = (7, 0)
        // edge[2] = (5, 3)
        // edge[3] = (5, 2)
        // edge[4] = (4, 0)
        // edge[5] = (6, 4)
        // edge[6] = (8, 3)


        

}

SPLA_GTEST_MAIN
