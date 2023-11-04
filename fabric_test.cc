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
