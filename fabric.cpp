#include "fabric.h"
#include "circuit.h"
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
