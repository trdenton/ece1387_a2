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

class block {
  private:
    int label;
    vector<block*> connections;

  public:
    block(string s);
    void connect(block* other);
  
};

class circuit {
  private:
    vector<block*> blocks;

  public:
    circuit(string s);
    ~circuit() {
    }

    bool fit(bool interactive);
    void add_block_connections(vector<string> toks);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
