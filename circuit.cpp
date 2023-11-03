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
    build_solver_matrix();
    build_solver_rhs();
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

void circuit::build_solver_rhs() {
    for(auto& c : cells) {
        if (c->is_fixed())
            continue;
        double xval=0., yval=0.;
        if (connects_to_fixed_cell(c)) {
            cell* other = get_connected_fixed_cell(c);
            pair<double,double> coords = other->get_coords();
            xval = get_clique_weight(c,other)*get<0>(coords);
            yval = get_clique_weight(c,other)*get<1>(coords);
        } 
        Q->Cx.push_back(xval);
        Q->Cy.push_back(yval);
    }
}

void circuit::build_solver_matrix() {
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
                val = sum_all_connected_weights(xcell);
            else if (xcell->is_connected_to(ycell))
                val = -get_clique_weight(xcell,ycell);
            else
                continue;

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

void circuit::iter() {
    double* x = new double[Q->n];
    double* y = new double[Q->n];
    int i=0;
    
    umfpack(X, x);
    umfpack(Y, y);

    for (auto& cell : cells) { 
        if (!cell->is_fixed()) {
            cell->set_coords(x[i],y[i]);
            ++i;
        }
    }

    assert(i == Q->n);

    delete[] x;
    delete[] y;
}

double circuit::get_clique_weight(cell* c1, cell* c2) {
    // need to get the common net
    string net_label = c1->get_mutual_net_label(c2);
    net* n = get_net(net_label);
    return n->get_weight();
}

double circuit::sum_all_connected_weights(cell* c) {
    double result = 0.0;
    
    for (auto& s : c->get_net_labels()) {
        net* n = get_net(s);
        result += n->get_weight();
    }

    return result;
}

circuit::~circuit() {
    if (Q)
        delete(Q);
}

bool circuit::connects_to_fixed_cell(cell* c1) {
    return (get_connected_fixed_cell(c1) != nullptr);
}

cell* circuit::get_connected_fixed_cell(cell* c1) {
    cell* result = nullptr;
    for(auto& nl : fixed_cell_labels) {
        cell* other = get_cell(nl);
        if ( c1->is_connected_to(other) ) {
            result = other;
            break;
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
    return (get_mutual_net_label(other) != NO_MUTUAL_LABEL);
}

string cell::get_mutual_net_label(cell* other) {
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
    if (intersection.empty()) {
        return NO_MUTUAL_LABEL;
    } else {
        return intersection[0];
    }
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
    return (double)(2./num_pins());
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
}
