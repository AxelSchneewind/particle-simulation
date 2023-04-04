#pragma once

#include "FastMultipole.hpp"

void setup_potentials(int potentialTerms, double *potentialCoefficients, int *potentialExponents);
void setup(int n, double size, double dt, bool exact);
void setup_values(double *pos, double *vel, double *force, double *mass);

double *position();
double *velocity();
double *force();
double *mass();


void step();

void loop();

void update();
void finish();

void access();
void release();