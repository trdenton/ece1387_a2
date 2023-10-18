#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <string>
#include <iostream>
#include <queue>
#include <sstream>
#include <inttypes.h>
#include <limits.h>
#include "spdlog/spdlog.h"

using namespace std;

#define UNUSED (INT_MAX)
#define TARGET (INT_MAX-1)
#define USED (INT_MAX-2)
#define ON_PATH(__X__) (__X__ < USED)
#define SOURCE (0)

class circuit {
  public:

    circuit(string s);

    ~circuit() {
    }

    bool fit(bool interactive);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
