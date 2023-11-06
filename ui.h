#ifndef __UI_H__
#define __UI_H__
#include "circuit.h"
#include "fabric.h"
#include <string>
using namespace std;
void ui_init(circuit*,fabric*,flow_state*);
void ui_teardown();

//drawing functions
void ui_draw(circuit*);
void ui_draw_fs(flow_state*);
#endif
