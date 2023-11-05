#ifndef __FABRIC_H__
#define __FABRIC_H__
#include <vector>
#include <algorithm>
#include <queue>
#include "circuit.h"
#include "psis.h"

using namespace std;

struct bin {
    double x;
    double y;
    int capacity;
    bool usable;
    vector<cell*> cells;
    int supply() {return max(0, usage() - capacity);};
    //int demand() {return 0.;};
    int usage() {return cells.size();};
    double map_cell(cell* c);
    void remove_cell(cell* c);
};

// path - is it just a list of bins?
// . Each path is a queue of bins whose first element is the overflowed bin λi . 

class fabric {
    private:
        bin*** bins;
        int width, height;
    public:
        fabric(int x, int y);
        ~fabric();
        void mark_obstruction(int x0, int y0, int x1, int y1);
        bin* get_bin(int x, int y);
        void map_cells(vector<cell*> cells);
        void run_flow_iter(double);
        void foreach_bin(void (*fn)(bin* b));
        vector<bin*> get_overused_bins();
        vector<bin*> get_neighbours();
        vector<queue<bin*>> find_candidate_paths(bin*,double);
        vector<bin*> get_neighbours(bin* b);
        void move_cells_over(queue<bin*> path);
        void run_flow(double (*psi)(int iter, psi_params* h), psi_params* h);
};

#endif
