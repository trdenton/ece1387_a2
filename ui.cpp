#include <stdio.h>
#include <string>
#include "easygl/graphics.h"
#include "spdlog/spdlog.h"
#include "circuit.h"
#include "ui.h"
#include "fabric.h"
#include <condition_variable>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
using namespace std;

// Callbacks for event-driven window handling.
void ui_drawscreen();
void ui_click_handler (float x, float y);
void ui_mouse_handler (float x, float y);
void ui_key_handler(char c);
void ui_draw_bin__path(vector<bin*> b);

float logic_cell_width = 10.0;

circuit* circ;
fabric* fab;
flow_state* fs;

bool draw_rats_nest = true;
bool draw_cells = true;
bool draw_paths = true;

sig_atomic_t update;

void ui_pump(void (*draw)()) {
    circuit_next_step();
    draw();
}

void ui_toggle_rat(void(*draw)()) {
    draw_rats_nest = !draw_rats_nest;
    draw();
}

void ui_toggle_cell(void(*draw)()) {
    draw_cells = !draw_cells;
    draw();
}

void ui_toggle_paths(void(*draw)()) {
    draw_paths = !draw_paths;
}

void ui_speed_0x(void(*draw)()) {
    set_interval(0UL);
}

void ui_speed_1x(void(*draw)()) {
    set_interval(10000);
}

void ui_speed_p1x(void(*draw)()) {
    set_interval(100000);
}

void ui_speed_p01x(void(*draw)()) {
    set_interval(1000000);
}

void ui_init(circuit* circuit, fabric* fabric, flow_state* flow_state) {
    circ = circuit;
    fab = fabric;
    fs = flow_state;
    spdlog::info("Init UI");
    init_graphics("A1", BLACK);
    create_button("Proceed","TOGGLE RAT", ui_toggle_rat);
    create_button("TOGGLE RAT","TOGGLE CELL", ui_toggle_cell);
    create_button("TOGGLE CELL","TOGGLE PATHS", ui_toggle_paths);
    create_button("TOGGLE PATHS","SPEED 0x", ui_speed_0x);
    create_button("SPEED 0x","SPEED 1x", ui_speed_1x);
    create_button("SPEED 1x","SPEED .1x", ui_speed_p1x);
    create_button("SPEED .1x","SPEED .01x", ui_speed_p01x);
    init_world(3.,22.,22.,3.);
    set_keypress_input(true);
    //set_mouse_move_input(true);

    event_loop(ui_click_handler, ui_mouse_handler, ui_key_handler, ui_drawscreen);   
}

void ui_teardown() {
    close_graphics ();
}

void ui_drawscreen() {
    static int clear=0;
    set_draw_mode (DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
    if (fs->step) {
        fab->run_flow_step(fs);
        clear++;
        if (clear%10 == 0)
            clearscreen();
    }
    else
        clearscreen();

    ui_draw(circ);
}

void ui_click_handler (float x, float y) {
	spdlog::debug("user clicked at {},{}",x,y);
}

void ui_mouse_handler (float x, float y) {
	spdlog::debug("mouse move {},{}",x,y);
}

void ui_key_handler(char c) {
	spdlog::debug("keypress {}",c);
    if (c=='n') {
        spdlog::debug("NEXT",c);
        clearscreen();
        ui_drawscreen();
        if (fs->step) {
            fab->run_flow_step(fs);
        }
    }
}

void ui_draw_fs(flow_state* fs) {
    for(auto& path : fs->P) {
        queue<bin*> pc = queue<bin*>(path);
        vector<bin*> p;
        while(pc.empty()==false) {
            bin* b = pc.front(); pc.pop();
            p.push_back(b);
        }
        ui_draw_bin__path(p);
    }
}

void ui_draw_cell_fn(circuit* circ, cell* c) {
    double width = 1.;
    double height = width;
    // center at the cells coords
    pair<double,double> p = c->get_coords();
    double x = get<0>(p);
    double y = get<1>(p);

    setcolor(c->is_fixed() ? MAGENTA : GREEN);
    setlinewidth(2);
    drawarc(x,y,width/2.,0.,360.);
    setlinewidth(1);
}

void ui_draw_net_fn(circuit* circ, net* n) {
    // get all pins on net
    // first approach: draw 0 -> 1, 1 -> 2, ... etc
    cell* c1;
    cell* c2;
    string sp = "";
    for (auto& s : n->get_cell_labels()) {
        if ( sp != "" ) {
            c1 = circ->get_cell(s);
            c2 = circ->get_cell(sp);
            double x0 = get<0>(c1->get_coords());
            double y0 = get<1>(c1->get_coords());
            double x1 = get<0>(c2->get_coords());
            double y1 = get<1>(c2->get_coords());
            drawline(x0, y0, x1, y1);
            #if 0
            char buff[32] = {'\0'};
            snprintf(buff,32,"%s: %f",n->label.c_str(), n->get_weight());
            drawtext((x0+x1)/2., (y0+y1)/2., buff, 10.0);
            #endif
        }
        sp = s;
    }
}

void ui_draw_bin__path(vector<bin*> b) {
    setcolor(RED);
    int i = 0;
    char label[32] = {0};
    for(auto& bi : b) {
        double width = 1.;
        double height = width;
        // center at the cells coords
        double x = bi->x;
        double y = bi->y;
        fillrect(x - width/2, y - height/2, x + width/2, y + width/2);
        setcolor(WHITE);
        snprintf(label,32,"p%d",i);
        drawtext(x,y,label,10.0);
        ++i;
        setcolor(BLUE);
    }
}

void ui_draw_bin_fn(bin* b) {
    double width = 1.;
    double height = width;
    // center at the cells coords
    double x = b->x;
    double y = b->y;

    setcolor(!(b->usable) ? DARKGREY : 
            b->supply() > 0 ? YELLOW : 
            LIGHTGREY);

    ( b->usable && b->usage() == 0 ? drawrect : fillrect)(x - width/2, y - height/2, x + width/2, y + width/2);
    if (b->supply() > 0) {
        setcolor(BLACK);
        char buff[32] = {'\0'};
        snprintf(buff,32,"%d",b->supply());
        drawtext(x, y, buff, 10.0);
    }
}

void ui_draw_cells(circuit* circ){
    setcolor(GREEN);
    setlinestyle(SOLID);
    setlinewidth(1);

    circ->foreach_cell( ui_draw_cell_fn );
}

void ui_draw_bins(fabric* fab) {
    fab->foreach_bin( ui_draw_bin_fn );
}

void ui_draw_rats_nest(circuit* circ){
    setcolor(WHITE);
    setlinestyle(DASHED);
    setlinewidth(1);
    circ->foreach_net( ui_draw_net_fn );
}

void ui_draw(circuit* circ) {
    ui_draw_bins(fab);
    if (draw_rats_nest)
        ui_draw_rats_nest(circ);
    if (draw_cells)
        ui_draw_cells(circ);
    if (draw_paths)
        ui_draw_fs(fs);
}
