#include "psis.h"

// uses param h->a as slope
double psi_linear(int iter, psi_params* h) {
    double t = (double)iter;
    return t*h->a;
}

// uses param h->b as x*x coeff h->a as x coeff
double psi_quadratic(int iter, psi_params* h) {
    double t = (double)iter;
    return h->a*t*t;
}

// uses param h->c as x^3 coeff 
//            h->b as x^2 coeff 
//            h->a as x coeff
double psi_cubic(int iter, psi_params* h) {
    double t = (double)iter;
    return h->a*t*t*t;
}
