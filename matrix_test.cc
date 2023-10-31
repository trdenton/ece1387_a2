#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"

TEST(Matrix, cct1_sum_all_weights) {
    circuit* c = new circuit("../data/cct1");
    cell* c26 = c->get_cell("26");

    // 26 4 23 -1
    // net 4 and net 23
    // net 4 only connects 2 cells
    // so hte weight should be 1.0

    // net 23 also has 2 pins, also 1.0
    // the total weight should be 2.0
    double weight = c->sum_all_connected_weights(c26);
    ASSERT_EQ( weight, 2. );

    delete(c);
}
