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
    vector<string> nets;
    int x;
    int y;

  public:
    string label;
    block(vector<string> s);
    void connect(block* other);
    void set_coords(int _x, int _y);
    vector<int>* get_coords();
    vector<string> get_nets();
  
};

class net {
  public:
    string label;
    net(string l);
};

class circuit {
  private:
    vector<block*> blocks;
    vector<net*> nets;

  public:
    circuit(string s);
    ~circuit() {
    }
    int get_n_blocks() { return blocks.size();}

    bool fit(bool interactive);
    block* get_block(string label);
    void add_block_connections(vector<string> toks);
    void add_block_coords(vector<string> toks);
    net* get_net(string label);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
