#ifndef __FABRIC_H__
#define __FABRIC_H__
#include <vector>
#include <algorithm>
#include "circuit.h"

using namespace std;

struct bin {
    double x;
    double y;
    int capacity;
    bool usable;
    vector<cell*> cells;
    int supply() {return max(0, usage() - capacity);};
    int demand() {return 0.;};
    int usage() {return cells.size();};
    double map_cell(cell* c);
    void remove_cell(cell* c);
};

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
        void run_flow(double (*psi)(int iter));
        void foreach_bin(void (*fn)(bin* b));
};

#endif
