#include "circuit.h"
#include <string>
#include <math.h>
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
#include "fabric.h"

#ifndef GTEST
#include "umfpack.h"
#endif

using namespace std; 

condition_variable cv;
mutex cv_m;
const string NO_MUTUAL_LABEL = "NO_MUTUAL_LABEL";
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
    fixed_weight_bias = 0;

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
                    add_cell_fixed_coords(vstrings);
                    break;
            }
        }
        infile.close();
    } else {
        spdlog::error("Could not open {}", file);
    }
}

net* circuit::get_net(string label) {
    //net n(label);
    //auto it = nets.find(n);
    //return &n;
    return nets[label];
}

bool circuit::fit(bool interactive) {
    return true;
}

void circuit::add_net(string s) {
    net* n = new net(s);
    if (nets.find(s) == nets.end()) {
        nets[s] = n;
    } else {
        delete(n);
    }
}

void circuit::add_cell_connections(vector<string> toks) {
    cell* c = new cell(toks);
    cells.push_back(c);

    vector<string> s_nets = std::vector<string>(toks.begin()+1,toks.end()-1);
    for(string net : s_nets) {
        add_net(net);
        c->add_net(net);
        nets[net]->add_cell(*c);
    }

}

void circuit::add_cell_fixed_coords(vector<string> s) {
    cell* c = get_cell(s[0]);
    int x = stoi(s[1]);
    int y = stoi(s[2]);
    c->set_coords(x,y,true);
    fixed_cell_labels.insert(s[0]);
}

cell* circuit::get_cell(string label) {
    for (auto b : cells) {
        if (b->label == label) {
            return b;
        }
    }
    return nullptr;
}

void circuit::build_solver_rhs(fabric* fab) {
    for(auto& c : cells) {
        if (c->is_fixed())
            continue;
        double xval=0., yval=0.;
        if (connects_to_fixed_cell(c)) {
            vector<cell*> others = get_connected_fixed_cells(c);
            for (auto& other : others) {
                pair<double,double> coords = other->get_coords();
                xval += (fixed_weight_bias + get_clique_weight(c,other))*get<0>(coords);   // w_i,z * x_z
                yval += (fixed_weight_bias + get_clique_weight(c,other))*get<1>(coords);   // w_i,z * y_z

                if (fixed_weight_bias != 0) {
                    spdlog::debug("adding fixed weight to RHS cell: {} other: {}", c->label, other->label);
                }

                spdlog::debug("cell {} wiz {} xz {} yz {} ", c->label, get_clique_weight(c,other), get<0>(coords), get<1>(coords));
            }
        } 

        if (fab != nullptr) {
            for(bin* b : fab->get_used_bins()) {
                if (find(b->cells.begin(), b->cells.end(), c) != b->cells.end()) {
                    spdlog::debug("found bin for cell @ {}, {}", b->x, b->y);
                    xval += (double)(fab->spread_weight * b->x);
                    yval += (double)(fab->spread_weight * b->y);
                    break;
                }
            }
        }

        Q->Cx.push_back(xval);
        Q->Cy.push_back(yval);
    }
}

void circuit::build_solver_matrix(fabric* fab) {
    int ncells = cells.size();
    Q = new solver_matrix();
    Q->n = 0;

    // stored in compressed sparse column format
    
    // cerr << "ncells " << ncells << endl;
    // cerr << "maxn " << 1+(ncells*ncells)/2 << endl;
    int last_pushed_x = -1;
    for(int x = 0; x < ncells; ++x) {
        cell* xcell = cells[x];
        if (xcell->is_fixed())
            continue;

        int movable_y_count = 0;
        for(int y = 0; y < ncells; ++y) {
            cell* ycell = cells[y];
            double val = 0.;
            
            if (ycell->is_fixed())
                continue;

            ++movable_y_count;

            if (x==y)
                val = sum_all_connected_weights(xcell, fab);
            else if (xcell->is_connected_to(ycell))
                val = -1*get_clique_weight(xcell,ycell);
            else
                continue;   //no connection, this becomes a zero

            // put clique weight in this cell location
            if (last_pushed_x != Q->n) {
                last_pushed_x = Q->n;
                Q->Ap.push_back(Q->Ax.size());
            }
            Q->Ax.push_back(val);
            Q->Ai.push_back(movable_y_count-1);

        }
        ++Q->n;
    }
    Q->Ap.push_back(Q->Ax.size());
#if 0 
    cerr << "checking by inspection...." << endl;
    cerr << "n: " << Q->n << endl;
    cerr << "Ap: [ ";
    for(auto& i : Q->Ap) {
        cerr << i << ", ";
    }
    cerr << " ]" << endl;

    cerr << "Ai: [ ";
    for(auto& i : Q->Ai) {
        cerr << i << ", ";
    }
    cerr << " ]" << endl;

    cerr << "Ax: [ ";
    for(auto& i : Q->Ax) {
        cerr << i << ", ";
    }
    cerr << " ]" << endl;
#endif
}

solver_matrix* circuit::get_solver_matrix() {
    return Q;
}

void circuit::umfpack(enum axis ax, double* res) {
    #ifndef GTEST
    int rc;
    double *null = (double *) NULL ;
    void *Symbolic, *Numeric ;

    rc = umfpack_di_symbolic (Q->n, Q->n, Q->get_Ap_ss(), Q->get_Ai_ss(), Q->get_Ax_ss(), &Symbolic, null, null) ;
    if (rc != UMFPACK_OK) {
        spdlog::error("Error in umfpack_di_symbolic: {}", rc);
    }

    rc = umfpack_di_numeric (Q->get_Ap_ss(), Q->get_Ai_ss(), Q->get_Ax_ss(), Symbolic, &Numeric, null, null) ;
    if (rc != UMFPACK_OK) {
        spdlog::error("Error in umfpack_di_numeric: {}", rc);
    }

    umfpack_di_free_symbolic (&Symbolic) ;

    rc = umfpack_di_solve (UMFPACK_A, Q->get_Ap_ss(), Q->get_Ai_ss(), Q->get_Ax_ss(), res, Q->get_C_ss(ax), Numeric, null, null) ;
    if (rc != UMFPACK_OK) {
        spdlog::error("Error in umfpack_di_solve: {}", rc);
    }

    umfpack_di_free_numeric (&Numeric) ;
    #endif
}

void circuit::iter(fabric* fab) {
    build_solver_matrix(fab);
    build_solver_rhs(fab);

    double* x = new double[Q->n];
    double* y = new double[Q->n];
    int i=0;

    umfpack(X, x);
    umfpack(Y, y);

    for (auto& cell : cells) { 
        if (!cell->is_fixed()) {
            spdlog::debug("cell {} x {} y {}", cell->label, x[i], y[i]);
            cell->set_coords(x[i],y[i]);
            ++i;
        }
    }

    assert(i == Q->n);
    spdlog::info("HPWL: {}", hpwl());

    delete[] x;
    delete[] y;
}

double circuit::get_clique_weight(cell* c1, cell* c2) {
    // need to get the common net
    vector<string> net_labels = c1->get_mutual_net_labels(c2);
    double result = 0.;
    for (auto& s : net_labels) {
        net* n = get_net(s);
        result += n->get_weight();
    }
    return result;
}

double circuit::sum_all_connected_weights(cell* c, fabric* fab) {
    double result = 0.0;
    
    spdlog::debug("cell {}:", c->label);
    for (auto& s : c->get_net_labels()) {
        net* n = get_net(s);

        for (auto& cell_label : n->get_cell_labels()) {
            cell* other = get_cell(cell_label);
            if (other != c) {
                double addition = n->get_weight();
                if (other->is_fixed()) {
                    addition += fixed_weight_bias;
                    spdlog::debug("adding fixed weight bias to cell {} other {} by net {}", c->label, other->label, s);
                }
                spdlog::debug("\tadding net {}, weight {} (from cell {})", n->label, addition, other->label);
                result += addition;
            }
        }
    }

    if (fab != nullptr) {
        for (bin* b : fab->get_used_bins()) {
            if (find(b->cells.begin(), b->cells.end(), c) != b->cells.end()) {
                spdlog::debug("adj: found bin for cell @ bin {}, {}", b->x, b->y);
                result += fab->spread_weight;
                break;
            }
        }
    }

    spdlog::debug("\ttotal: {}", result);

    return result;
}

circuit::~circuit() {
    if (Q)
        delete(Q);
}

bool circuit::connects_to_fixed_cell(cell* c1) {
    return (!get_connected_fixed_cells(c1).empty());
}

vector<cell*> circuit::get_connected_fixed_cells(cell* c1) {
    vector<cell*> result;
    for(auto& nl : fixed_cell_labels) {
        cell* other = get_cell(nl);
        if ( c1->is_connected_to(other) ) {
            result.push_back(other);
        }
    }
    return result;
}

void circuit::foreach_cell(void (*fn)(circuit* circ, cell* c)) {
    for(auto* c : cells) {
        fn(this,c);
    }
}

void circuit::foreach_net(void (*fn)(circuit* circ, net* c)) {
    for(auto& c : nets) {
        fn(this, c.second);
    }
}

double circuit::hpwl() {
    double min_x=0.;
    double max_x=0.;
    double min_y=0.;
    double max_y=0.;
    
    bool first = true;

    for(auto* c: cells) {
        if (!c->is_fixed()) {
            pair<double,double> coords = c->get_coords();
            double x = get<0>(coords);
            double y = get<1>(coords);
            if (first) {
                min_y = y;
                min_x = x;
                max_y = y;
                max_x = x;
                first = false;
            }

            if (y < min_y)
                min_y = y; 
            if (y > max_y)
                max_y = y;

            if (x > max_x)
                max_x = x;
            if (x < min_x)
                min_x = x; 
        }
    }
    return (max_x - min_x) + (max_y - min_y);
}

void circuit::set_fixed_weight_bias(double n) {
    spdlog::debug("WE CHANGE TO {}", n);
    fixed_weight_bias = n;
}

/****
*
* cell class functions
*
****/

bool cell::is_fixed() {
    return fixed;
}

cell::cell(vector<string> s) {
    x = 0;
    y = 0;
    label = s[0];
    fixed = false;
    //nets = std::vector<string>(s.begin()+1,s.end()-1);
}

void cell::set_coords(double _x, double _y, bool _fixed) {
    x = _x;
    y = _y;
    fixed = _fixed;
}

unordered_set<string> cell::get_net_labels() {
    return net_labels;
}

pair<double,double> cell::get_coords() {
    return pair<double,double>(x,y);
}

void cell::add_net(string s) {
    assert(s.length() > 0);
    net_labels.insert(s); 
}

void cell::add_net(net& n) {
    add_net(n.label);
}

bool cell::is_connected_to(cell* other) {
    return (get_mutual_net_labels(other).size() > 0);
}

vector<string> cell::get_mutual_net_labels(cell* other) {
    vector<string> my_net_labels_ordered;
    vector<string> other_net_labels_ordered;
    
    
    for (auto n : net_labels) {
        my_net_labels_ordered.push_back(n);
    }
    
    for (auto n : other->net_labels) {
        other_net_labels_ordered.push_back(n);
    }

    sort(my_net_labels_ordered.begin(), my_net_labels_ordered.end());
    sort(other_net_labels_ordered.begin(), other_net_labels_ordered.end());

    vector<string> intersection;

    set_intersection(my_net_labels_ordered.begin(), my_net_labels_ordered.end(),
                     other_net_labels_ordered.begin(), other_net_labels_ordered.end(),
                     back_inserter(intersection));
    return intersection;
}

double cell::distance_to(cell* other) {
    pair<double,double> p0 = get_coords(), p1 = other->get_coords();

    double dx = get<0>(p1) - get<0>(p0);
    double dy = get<1>(p1) - get<1>(p0);

    return sqrt(dx*dx + dy*dy);
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

net::net(string l) {
    label = l;
}

unordered_set<string> net::get_cell_labels() {
    return cell_labels;
}

int net::num_pins() {
    return cell_labels.size();
}

double net::get_weight() {
    return (double)(2./(double)(num_pins()));
}

/****
*
* solver_matrix struct functions
*
****/

int* solver_matrix::get_Ap_ss() {
    return &Ap[0];
}

int* solver_matrix::get_Ai_ss() {
    return &Ai[0];
}

double* solver_matrix::get_Ax_ss() {
    return &Ax[0];
}

double* solver_matrix::get_C_ss(enum axis ax) {
    if (ax == X) 
        return &Cx[0];
    if (ax == Y)
        return &Cy[0];
    spdlog::error("Received invalid axis {}", ax);
    return nullptr;
}
