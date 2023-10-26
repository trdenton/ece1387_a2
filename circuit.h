#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <unordered_set>
#include <string>
#include <iostream>
#include <queue>
#include <sstream>
#include <inttypes.h>
#include <limits.h>
#include "spdlog/spdlog.h"

using namespace std;

class block;
class net_hash;

class net {
    public:
        float weight;
        string label;
        net(string l);
        vector<block*> get_blocks();
        bool operator==(const net& other) const {
            return this->label == other.label;
        }
};

class net_hash {
    public:
        size_t operator() (const net& n) const {
            return n.label.length();
        }
};

class block {
    private:
        std::unordered_set<net, net_hash> nets;
        int x;
        int y;
        unordered_set<string> net_labels;

    public:
        string label;
        block(vector<string> s);
        void connect(block* other);
        void set_coords(int _x, int _y);
        pair<int,int> get_coords();
        unordered_set<string> get_net_labels();
        void add_net(net& n);
        void add_net(string s);

};

class circuit {
    private:
        vector<block*> blocks;
        unordered_set<net, net_hash> nets;

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
        void add_net(string s);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
