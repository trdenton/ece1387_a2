#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

TEST(Cell, cct1_fixed_nets) {
    circuit* c = new circuit("../data/cct1");
    cell* c8 = c->get_cell("8");
    cell* c1 = c->get_cell("1");

    ASSERT_TRUE(c8->is_fixed());
    ASSERT_FALSE(c1->is_fixed());

    delete(c);
}
