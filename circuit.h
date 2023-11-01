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
        bool fixed;
        unordered_set<string> net_labels;

    public:
        string label;
        cell(vector<string> s);
        void connect(cell* other);
        void set_coords(int _x, int _y, bool _fixed=false);
        pair<int,int> get_coords();
        unordered_set<string> get_net_labels();
        void add_net(net& n);
        void add_net(string s);
        bool is_connected_to(cell* other);
        string get_mutual_net_label(cell* other);
        bool is_fixed();
};

struct adjacency_matrix {
    int ncells;
    // the actual adjacency matrix
    vector<int> Ap;
    vector<int> Ai;
    vector<double> Ax;

    // the RHS of the equation
    vector<double> C;

    int* get_Ap_ss();
    int* get_Ai_ss();
    double* get_Ax_ss();
    double* get_C_ss();
    
};

class circuit {
    private:
        adjacency_matrix* Q;
        vector<cell*> cells;
        unordered_set<string> fixed_cell_labels;
        unordered_map<string, net*> nets;
        void build_adjacency_matrix();
        void build_rhs();

    public:
        circuit(string s);
        ~circuit();
        int get_n_cells() { return cells.size();}

        bool fit(bool interactive);
        cell* get_cell(string label);
        void add_cell_connections(vector<string> toks);
        void add_cell_fixed_coords(vector<string> toks);
        net* get_net(string label);
        void add_net(string s);
        double sum_all_connected_weights(cell* c);
        double get_clique_weight(cell* c1, cell* c2);
        adjacency_matrix* get_adjacency_matrix();
        bool connects_to_fixed_cell(cell* c1);
        cell* get_connected_fixed_cell(cell* c1);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
