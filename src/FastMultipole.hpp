#pragma once

#include "Potentials.hpp"
#include "Vector.hpp"
#include "QuadTree.hpp"
#include "Stack.hpp"
#include "UniformGrid.hpp"


typedef Vector<2> Vec;
typedef Matrix<2,2> Mat;

typedef struct{int n; Vec u; } MultipoleInfo;
typedef QuadTree<int, MultipoleInfo> Tree;
typedef UniformGrid<int> Grid;

typedef struct { 
    double* buffer; 
    Vec* position; 
    Vec* velocity; 
    Vec* force; 
    double* mass;
    Grid* grid; 
	Tree* tree;
} Buffer;


class SimulationInfo {
public:
	int N;
	double dt;
	double bounds;

    Buffer bufferCurrent;
    Buffer bufferNext;

    PotentialInfo potentialInfoLongRange;
    PotentialInfo potentialInfoShortRange;
};


void setup_potentials(int potentialTerms, double* potentialCoefficients, int* potentialExponents);

void setup_values(double* pos, double* vel, double* force, double* mass);

void setup(int n, double size, double dt) ;

double* position();
double* velocity();
double* force();
double* mass();




void calcOutgoingExpansions(Tree *tree, Vec *position);

void updateTree(Tree *tree, Vec *position);

void updateGrid(Grid *grid, Vec *position);

Vec FLongRange(int particle, Vec position, Tree *tree, PotentialInfo &pot);

Vec FShortRange(int particle, Vec position, Tree *tree, PotentialInfo &pot);

Vec FExact(int particle, Vec position, Tree *tree, PotentialInfo &pot);

void compute_bounces(Vec &position, Vec &velocity, double bounds, double dt);

void step();

void loop();

void update();