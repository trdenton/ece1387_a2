#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

// Basic file read sanity checks
TEST(FileRead, cct1_cells_coords) {
    circuit* c = new circuit("../data/cct1");

    ASSERT_EQ(c->get_n_cells(),26);

    cell* b = c->get_cell("24");
    std::pair<int,int> coords = b->get_coords();
    ASSERT_EQ(std::get<0>(coords), 25);
    ASSERT_EQ(std::get<1>(coords), 22);

    delete c;
}

TEST(FileRead, cct1_check_net_pins) {
    circuit* c = new circuit("../data/cct1");
    net* n = c->get_net("12");
    unordered_set<string> expected = {"3","4","8","13","18","23","24","1"};
    ASSERT_EQ(n->label, "12");
    ASSERT_EQ(expected, n->get_cell_labels());
    delete c;
}

TEST(FileRead, cct1_check_cell_nets) {
    circuit* c = new circuit("../data/cct1");
    cell* b = c->get_cell("24");
    unordered_set<string> expected = {"12", "30", "27", "24", "6", "23", "26"};
    ASSERT_EQ(expected, b->get_net_labels());
    delete c;
}

