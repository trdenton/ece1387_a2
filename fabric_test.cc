#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"
#include "fabric.h"

TEST(Fabric, base) {
    fabric* fab = new fabric(10,10);

    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            bin* b = fab->get_bin(i,j);
            ASSERT_EQ(b->x, (double)i);
            ASSERT_EQ(b->y, (double)j);
            ASSERT_TRUE(b->usable);
        }
    }

    delete fab;
}

TEST(Fabric, obstruction) {
    fabric* fab = new fabric(10,10);
    fab->mark_obstruction(1,1,4,4);

    ASSERT_TRUE( fab->get_bin(0,0)->usable);
    ASSERT_TRUE( fab->get_bin(0,1)->usable);
    ASSERT_TRUE( fab->get_bin(0,5)->usable);
    ASSERT_TRUE( fab->get_bin(1,5)->usable);
    ASSERT_TRUE( fab->get_bin(5,5)->usable);
    ASSERT_TRUE( fab->get_bin(5,0)->usable);
    ASSERT_TRUE( fab->get_bin(5,1)->usable);

    ASSERT_FALSE( fab->get_bin(1,1)->usable);
    ASSERT_FALSE( fab->get_bin(1,2)->usable);
    ASSERT_FALSE( fab->get_bin(1,3)->usable);
    ASSERT_FALSE( fab->get_bin(1,4)->usable);

    ASSERT_FALSE( fab->get_bin(2,1)->usable);
    ASSERT_FALSE( fab->get_bin(2,2)->usable);
    ASSERT_FALSE( fab->get_bin(2,3)->usable);
    ASSERT_FALSE( fab->get_bin(2,4)->usable);

    ASSERT_FALSE( fab->get_bin(3,1)->usable);
    ASSERT_FALSE( fab->get_bin(3,2)->usable);
    ASSERT_FALSE( fab->get_bin(3,3)->usable);
    ASSERT_FALSE( fab->get_bin(3,4)->usable);

    ASSERT_FALSE( fab->get_bin(4,1)->usable);
    ASSERT_FALSE( fab->get_bin(4,2)->usable);
    ASSERT_FALSE( fab->get_bin(4,3)->usable);
    ASSERT_FALSE( fab->get_bin(4,4)->usable);

    delete fab;
}
