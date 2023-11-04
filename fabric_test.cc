#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"
#include "fabric.h"

TEST(Fabric, base) {
    fabric* fab = new fabric(10,10);

    delete fab;
}
