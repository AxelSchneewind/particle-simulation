#pragma once

#include "Vector.hpp"

typedef Vector<2> Vec;
typedef Matrix<2,2> Mat;



// for long range potentials: cutOff is ratio of distance to cell size from where approximation via expansion is used
// for short range potentials: cutOff is distance from where potential is ignored
typedef struct { int potentialTerms; double* potentialCoefficients; int* potentialExponents; double cutOff; } PotentialInfo;

double potential(Vec d, PotentialInfo& pot);

Vec firstTerm(Vec d, PotentialInfo& pot);

Mat secondTerm(Vec d, PotentialInfo& pot);

Vec F_Tilde(int& n, Vec& u, Vec d, PotentialInfo& pot);