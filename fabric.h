#ifndef __FABRIC_H__
#define __FABRIC_H__
#include <vector>
#include "circuit.h"

using namespace std;

struct bin {
    double x;
    double y;
    bool usable;
    vector<cell*> cells;
    double get_supply();
    double get_demand();
    double map_cell(cell* c);
    void remove_cell(cell* c);
};

class fabric {
    private:
        bin*** bins;
    public:
        fabric(int x, int y);
        void add_obstruction(int x0, int y0, int x1, int y1);
        bin* get_bin(int x, int y);
        void map_cells(vector<cell*> cells);
        void run_flow();
};

#endif
