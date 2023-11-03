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

enum axis {
    X,
    Y
};

class cell;

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

class cell {
    private:
        double x;
        double y;
        bool fixed;
        unordered_set<string> net_labels;

    public:
        string label;
        cell(vector<string> s);
        void connect(cell* other);
        void set_coords(double _x, double _y, bool _fixed=false);
        pair<double,double> get_coords();
        unordered_set<string> get_net_labels();
        void add_net(net& n);
        void add_net(string s);
        bool is_connected_to(cell* other);
        vector<string> get_mutual_net_labels(cell* other);
        bool is_fixed();
};

struct solver_matrix {
    int n;
    // the actual solver matrix
    vector<int> Ap;
    vector<int> Ai;
    vector<double> Ax;

    // the RHS of the equation
    vector<double> Cx;
    vector<double> Cy;

    int* get_Ap_ss();
    int* get_Ai_ss();
    double* get_Ax_ss();
    double* get_C_ss(enum axis);
    
};

class circuit {
    private:
        int fixed_weight_bias;
        solver_matrix* Q;
        vector<cell*> cells;
        unordered_set<string> fixed_cell_labels;
        unordered_map<string, net*> nets;
        void build_solver_matrix();
        void build_solver_rhs();
        void umfpack(enum axis ax, double* res);

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
        solver_matrix* get_solver_matrix();
        bool connects_to_fixed_cell(cell* c1);
        cell* get_connected_fixed_cell(cell* c1);
        void iter();
        void foreach_cell(void (*fn)(circuit* circ, cell* c));
        void foreach_net(void (*fn)(circuit* circ, net* n));
        double hpwl();
        void set_fixed_weight_bias(int n);
};
void circuit_wait_for_ui();
void circuit_next_step();
#endif
