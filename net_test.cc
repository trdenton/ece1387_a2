#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

TEST(Net, cct1_check_nets_in_common) {
    circuit* c = new circuit("../data/cct1");
    cell* c1 = c->get_cell("1");
    cell* c12 = c->get_cell("12");
    cell* c14 = c->get_cell("14");

    ASSERT_TRUE(c1->is_connected_to(c12));
    ASSERT_FALSE(c1->is_connected_to(c14));

    ASSERT_TRUE(c12->is_connected_to(c14)); // has net 21 in common

    delete(c);
}
