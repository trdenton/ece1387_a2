#include "fabric.h"
#include "circuit.h"
#include "spdlog/spdlog.h"
#include <math.h>
#include <vector>
#include <queue>
#include <stack>
#include <map>

// forward declarations
static double compute_cost(bin* bi, bin* bk);

using namespace std;

fabric::fabric(int x, int y) {
    bins = new bin**[x+1];
    width=x;
    height=y;
    for(int i = 0; i <= x; i ++) {      // TODO this seems weird but input file necessitated?
        bins[i] = new bin*[y+1];
        for(int j = 0; j <= y; j++) {
            bins[i][j] = new bin;
            bins[i][j]->x = (double)i;
            bins[i][j]->y = (double)j;
            bins[i][j]->usable = true;
            bins[i][j]->capacity = 1; // assumption for this assignment
        }
    }
}

fabric::~fabric() {
    for(int i = 0; i <= width; i++) {
        for(int j = 0; j <= height; j++) {
            delete bins[i][j];
        }
        delete bins[i];
    } 
}

bin* fabric::get_bin(int x, int y) {
    return bins[x][y]; 
}

void fabric::foreach_bin(void (*fn)(bin* b)) {
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            fn(get_bin(i,j));
        }
    } 
}

void fabric::mark_obstruction(int x0, int y0, int x1, int y1) {
    int x0p = min(x0, width);
    int x1p = min(x1, width);
    int y0p = min(y0, height);
    int y1p = min(y1, height);

    if (x0p != x0)
        spdlog::warn("invalid x0, truncating");
    if (x1p != x1)
        spdlog::warn("invalid x1, truncating");
    if (y0p != y0)
        spdlog::warn("invalid y0, truncating");
    if (y1p != y1)
        spdlog::warn("invalid y1, truncating");

    for(int i = x0p; i <= x1p; ++i) {
        for(int j = y0p; j <= y1p; ++j) {
            bins[i][j]->usable=false;
        }
    }
}

void fabric::map_cells(vector<cell*> cells) {
    for(auto& c: cells) {
        pair<double,double> coords = c->get_coords();
        int x = round(get<0>(coords));
        int y = round(get<1>(coords));
        bins[x][y]->cells.push_back(c);
    }
}

bool fn_sort_candidate_paths(queue<bin*> i, queue<bin*> j) {
    // compare cost
    // need last element and first element
    double cost_i = compute_cost(i.front(), i.back());
    double cost_j = compute_cost(j.front(), j.back());
    return cost_i < cost_j;
}

double distance_cell_to_bin(cell* i, bin* j) {
    pair<double,double> p0 = i->get_coords();
    double x1 = j->x;
    double y1 = j->y;
    
    double dx = x1 - get<0>(p0);
    double dy = y1 - get<1>(p0);
    
    return sqrt(dx*dx + dy*dy);
}

bin* g_nearest_ref;
bool fn_sort_nearest(cell* i, cell* j) {
    return (distance_cell_to_bin(i,g_nearest_ref) < distance_cell_to_bin(j,g_nearest_ref));
}

void fabric::move_along_path(queue<bin*> path, double psi) {
    stack<bin*> S;
    bin* vsrc = path.front(); path.pop();
    S.push(vsrc);
    while(!path.empty()) {
        bin* vsink = path.front(); path.pop(); // starts with an empty bin
        double cost = compute_cost(vsrc,vsink);
        if (cost < psi) {
            S.push(vsink);
        } else {
            return;
        }
    }
    bin* vsink = S.top(); S.pop();
    while(!S.empty()) {
        vsrc = S.top(); S.pop();
        vector<cell*> cells = vector<cell*>(vsrc->cells);
        g_nearest_ref = vsink;
        sort(vsrc->cells.begin(), vsrc->cells.end(), fn_sort_nearest);
        vsink->cells.push_back(vsrc->cells[0]);        
        vsrc->cells.erase(vsrc->cells.begin());
        vsink = vsrc;
    }
}

void fabric::run_flow(double (*psi)(int iter, psi_params* h), psi_params* h) {
    int iter=0;
    if (psi == nullptr || h == nullptr) {
        spdlog::error("psi or psi_param is null");
    }
    while (get_overused_bins().empty() == false) {
        double p = psi(iter,h);
        run_flow_iter(p);
        iter++;
    }
}
void fabric::run_flow_iter(double psi) {
    vector<queue<bin*>> candidate_paths ;
    bin* bi = get_overused_bins()[0];
    candidate_paths = find_candidate_paths(bi, psi);
    sort(candidate_paths.begin(), candidate_paths.end(), fn_sort_candidate_paths);
    for(auto& pk : candidate_paths) {
        if (bi->supply() > 0) {
            move_along_path(pk, psi);
        }
    }
}

bool fn_sort_bin_supply(bin* i, bin* j) {
    return i->supply() < j->supply();
}

vector<bin*> fabric::get_neighbours(bin* b) {
    vector<bin*> ns;
    int x = b->x;
    int y = b->y;

    if ((x - 1 >= 0) && get_bin(x-1,y)->usable)
        ns.push_back(get_bin(x-1,y));

    if ((x + 1 < width) && get_bin(x+1,y)->usable)
        ns.push_back(get_bin(x+1,y));

    if ((y - 1 >= 0) && get_bin(x,y-1)->usable)
        ns.push_back(get_bin(x,y-1));

    if ((y + 1 < height) && get_bin(x,y+1)->usable)
        ns.push_back(get_bin(x,y+1));

    return ns;
}

static double compute_cost(bin* bi, bin* bk) {
    double dx = bk->x - bi->x;
    double dy = bk->y - bi->y;
    return (dx*dx) + (dy*dy);
}

vector<queue<bin*>> fabric::find_candidate_paths(bin* bi, double psi) {
    vector<queue<bin*>> P; // these are complete paths
    queue<queue<bin*>> paths; // this is our working FIFO of paths
    // mark all bins as not visited
    // mark this particular bin as visited

    // a path is a queue<bin*>
    int demand = 0;

    map<bin*,bool> bins_visited;
    // semantics: if it isnt in the map, it hasnt been visited
    bins_visited[bi] = true;
    
    queue<bin*> first_path; 
    first_path.push(bi);
    paths.push(first_path);

    while (paths.empty() == false && demand < bi->supply()) {
        queue<bin*> p;
        p = paths.front(); paths.pop();

        // get possible next paths
        bin* tailbin = p.front();
        vector<bin*> neighbours = get_neighbours(tailbin);
        for(auto& bk : neighbours) {
            if (bins_visited.find(bk) == bins_visited.end()) { // if not visited
                double cost = compute_cost(bi, bk);
                if (cost < psi) {
                    auto pcopy = queue<bin*>(p);
                    pcopy.push(bk);

                    if (bk->supply() == 0) {
                        P.push_back(pcopy);
                        demand++;
                    } else {
                        paths.push(pcopy);
                    }

                }
                bins_visited[bk] = true;
            }
        }
    }
    
    
    return P;
}

vector<bin*> fabric::get_overused_bins() {
    vector<bin*> result;
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            bin* b = get_bin(i,j);
            if (b->supply() > 0)
                result.push_back(b);
        }
    } 

    // sort by amount of oversupply
    sort(result.begin(), result.end(), fn_sort_bin_supply);
    return result;
}
