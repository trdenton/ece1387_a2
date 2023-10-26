#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iterator>
#include "spdlog/spdlog.h"
#include <thread>
#include <unistd.h>
#include <condition_variable>

using namespace std; 

condition_variable cv;
mutex cv_m;
int update_sig = 0;

void circuit_wait_for_ui() {
  unique_lock<std::mutex> lk(cv_m);
  cv.wait(lk,[]{return update_sig==1;});
  update_sig=0;
}

void circuit_next_step() {
  {
    lock_guard<mutex> lk(cv_m);
    update_sig = 1;
  }
  cv.notify_one();
}

enum input_read_state {
  SECTION_1,
  SECTION_2
};

circuit::circuit(string file) {
  string line;
  ifstream infile (file);
  spdlog::debug("Reading input file {}", file);

  if (infile.is_open()) {

    enum input_read_state read_state = SECTION_1;

    bool done = false;
    while (!done) {
      getline(infile, line);
      std::stringstream ss(line);
      istream_iterator<std::string> begin(ss);
      istream_iterator<std::string> end;
      vector<string> vstrings(begin, end);
      switch(read_state) {
        case SECTION_1:
          if (vstrings[0] == "-1") {
            read_state = SECTION_2;
            break;
          }
          add_block_connections(vstrings);
           
          break;
        case SECTION_2:
          if (vstrings[0] == "-1") {
            done = true;
            break;
          }
          add_block_coords(vstrings);
          break;
      }
    }
    infile.close();
  } else {
    spdlog::error("Could not open {}", file);
  }
}

net* circuit::get_net(string label) {
    return new net(label);
}

bool circuit::fit(bool interactive) {
  return true;
}

void circuit::add_block_connections(vector<string> toks) {
  block* b = new block(toks);
  blocks.push_back(b);
}

void circuit::add_block_coords(vector<string> s) {
  block* b = get_block(s[0]);
  int x = stoi(s[1]);
  int y = stoi(s[2]);
  b->set_coords(x,y);
}

block* circuit::get_block(string label) {
  for (auto b : blocks) {
    if (b->label == label) {
      return b;
    }
  }
  return nullptr;
}


block::block(vector<string> s) {
  x = 0;
  y = 0;
  label = s[0];
  nets = std::vector<string>(s.begin()+1,s.end()-1);
}

void block::set_coords(int _x, int _y) {
  x = _x;
  y = _y;
}

vector<int>* block::get_coords() {
  vector<int>* vec = new vector<int>();
  vec->push_back(x);
  vec->push_back(y);
  return vec;
}

vector<string> block::get_nets() {
  return nets;
}

net::net(string l) {
  label = l;
}

