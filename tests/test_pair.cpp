#include "test_common.hpp"

#include "spla.hpp"

TEST(pair, struct_creation) {
    spla::Pair p(2.5f, 2);  
    EXPECT_EQ(p.weight, 2.5f);
    EXPECT_EQ(p.vertex, 2);
}
TEST(pair, basic_operations) {
    spla::Pair p1(2.5f, 2);
    spla::Pair p2(1.5f, 5);
    
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

SPLA_GTEST_MAIN
