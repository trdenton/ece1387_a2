#include "fabric.h"
#include "circuit.h"
#include "spdlog/spdlog.h"
#include <vector>

using namespace std;

fabric::fabric(int x, int y) {
    bins = new bin**[x];
    width=x;
    height=y;
    for(int i = 0; i < x; i ++) {
        bins[i] = new bin*[y];
        for(int j = 0; j < y; j++) {
            bins[i][j] = new bin;
            bins[i][j]->x = (double)i;
            bins[i][j]->y = (double)j;
            bins[i][j]->usable = true;
        }
    }
}

fabric::~fabric() {
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            delete bins[i][j];
        }
        delete bins[i];
    } 
}

bin* fabric::get_bin(int x, int y) {
    return bins[x][y]; 
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
