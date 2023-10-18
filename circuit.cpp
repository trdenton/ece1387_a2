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

    // first line -  grid size 
    // second line - tracks per channel
    getline(infile, line);
    grid_size = stoi(line);

    getline(infile, line);
    tracks_per_channel = stoi(line);
    if (force_w > 0)
        tracks_per_channel = force_w;
        

    spdlog::debug("grid size {} tracks per channel {}", grid_size, tracks_per_channel);

    while (true) {
      getline(infile, line);
      std::stringstream ss(line);
      istream_iterator<std::string> begin(ss);
      istream_iterator<std::string> end;
      vector<string> vstrings(begin, end);
      if (vstrings[0] == "-1")
        break;
      spdlog::debug("size of vstrings: {}", vstrings.size());
      connection* c = new connection(vstrings);
      spdlog::debug("size of vstrings: {}", vstrings.size());
      if (c->d0 == 1 or c->d1 == 1) {
        spdlog::debug("detected dense circuit");
        layers = 2;
      }
      add_connection(c);
    }
    string conns = dump_connections();
    spdlog::debug("connection dump:\n{}", conns);

    infile.close();
  } else {
    spdlog::error("Could not open {}", file);
  }
}

bool circuit::fit(bool interactive) {
    return true;
}


