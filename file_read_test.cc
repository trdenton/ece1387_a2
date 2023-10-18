#include <gtest/gtest.h>
#include <vector>
#include "circuit.h"

// Basic file read sanity checks
TEST(FileRead, cct1) {
  circuit* c = new circuit("../data/cct1");

  ASSERT_TRUE(true);

  delete(c);
}
