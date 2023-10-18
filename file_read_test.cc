#include <gtest/gtest.h>
#include <vector>
#include "circuit.h"

// Basic file read sanity checks
TEST(FileRead, cct1) {
  circuit* c = new circuit("../data/cct1");

  ASSERT_EQ(c->get_n_blocks(),26);

  delete(c);
}
