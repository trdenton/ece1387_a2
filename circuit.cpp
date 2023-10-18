#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
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
          }
          break;
      }
    }
    infile.close();
  } else {
    spdlog::error("Could not open {}", file);
  }
}

bool circuit::fit(bool interactive) {
  return true;
}

void circuit::add_block_connections(vector<string> toks) {
}


block::block(string s) {
  label = stoi(s);
}
