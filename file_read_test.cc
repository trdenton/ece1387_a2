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

TEST(FileRead, cct1_nets) {
  circuit* c = new circuit("../data/cct1");
  block* b = c->get_block("24");
  vector<string> nets = b->get_nets();
  ASSERT_EQ(nets[0],"12");
  ASSERT_EQ(nets[1],"30");
  ASSERT_EQ(nets[2],"27");
  ASSERT_EQ(nets[3],"24");
  ASSERT_EQ(nets[4],"6");
  ASSERT_EQ(nets[5],"23");
  ASSERT_EQ(nets[6],"26");
  delete(c);
}
