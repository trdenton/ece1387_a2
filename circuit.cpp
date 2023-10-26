#include "circuit.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <iterator>
#include "spdlog/spdlog.h"
#include <thread>
#include <unistd.h>
#include <condition_variable>
#include <cassert>

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

/****
*
* circuit class functions
*
****/

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
                    add_cell_connections(vstrings);

                    break;
                case SECTION_2:
                    if (vstrings[0] == "-1") {
                        done = true;
                        break;
                    }
                    add_cell_coords(vstrings);
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

void circuit::add_net(string s) {
    net* n = new net(s);
    if (nets.find(*n) == nets.end()) {
        nets.insert(*n);
    } else {
        delete(n);
    }
}

void circuit::add_cell_connections(vector<string> toks) {
    cell* b = new cell(toks);
    cells.push_back(b);

    vector<string> nets = std::vector<string>(toks.begin()+1,toks.end()-1);
    for(string net : nets) {
        add_net(net);
    }

}

void circuit::add_cell_coords(vector<string> s) {
    cell* b = get_cell(s[0]);
    int x = stoi(s[1]);
    int y = stoi(s[2]);
    b->set_coords(x,y);
}

cell* circuit::get_cell(string label) {
    for (auto b : cells) {
        if (b->label == label) {
            return b;
        }
    }
    return nullptr;
}

/****
*
* cell class functions
*
****/


cell::cell(vector<string> s) {
    x = 0;
    y = 0;
    label = s[0];
    //nets = std::vector<string>(s.begin()+1,s.end()-1);
}

void cell::set_coords(int _x, int _y) {
    x = _x;
    y = _y;
}

unordered_set<string> cell::get_net_labels() {
    return net_labels;
}

pair<int,int> cell::get_coords() {
    return pair<int,int>(x,y);
}

void cell::add_net(string s) {
    assert(s.length() > 0);
    net_labels.insert(s); 
}
void cell::add_net(net& n) {
    add_net(n.label);
}

/****
*
* net class functions
*
****/

void net::add_cell(string s) {
    assert(s.length() > 0);
    cell_labels.insert(s);
}

void net::add_cell(cell& c) {
    add_cell(c.label);
}

vector<cell*> net::get_cells() {
    return vector<cell*>();
}

net::net(string l) {
    label = l;
}
