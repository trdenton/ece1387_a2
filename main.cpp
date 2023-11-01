#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <getopt.h>
#include "spdlog/spdlog.h"
#include "version.h"
#include "easygl/graphics.h"
#include "ui.h"
#include "umfpack.h"
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
    int force_w = 0;

    for(;;)
    {
        switch(getopt(n, args, "vhf:dis"))
        {
            case 'f':
                file = optarg;
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

    if (file == "") {
        spdlog::error("Error: must provide input file");
        print_usage();
        return 1;
    }

    print_version();

    //test_suitesparse();

    circuit* circ = new circuit(file);
    circ->iter();
    #if 0
    thread t1(route_thread, circ, step);

    if (!interactive)
        t1.join();

    if (interactive) {
        spdlog::info("Entering interactive mode");
        ui_init(circ);
        ui_teardown();
        t1.join();
    }
    #endif

    spdlog::info("Exiting");
    delete(circ);
    return 0;
}
