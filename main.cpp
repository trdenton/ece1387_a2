#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <getopt.h>
#include "spdlog/spdlog.h"
#include "version.h"
#include "easygl/graphics.h"
#include "ui.h"
#include "fabric.h"
#include "umfpack.h"
#include "psis.h"
#include <thread>

using namespace std;

void test_suitesparse() {
    int    n = 5 ;
    int    Ap [ ] = {0, 2, 5, 9, 10, 12} ;
    int    Ai [ ] = { 0,  1,  0,   2,  4,  1,  2,  3,   4,  2,  1,  4} ;
    double Ax [ ] = {2., 3., 3., -1., 4., 4., -3., 1., 2., 2., 6., 1.} ;
    double b [ ] = {8., 45., -3., 3., 19.} ;
    double x [5] ;

    double *null = (double *) NULL ;
    int i ;
    void *Symbolic, *Numeric ;
    (void) umfpack_di_symbolic (n, n, Ap, Ai, Ax, &Symbolic, null, null) ;
    (void) umfpack_di_numeric (Ap, Ai, Ax, Symbolic, &Numeric, null, null) ;
    umfpack_di_free_symbolic (&Symbolic) ;
    (void) umfpack_di_solve (UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, null, null) ;
    umfpack_di_free_numeric (&Numeric) ;
    for (i = 0 ; i < n ; i++) printf ("x [%d] = %g\n", i, x [i]) ;
}

void print_usage() {
    cout << "Usage: ./a2 [-hdvis] -f filename" << endl;
    cout << "\t-h: this help message" <<endl;
    cout << "\t-v: print version info" <<endl;
    cout << "\t-f circuit_file: the circuit file (required)" <<endl;
    cout << "\t-d: turn on debug log level" <<endl;
    cout << "\t-i: enable interactive (gui) mode" <<endl;
    cout << "\t-s: step through algorithm" <<endl;
    cout << "\t-w b: incraese fixed cell weight bias by b% (higher = heavier, integer)" <<endl;
    cout << "\t-p:  [l]inear,[q]uadratic,[c]ubic cost increase function " <<endl;
}

void print_version() {
    spdlog::info("a2 - Troy Denton 2023");
    spdlog::info("Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
    spdlog::info("Commit {}", GIT_COMMIT);
    spdlog::info("Built {}" , __TIMESTAMP__);
}

void route_thread(circuit* circ, bool interactive) {
    bool result = circ->fit(interactive);
    if (result)
        spdlog::info("Successfully routed design");
    else
        spdlog::error("Could not route design");
}

int main(int n, char** args) {
    string file = "";
    string ps_file = "";

    bool interactive = false;
    bool step = false;
    int fixed_weight = 0;
    int spread_weight = 1.0;
    double (*psi_fn)(int, psi_params*) = psi_quadratic;

    for(;;)
    {
        switch(getopt(n, args, "vhf:disw:p:iz:"))
        {
            case 'z':
                spread_weight = stoi(optarg);
                continue;

            case 'p':
                switch(optarg[0])
                {
                    case 'l':
                        psi_fn = psi_linear;
                        break;
                    case 'q':
                        psi_fn = psi_quadratic;
                        break;
                    case 'c':
                        psi_fn = psi_cubic;
                        break;
                    default:
                        spdlog::error("Invalid psi function: specify l, q, c");
                        print_usage();
                        return 1;
                        break;
                }
                continue;

            case 'f':
                file = optarg;
                continue;

            case 'w':
                fixed_weight = stoi(optarg);
                continue;

            case 'd':
                spdlog::set_level(spdlog::level::debug);
                continue;

            case 'v':
                print_version();
                return 0;

            case 's':
                step = true;
                continue;

            case 'i':
                interactive = true;
                continue;
            case '?':
            case 'h':
            default :
                print_usage();
                return 1;

            case -1:
                break;
        }
        break;
    }

    print_version();

    if (file == "") {
        spdlog::error("Error: must provide input file");
        print_usage();
        return 1;
    }

    if (fixed_weight != 0) {
        spdlog::info("using fixed weight bias {}", fixed_weight);
    }


    //test_suitesparse();

    circuit* circ = new circuit(file);
    if (fixed_weight != 0) {
        circ->set_fixed_weight_bias(fixed_weight);
    }
    circ->iter();

    fabric* fab = new fabric(25,25);
    fab->mark_obstruction(2,2,9,9);
    fab->map_cells(circ->get_cells());

    //psi_params pps = {.a= 100., .b= 50., .c=25};
    psi_params pps = {.a= 0.1, .b= 0.1, .c=.1};
    flow_state fs = {.iter=0, .psi_fn = psi_fn, .h = pps, .step = step};

    fab->spread_weight=(double)spread_weight;

    if (!fs.step) {
        spdlog::debug("Running entire flow");
        while(!fab->run_flow_step(&fs));
    }

    if (interactive) {
        spdlog::info("Entering interactive mode");
        ui_init(circ, fab, &fs);
        ui_teardown();
    }

    spdlog::info("performing post flow spread");
    circ->iter(fab);
    if (interactive) {
        ui_init(circ, fab, &fs);
        ui_teardown();
    }

    spdlog::info("Exiting");
    delete(circ);
    delete(fab);
    return 0;
}
