#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <semaphore>

#include "Simulation.hpp"

SimulationInfo sim;

std::counting_semaphore SignalRequestUpdate{0};
std::counting_semaphore SignalRequestAccess{0};
std::counting_semaphore SignalDataReady{0};

void setup_potentials(int potentialTerms, double *potentialCoefficients, int *potentialExponents)
{
	int longRangeCount = 0;
	for (int i = 0; i < potentialTerms; i++)
	{
		if (potentialExponents[i] < 2)
			longRangeCount++;
	}
	int shortRangeCount = potentialTerms - longRangeCount;

	sim.potentialInfo = PotentialInfo{potentialTerms, new double[potentialTerms], new int[potentialTerms], 30.0};
	sim.potentialInfoLongRange = PotentialInfo{longRangeCount, new double[longRangeCount], new int[longRangeCount], 30.0};
	sim.potentialInfoShortRange = PotentialInfo{shortRangeCount, new double[shortRangeCount], new int[shortRangeCount], 10.0};

	int s = 0, l = 0;
	for (int i = 0; i < potentialTerms; i++)
	{
		sim.potentialInfo.potentialCoefficients[i] = potentialCoefficients[i];
		sim.potentialInfo.potentialExponents[i] = potentialExponents[i];

		// std::cout << i << ": " << potentialCoefficients[i] << " * (1/r)^" << potentialExponents[i] << std::endl;
		if (potentialExponents[i] < 2)
		{
			sim.potentialInfoLongRange.potentialCoefficients[l] = potentialCoefficients[i];
			sim.potentialInfoLongRange.potentialExponents[l] = potentialExponents[i];
			l++;
		}
		else
		{
			sim.potentialInfoShortRange.potentialCoefficients[s] = potentialCoefficients[i];
			sim.potentialInfoShortRange.potentialExponents[s] = potentialExponents[i];
			s++;
		}
	}
}

void setup(int n, double size, double dt, bool exact)
{
	sim.N = n;
	sim.dt = dt;
	sim.bounds = size;
	sim.exact = exact;

	double potentialCutOff = 10;
	int cellCount = int(round(2 * size / potentialCutOff));
	int cellCapacity = int(round(64 * sim.N / cellCount));

	sim.bufferCurrent.position = new Vec[sim.N];
	sim.bufferCurrent.velocity = new Vec[sim.N];
	sim.bufferCurrent.force = new Vec[sim.N];
	sim.bufferCurrent.mass = new double[sim.N];
	sim.bufferCurrent.grid = new Grid(Vec{0.0, 0.0}, 2 * size, cellCount, cellCapacity, 3);
	sim.bufferCurrent.tree = new Tree(zeros<2>(), sim.bounds, 0, nullptr);

	sim.bufferNext.position = new Vec[sim.N];
	sim.bufferNext.velocity = new Vec[sim.N];
	sim.bufferNext.force = new Vec[sim.N];
	sim.bufferNext.mass = sim.bufferCurrent.mass;
	sim.bufferNext.grid = new Grid(Vec{0.0, 0.0}, 2 * size, cellCount, cellCapacity, 3);
	sim.bufferNext.tree = new Tree(zeros<2>(), sim.bounds, 0, nullptr);

	std::cout << "size = " << sim.bounds << std::endl
			  << "dt = " << sim.dt << std::endl
			  << "N = " << sim.N << std::endl
			  << "exact = " << sim.exact << std::endl;
}

void setup_values(double *pos, double *vel, double *force, double *mass)
{
	Vec *p = (Vec *)pos;
	Vec *v = (Vec *)vel;
	Vec *f = (Vec *)force;
	double *m = mass;

	if (sim.bufferCurrent.tree->objCounter > 0)
		sim.bufferCurrent.tree->clear();
	if (sim.bufferCurrent.grid->objCounter > 0)
		sim.bufferCurrent.grid->clear();

	for (int i = 0; i < sim.N; i++)
	{
		sim.bufferCurrent.position[i] = p[i];
		sim.bufferCurrent.velocity[i] = v[i];
		sim.bufferCurrent.force[i] = f[i];
		sim.bufferCurrent.mass[i] = m[i];

		std::cout << "set up values for " << i
				  << " position: " << sim.bufferCurrent.position[i] << ","
				  << " velocity: " << sim.bufferCurrent.velocity[i] << ","
				  << " force: " << sim.bufferCurrent.force[i] << ","
				  << " mass: " << sim.bufferCurrent.mass[i] << std::endl;
	}

	for (int i = 0; i < sim.N; i++)
	{
		sim.bufferCurrent.tree->insert(i, sim.bufferCurrent.position[i]);
		sim.bufferCurrent.grid->insert(i, sim.bufferCurrent.position[i]);
		sim.bufferNext.tree->insert(i, sim.bufferCurrent.position[i]);
		sim.bufferNext.grid->insert(i, sim.bufferCurrent.position[i]);
	}

	std::cout << sim.bufferCurrent.tree << std::endl;
	std::cout << sim.bufferCurrent.grid << std::endl;

	SignalRequestAccess.release();
	SignalDataReady.release();
}

double *position() { return (double *)sim.bufferCurrent.position; }
double *velocity() { return (double *)sim.bufferCurrent.velocity; }
double *force() { return (double *)sim.bufferCurrent.force; }
double *mass() { return sim.bufferCurrent.mass; }

std::chrono::time_point<std::chrono::high_resolution_clock> t1;
std::chrono::time_point<std::chrono::high_resolution_clock> t2;
std::chrono::time_point<std::chrono::high_resolution_clock> t3;
std::chrono::time_point<std::chrono::high_resolution_clock> t4;
std::chrono::time_point<std::chrono::high_resolution_clock> t5;
std::chrono::time_point<std::chrono::high_resolution_clock> t6;

void step()
{
	Buffer &cBuf = sim.bufferCurrent;
	Buffer &nBuf = sim.bufferNext;

	t1 = std::chrono::high_resolution_clock::now();

	updateTree(nBuf.tree, cBuf.position);

	t2 = std::chrono::high_resolution_clock::now();

	calcOutgoingExpansions(nBuf.tree, cBuf.position);

	t3 = std::chrono::high_resolution_clock::now();

	updateGrid(nBuf.grid, cBuf.position);

	t4 = std::chrono::high_resolution_clock::now();

	if (sim.exact)
	{
		for (int i = 0; i < sim.N; i++)
		{
			nBuf.force[i] = zeros<2>();
			nBuf.force[i] += FLongRange(i, cBuf.position[i], nBuf.tree, sim.potentialInfoLongRange);
			nBuf.force[i] += FShortRange(i, cBuf.position[i], nBuf.grid, sim.bufferCurrent.position, sim.potentialInfoShortRange);

			nBuf.velocity[i] = cBuf.velocity[i] + nBuf.force[i] * (sim.dt / nBuf.mass[i]);
		}
	}
	else
	{
		for (int i = 0; i < sim.N; i++)
		{
			nBuf.force[i] = FExact(i, cBuf.position[i], nBuf.tree, sim.potentialInfo);
			nBuf.velocity[i] = cBuf.velocity[i] + nBuf.force[i] * (sim.dt / nBuf.mass[i]);
		}
	}
	t5 = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < sim.N; i++)
	{
		Vec nextPos = cBuf.position[i] + nBuf.velocity[i] * sim.dt;
		Vec newVel = nBuf.velocity[i];
		compute_bounces(nextPos, newVel, sim.bounds, sim.dt);
		nBuf.position[i] = nextPos;
		nBuf.velocity[i] = newVel;
	}

	t6 = std::chrono::high_resolution_clock::now();

	SignalRequestAccess.acquire();

	// swap buffers
	Buffer temp = sim.bufferCurrent;
	sim.bufferCurrent = sim.bufferNext;
	sim.bufferNext = temp;

	SignalRequestAccess.release();
};

std::ofstream timings;
bool finishSimulation = false;
void loop()
{
	while (!finishSimulation)
	{
		SignalRequestUpdate.acquire();

		step();

		auto tree = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		auto expansions = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
		auto grid = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
		auto forces = std::chrono::duration_cast<std::chrono::nanoseconds>(t5 - t4).count();
		auto positions = std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count();
		auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t1).count();

		timings << tree
				<< "," << expansions
				<< "," << grid
				<< "," << forces
				<< "," << positions
				<< "," << total
				<< std::endl;

		SignalDataReady.release();
	}
}

std::thread *worker = nullptr;
void update()
{
	if (worker == nullptr)
	{
		worker = new std::thread(loop);

		std::stringstream filename;
		filename << "timings-" << sim.exact << "-" << sim.N << ".csv";
		timings.open(filename.str());
		timings << "tree,expansions,grid,forces,positions,total" << std::endl;
	}

	SignalDataReady.acquire();
	SignalRequestUpdate.release();
}

void finish()
{
	finishSimulation = true;
	SignalRequestUpdate.release();
	SignalDataReady.acquire();
	SignalRequestAccess.acquire();

	worker->join();
}

void access()
{
	SignalRequestAccess.acquire();
}

void release()
{
	SignalRequestAccess.release();
}
