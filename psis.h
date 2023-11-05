#ifndef __PSIS_H__
#define __PSIS_H__

struct psi_params {
    double a,b,c,d;
};

double psi_linear(int inter, psi_params* h);
double psi_quadratic(int inter, psi_params* h);
double psi_cubic(int inter, psi_params* h);

#endif
