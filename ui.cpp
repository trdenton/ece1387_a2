#include <stdio.h>
#include <string>
#include "easygl/graphics.h"
#include "spdlog/spdlog.h"
#include "circuit.h"
#include "ui.h"
#include "fabric.h"
#include <condition_variable>
using namespace std;

// Callbacks for event-driven window handling.
void ui_drawscreen();
void ui_click_handler (float x, float y);
void ui_mouse_handler (float x, float y);
void ui_key_handler(char c);

float logic_cell_width = 10.0;

circuit* circ;
fabric* fab;


void ui_pump(void (*draw)()) {
    circuit_next_step();
    draw();
}

void ui_init(circuit* circuit, fabric* fabric) {
    circ = circuit;
    fab = fabric;
    spdlog::info("Init UI");
    init_graphics("A1", BLACK);
    create_button("Proceed","PUMP", ui_pump);
    //init_world(-50.,-50.,150.,150.);
    init_world(-50.,150.,150.,-50.);
    //set_keypress_input(true);
    //set_mouse_move_input(true);
    event_loop(ui_click_handler, ui_mouse_handler, ui_key_handler, ui_drawscreen);   
}

void ui_teardown() {
    close_graphics ();
}

void ui_drawscreen() {
    clearscreen();
	set_draw_mode (DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
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
    drawrect(x - width/2, y - height/2, x + width/2, y + width/2);
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
            char buff[32] = {'\0'};
            snprintf(buff,32,"%s: %f",n->label.c_str(), n->get_weight());
            spdlog::debug("weight is {}", n->get_weight());
            drawtext((x0+x1)/2., (y0+y1)/2., buff, 10.0);
        }
        sp = s;
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
    ui_draw_rats_nest(circ);
    ui_draw_cells(circ);
}
