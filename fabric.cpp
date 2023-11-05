#include "fabric.h"
#include "circuit.h"
#include "spdlog/spdlog.h"
#include <vector>
#include <queue>
#include <map>

using namespace std;

fabric::fabric(int x, int y) {
    bins = new bin**[x+1];
    width=x;
    height=y;
    for(int i = 0; i <= x; i ++) {      // TODO this seems weird but input file necessitated
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

void fabric::run_flow_iter(double (*psi)(int, psi_params*)) {
    if (psi == nullptr) {
        spdlog::error("psi doesnt look like a function, I'm giving up");
        return;
    }
    queue<queue<bin*>> candidate_paths ;
    for(auto& bin : get_overused_bins()) {
        auto candidate_paths = find_candidate_paths(bin);
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

queue<queue<bin*>> fabric::find_candidate_paths(bin* b) {
    queue<queue<bin*>> paths;
    // mark all bins as not visited
    // mark this particular bin as visited

    // a path is a queue<bin*>

    map<bin*,bool> bins_visited;
    // semantics: if it isnt in the map, it hasnt been visited
    bins_visited[b] = true;
    
    queue<bin*> first_path; 
    first_path.push(b);
    paths.push(first_path);

    while (paths.empty() == false) {
        queue<bin*> current_path;
        current_path = paths.front(); paths.pop();

        // get possible next paths
        bin* b = current_path.front();
        vector<bin*> neighbours = get_neighbours(b);
    }
    
    
    return paths;
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
