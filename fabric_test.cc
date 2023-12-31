#include <gtest/gtest.h>
#include <vector>
#include <unordered_set>
#include <utility>
#include "circuit.h"
#include "fabric.h"

TEST(Fabric, base) {
    fabric* fab = new fabric(10,10);

    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            bin* b = fab->get_bin(i,j);
            ASSERT_EQ(b->x, (double)i);
            ASSERT_EQ(b->y, (double)j);
            ASSERT_TRUE(b->usable);
        }
    }

    delete fab;
}

TEST(Fabric, obstruction) {
    fabric* fab = new fabric(10,10);
    fab->mark_obstruction(1,1,4,4);

    ASSERT_TRUE( fab->get_bin(0,0)->usable);
    ASSERT_TRUE( fab->get_bin(0,1)->usable);
    ASSERT_TRUE( fab->get_bin(0,5)->usable);
    ASSERT_TRUE( fab->get_bin(1,5)->usable);
    ASSERT_TRUE( fab->get_bin(5,5)->usable);
    ASSERT_TRUE( fab->get_bin(5,0)->usable);
    ASSERT_TRUE( fab->get_bin(5,1)->usable);

    ASSERT_FALSE( fab->get_bin(1,1)->usable);
    ASSERT_FALSE( fab->get_bin(1,2)->usable);
    ASSERT_FALSE( fab->get_bin(1,3)->usable);
    ASSERT_FALSE( fab->get_bin(1,4)->usable);

    ASSERT_FALSE( fab->get_bin(2,1)->usable);
    ASSERT_FALSE( fab->get_bin(2,2)->usable);
    ASSERT_FALSE( fab->get_bin(2,3)->usable);
    ASSERT_FALSE( fab->get_bin(2,4)->usable);

    ASSERT_FALSE( fab->get_bin(3,1)->usable);
    ASSERT_FALSE( fab->get_bin(3,2)->usable);
    ASSERT_FALSE( fab->get_bin(3,3)->usable);
    ASSERT_FALSE( fab->get_bin(3,4)->usable);

    ASSERT_FALSE( fab->get_bin(4,1)->usable);
    ASSERT_FALSE( fab->get_bin(4,2)->usable);
    ASSERT_FALSE( fab->get_bin(4,3)->usable);
    ASSERT_FALSE( fab->get_bin(4,4)->usable);

    delete fab;
}

TEST(Fabric, map_cells) {
    fabric* fab = new fabric(10,10);
    vector<string> nets = {"a","b"}; // irrelevant
    cell c1(nets);
    cell c2(nets);
    cell c3(nets);
    cell c4(nets);

    c1.set_coords(1.1,1.1);
    c2.set_coords(0.0,1.0);
    c3.set_coords(5.1,5.4);
    c4.set_coords(5.1,5.4);
    vector<cell*> cells = {&c1,&c2,&c3,&c4};

    fab->map_cells(cells);
    // should map to floor,floor

    ASSERT_TRUE(fab->get_bin(0,0)->cells.empty());

    ASSERT_FALSE( fab->get_bin(1,1)->cells.empty());
    ASSERT_FALSE( fab->get_bin(0,1)->cells.empty());
    ASSERT_FALSE( fab->get_bin(5,5)->cells.empty());

    ASSERT_EQ( fab->get_bin(1,1)->cells.front(), &c1 );
    ASSERT_EQ( fab->get_bin(0,1)->cells.front(), &c2 );
    ASSERT_EQ( fab->get_bin(5,5)->cells.front(), &c3 );
    ASSERT_EQ( fab->get_bin(5,5)->cells.back(), &c4 );
    
    delete fab;
}

TEST(Fabric, supply_calc) {
    fabric* fab = new fabric(10,10);
    vector<string> nets = {"a","b"}; // irrelevant
    cell c1(nets);
    cell c2(nets);
    cell c3(nets);
    cell c4(nets);
    cell c5(nets);
    cell c6(nets);
    vector<cell*> cells = {&c1,&c2,&c3,&c4,&c5,&c6};

    c1.set_coords(1.1,1.1);
    c2.set_coords(0.0,1.0);
    c3.set_coords(5.1,5.4);
    c4.set_coords(5.1,5.4);
    c5.set_coords(0.4,1.4);
    c6.set_coords(0.4,1.4);

    fab->map_cells(cells);
    
    ASSERT_EQ( fab->get_bin(1,1)->supply(), 0);
    ASSERT_EQ( fab->get_bin(5,5)->supply(), 1);
    ASSERT_EQ( fab->get_bin(0,1)->supply(), 2);
    delete fab;
}

TEST(Fabric, get_overused_bins) {
    fabric* fab = new fabric(10,10);
    vector<string> nets = {"a","b"}; // irrelevant
    cell c1(nets);
    cell c2(nets);
    cell c3(nets);
    cell c4(nets);
    cell c5(nets);
    cell c6(nets);
    vector<cell*> cells = {&c1,&c2,&c3,&c4,&c5,&c6};

    c1.set_coords(1.1,1.1);
    c2.set_coords(0.0,1.0);
    c3.set_coords(5.1,5.4);
    c4.set_coords(5.1,5.4);
    c5.set_coords(0.4,1.4);
    c6.set_coords(0.4,1.4);

    fab->map_cells(cells);
    
    vector<bin*> overused = fab->get_overused_bins();
    ASSERT_EQ(overused[0]->supply(), 1);
    ASSERT_EQ(overused[1]->supply(), 2);
    delete fab;
}

TEST(Fabric, get_neighbours) {
    fabric* fab = new fabric(10,10);

    vector<bin*> n1 = fab->get_neighbours(fab->get_bin(3,3));
    ASSERT_EQ(n1.size(),4);
    delete fab;
}

TEST(Fabric, get_neighbours_edge) {
    fabric* fab = new fabric(10,10);

    vector<bin*> n1 = fab->get_neighbours(fab->get_bin(0,3));
    ASSERT_EQ(n1.size(),3);
    delete fab;
}

TEST(Fabric, get_neighbours_unusable) {
    fabric* fab = new fabric(10,10);
    fab->mark_obstruction(2,3,2,3); // this is exactly one block

    vector<bin*> n1 = fab->get_neighbours(fab->get_bin(3,3));
    ASSERT_EQ(n1.size(),3);
    delete fab;
}

TEST(Fabric, move_along_path) {
    fabric* fab = new fabric(10,10);
    
    vector<string> nets = {"a","b"}; // irrelevant
    cell c0(nets);
    cell c1(nets);
    cell c2(nets);

    c0.set_coords(0.1,0.1);
    c1.set_coords(0.2,0.2);
    c2.set_coords(0.3,1.1);
    
    vector<cell*> cells = {&c0,&c1,&c2};
    fab->map_cells(cells);

    queue<bin*> pk;

    // the first one is the source cell
    bin* b00 = fab->get_bin(0,0);   // c0 and c1 are here
    bin* b01 = fab->get_bin(0,1);   // c2 is here
    bin* b02 = fab->get_bin(0,2);

    pk.push(b00);
    pk.push(b01);
    pk.push(b02);

    fab->move_along_path(pk,99999.);

    //started with c1 and c2 in b00, c3 in b01
    //should end with c1 in b00, c2 in b01, c3 in b02
    ASSERT_FALSE(b02->cells.empty());
    ASSERT_FALSE(b01->cells.empty());
    ASSERT_FALSE(b00->cells.empty());
    
    ASSERT_EQ(b02->cells.size(), 1);
    ASSERT_EQ(b01->cells.size(), 1);
    ASSERT_EQ(b00->cells.size(), 1);

    ASSERT_EQ(b02->cells.front(), &c2);
    ASSERT_EQ(b01->cells.front(), &c1);
    ASSERT_EQ(b00->cells.front(), &c0);


    delete fab;
}
