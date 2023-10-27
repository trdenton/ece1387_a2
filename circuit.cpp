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
    build_adjacency_matrix();
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
        //this->get_net(net)->add_cell(*c);
        nets[net]->add_cell(*c);
        //(nets.find(net))->add_cell(*c);
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

void circuit::build_adjacency_matrix() {
    int ncells = cells.size();
    int max_size = ncells*(ncells/2);

    int *Ap = new int[max_size];
    int *Ai = new int[max_size];
    fill(Ap,Ap+max_size,0);
    fill(Ai,Ai+max_size,0);

    double *Ax = new double[max_size];
    fill(Ax,Ax+max_size,0.);
    double *b = new double[ncells];
    fill(b,b+ncells,0.);
    double *x = new double[ncells];
    fill(b,b+ncells,0.);

    // stored in compressed sparse column format
    
    int ap_idx=0, ai_idx=0, new_column_id=0;
    bool started_new_column = true; // latches at most once per column
    for(int x = 0; x < ncells; ++x) {
        for(int y = 0; y < ncells; ++y) {
            if (x==y)
                continue; // no connections to itself
            cell* xcell = cells[x];
            cell* ycell = cells[y];
            if (xcell->is_connected_to(ycell)) {
                // put clique weight in this cell location
                Ax[ai_idx] = 1.0;
                Ai[ai_idx] = y;
                ++ai_idx;
                if (!started_new_column) {
                    started_new_column = true;
                    new_column_id = ai_idx-1;
                }
            }
        }
        if (started_new_column) {
            Ap[ap_idx] = new_column_id;
            ++ap_idx;
            started_new_column = false;
        }
    }
#if 0 
    cerr << "checking by inspection...." << endl;
    cerr << "Ap: [ ";
    for(int i = 0; i < ap_idx; ++i) {
        cerr << Ap[i] << (i==ap_idx-1? "":", ");
    }
    cerr << " ]" << endl;

    cerr << "Ai: [ ";
    for(int i = 0; i < ai_idx; ++i) {
        cerr << Ai[i] << (i==ai_idx-1? "":", ");
    }
    cerr << " ]" << endl;

    cerr << "Ax: [ ";
    for(int i = 0; i < ai_idx; ++i) {
        cerr << Ax[i] << (i==ai_idx-1? "":", ");
    }
    cerr << " ]" << endl;
#endif
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

bool cell::is_connected_to(cell* other) {
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
    return !intersection.empty();
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
    return (double)(num_pins()-1);
}
