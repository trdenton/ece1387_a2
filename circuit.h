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

//this is used to instantiate test connections for ui test purposes
#include "ui_tests.h"

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

    string dump_connections() {
      ostringstream outstring;
      for (auto* conn : conns){
        outstring << conn->to_string() << endl;
      }

      return outstring.str();
    }

    bool fit(bool interactive);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
