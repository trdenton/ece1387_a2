#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>
#include <queue>
#include <sstream>
#include <inttypes.h>
#include <limits.h>
#include "spdlog/spdlog.h"

using namespace std;

class cell;
class net_hash;

class net {
    private:
        unordered_set<string> mutable cell_labels;
    public:
        string label;

        net(string l);
        unordered_set<string> get_cell_labels();
        bool operator==(const net& other) const {
            return this->label == other.label;
        }
        void add_cell(cell& n);
        void add_cell(string c);
        int num_pins();
        double get_weight();
};

class net_hash {
    public:
        size_t operator() (const net& n) const {
            return n.label.length();
        }
};

class cell {
    private:
        int x;
        int y;
        unordered_set<string> net_labels;

    public:
        string label;
        cell(vector<string> s);
        void connect(cell* other);
        void set_coords(int _x, int _y);
        pair<int,int> get_coords();
        unordered_set<string> get_net_labels();
        void add_net(net& n);
        void add_net(string s);
        bool is_connected_to(cell* other);

};

class circuit {
    private:
        vector<cell*> cells;
        unordered_map<string, net*> nets;
        void build_adjacency_matrix();

    public:
        circuit(string s);
        ~circuit() {
        }
        int get_n_cells() { return cells.size();}

        bool fit(bool interactive);
        cell* get_cell(string label);
        void add_cell_connections(vector<string> toks);
        void add_cell_coords(vector<string> toks);
        net* get_net(string label);
        void add_net(string s);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
