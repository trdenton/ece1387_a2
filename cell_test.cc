#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

TEST(Cell, cct1_fixed_nets) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    cell* c8 = c->get_cell("8");
    cell* c1 = c->get_cell("1");

    ASSERT_TRUE(c8->is_fixed());
    ASSERT_FALSE(c1->is_fixed());

    delete(c);
}

TEST(Cell, cct1_connects_to_fixed_cell) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    cell* c4 = c->get_cell("4");
    ASSERT_TRUE(c->connects_to_fixed_cell(c4));
    delete(c);
}

TEST(Cell, cct1_no_connect_to_fixed_cell) {
    circuit* c = new circuit("../data/cct_no_connect_to_fixed");
    c->iter();
    cell* c3 = c->get_cell("3");
    ASSERT_FALSE(c->connects_to_fixed_cell(c3));
    delete(c);
}
