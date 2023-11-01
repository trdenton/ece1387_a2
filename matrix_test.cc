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

TEST(Matrix, cct_mat_test) {
    circuit* c = new circuit("../data/cct_inspect_csc");
    adjacency_matrix* Q = c->get_adjacency_matrix();
    ASSERT_EQ(Q->Ap[0], 0);
    ASSERT_EQ(Q->Ap[1], 2);
    ASSERT_EQ(Q->Ap[2], 5);
    
    ASSERT_EQ(Q->Ai[0], 0);
    ASSERT_EQ(Q->Ai[1], 1);
    ASSERT_EQ(Q->Ai[2], 0);
    ASSERT_EQ(Q->Ai[3], 1);
    ASSERT_EQ(Q->Ai[4], 2);
    ASSERT_EQ(Q->Ai[5], 1);
    ASSERT_EQ(Q->Ai[6], 2);

    ASSERT_EQ(Q->Ax[0], 1);
    ASSERT_EQ(Q->Ax[1], -1);
    ASSERT_EQ(Q->Ax[2], -1);
    ASSERT_EQ(Q->Ax[3], 2);
    ASSERT_EQ(Q->Ax[4], -1);
    ASSERT_EQ(Q->Ax[5], -1);
    ASSERT_EQ(Q->Ax[6], 1);
    delete(c);
}

TEST(Matrix, cct_rhs_test) {
    circuit* circ = new circuit("../data/cct_inspect_csc");
    adjacency_matrix* Q = circ->get_adjacency_matrix();
    double* C = Q->get_C_ss();
    //ASSERT_EQ(C[0], 0);
    //ASSERT_EQ(C[1], 0);
    delete circ;
}
