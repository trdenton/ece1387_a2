#include <gtest/gtest.h>
#include <vector>
#include "circuit.h"

// Basic file read sanity checks
TEST(FileRead, cct1_blocks_coords) {
    circuit* c = new circuit("../data/cct1");

    ASSERT_EQ(c->get_n_blocks(),26);

    block* b = c->get_block("24");
    vector<int>* coords = b->get_coords();
    ASSERT_EQ((*coords)[0], 25);
    ASSERT_EQ((*coords)[1], 22);

    delete(c);
}

TEST(FileRead, cct1_check_cell_nets) {
    circuit* c = new circuit("../data/cct1");
    block* b = c->get_block("24");
    delete(c);
}

TEST(FileRead, cct1_check_net_pins) {
    circuit* c = new circuit("../data/cct1");
    net* n = c->get_net("12");
    ASSERT_EQ(n->label, "12");
    delete(c);
}
