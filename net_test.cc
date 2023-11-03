#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

TEST(Net, cct1_check_nets_in_common) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    cell* c1 = c->get_cell("1");
    cell* c12 = c->get_cell("12");

    ASSERT_TRUE(c1->is_connected_to(c12));

    delete(c);
}

TEST(Net, cct1_check_nets_not_in_common) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    cell* c1 = c->get_cell("1");
    cell* c12 = c->get_cell("12");
    cell* c26 = c->get_cell("26");

    ASSERT_FALSE(c1->is_connected_to(c26));

    ASSERT_FALSE(c12->is_connected_to(c26)); // has net 21 in common

    delete(c);
}

TEST(Net, cct1_check_net_n_pins) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    net* n3 = c->get_net("3"); // should have 4 pins
    net* n4 = c->get_net("4"); // should have 4 pins

    ASSERT_EQ(n3->num_pins(), 4);
    ASSERT_EQ(n4->num_pins(), 2);
    delete(c);
}

TEST(Net, cct1_check_net_weights) {
    circuit* c = new circuit("../data/cct1");
    c->iter();
    net* n3 = c->get_net("3"); // should have 4 pins
    net* n4 = c->get_net("4"); // should have 4 pins

    // initially its the clique weight, 2/p
    ASSERT_EQ(n3->get_weight(), 2./4.);
    ASSERT_EQ(n4->get_weight(), 2./2.);
    delete(c);
}
