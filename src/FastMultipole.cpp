#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <semaphore>

#include "FastMultipole.hpp"

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

	sim.potentialInfoLongRange = PotentialInfo{potentialTerms, new double[longRangeCount], new int[longRangeCount], 10.0};
	sim.potentialInfoShortRange = PotentialInfo{potentialTerms, new double[shortRangeCount], new int[shortRangeCount], 10.0};

	int s = 0, l = 0;
	for (int i = 0; i < potentialTerms; i++)
	{
		if (potentialExponents[i] < 2)
		{
			sim.potentialInfoLongRange.potentialCoefficients[l] = potentialCoefficients[i];
			sim.potentialInfoLongRange.potentialExponents[l] = potentialExponents[i];
			l++;
			std::cout << i << ": " << sim.potentialInfoLongRange.potentialCoefficients[l] << " * (1/r)^" << sim.potentialInfoLongRange.potentialExponents[l] << std::endl;
		}
		else
		{
			sim.potentialInfoShortRange.potentialCoefficients[s] = potentialCoefficients[i];
			sim.potentialInfoShortRange.potentialExponents[s] = potentialExponents[i];
			s++;
			std::cout << i << ": " << sim.potentialInfoShortRange.potentialCoefficients[s] << " * (1/r)^" << sim.potentialInfoShortRange.potentialExponents[s] << std::endl;
		}
	}
}

void setup(int n, double size, double dt)
{
	sim.N = n;
	sim.dt = dt;
	sim.bounds = size;

	double potentialCutOff = 10;
	int cellCount = int(round(2 * size / potentialCutOff));
	int cellCapacity = int(round(64  * sim.N/ cellCount));

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
			  << "N = " << sim.N << std::endl;
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

Stack<Tree *> s;
void calcOutgoingExpansions(Tree *tree, Vec *position)
{
	if (s.size < 4 * tree->objCounter)
		s.alloc(4 * tree->objCounter);

	initPostOrder(&s, tree);
	while (s.getHeight() > 0)
	{
		Tree *c = nextPostOrder<int>(&s);

		int n = 0;
		Vec u = zeros<2>();

		if (c->hasObj)
		{
			n = 1;
			u = c->center - position[c->obj];
		}
		else if (!c->leaf)
		{
			for (int i = 0; i < 4; i++)
			{
				Tree *d = c->children[i];
				n += d->info.n;
				u = u + d->info.u - ((d->center - c->center) * d->info.n);
			}
		}

		c->info.n = n;
		c->info.u = u;
	}

	s.clear();
};

typedef struct
{
	int index;
	Vec position;
} ReinsertionInfo;


Stack<ReinsertionInfo> reinsert;
void updateTree(Tree *tree, Vec *position)
{
	if (reinsert.size < tree->objCounter)
		reinsert.alloc(tree->objCounter);
	if (s.size < 4 * tree->objCounter)
		s.alloc(4 * tree->objCounter);

	initPostOrder<int>(&s, tree);
	while (s.getHeight() > 0)
	{
		Tree *c = nextPostOrder(&s);
		if (!c->hasObj)
			continue;

		int p = c->obj;
		if (c->inBounds(position[p]))
			c->objPosition = position[p];
		else
			reinsert.push(ReinsertionInfo{p, c->center});
	}

	for (int i = 0; i < reinsert.getHeight(); i++)
	{
		ReinsertionInfo r = reinsert.getList()[i];
		tree->remove(r.position);
	}
	for (int i = 0; i < reinsert.getHeight(); i++)
	{
		ReinsertionInfo r = reinsert.getList()[i];
		tree->insert(r.index, position[r.index]);
	}

	reinsert.clear();
};

Stack<int> reinsertGrid;
void updateGrid(Grid *grid, Vec *position)
{
	if (reinsertGrid.size < grid->objCounter)
		reinsertGrid.alloc(grid->objCounter);

	int count = grid->objCounter;

	grid->clear();

	for (int i = 0; i < count; i++)
	{
		grid->insert(i, position[i]);
	}
}

Vec FLongRange(int particle, Vec position, Tree *tree, PotentialInfo &pot)
{
	Vec delta = position - tree->center;
	double sqrLen = delta.sqrLength();

	if (tree->leaf)
	{
		if (!tree->hasObj || tree->obj == particle)
			return zeros<2>();
		else
			return firstTerm(position - tree->objPosition, pot); // exact solution given the exact position
	}
	else if (sqrLen / (tree->size * tree->size) > pot.cutOff * pot.cutOff)
		return F_Tilde(particle, position, delta, pot);
	else
	{
		Vec s = FLongRange(particle, position, tree->children[0], pot);
		s += FLongRange(particle, position, tree->children[1], pot);
		s += FLongRange(particle, position, tree->children[2], pot);
		s += FLongRange(particle, position, tree->children[3], pot);
		return s;
	}
};

Vec FShortRange(int particle, Vec position, Grid* grid, Vec* positions, PotentialInfo &pot)
{
	Vec result = zeros<2>();
	for (int s = 0; s < grid->cellNeighborCount; s++)
	{
		int idx = grid->neighboringCells(position)[s];
		if(idx < 0) continue;

		Stack<int>& stack = *grid->cell(idx);
		for (int i = 0; i < stack.getHeight(); i++)
		{
			int other = stack.getList()[i];
			Vec delta = (position - positions[other]);
			if (delta.sqrLength() != 0 && delta.sqrLength() < pot.cutOff * pot.cutOff)
			{
				result += firstTerm(delta, pot);
			}
		}
	}
	return result;
};

Vec FExact(int particle, Vec position, Tree *tree, PotentialInfo &pot)
{
	Vec delta = position - tree->objPosition;
	if (tree->leaf)
	{
		if (!tree->hasObj || tree->obj == particle)
			return zeros<2>();
		else
			return firstTerm(delta, pot); // exact solution given the exact position
	}
	else
	{
		Vec s = zeros<2>();
		for (int i = 0; i < 4; i++)
		{
			s = s + FExact(particle, position, tree->children[i], pot);
		}
		return s;
	}
};

void compute_bounces(Vec &position, Vec &velocity, double bounds, double dt)
{
	if (position[0] < -bounds)
	{
		position[0] = -bounds - (position[0] - (-bounds));
		velocity[0] = -velocity[0];
	}
	else if (position[0] > bounds)
	{
		position[0] = bounds - (position[0] - bounds);
		velocity[0] = -velocity[0];
	}

	if (position[1] < -bounds)
	{
		position[1] = -bounds - (position[1] - (-bounds));
		velocity[1] = -velocity[1];
	}
	else if (position[1] > bounds)
	{
		position[1] = bounds - (position[1] - bounds);
		velocity[1] = -velocity[1];
	}
};

std::chrono::time_point<std::chrono::high_resolution_clock> t1;
std::chrono::time_point<std::chrono::high_resolution_clock> t2;
std::chrono::time_point<std::chrono::high_resolution_clock> t3;
std::chrono::time_point<std::chrono::high_resolution_clock> t4;
std::chrono::time_point<std::chrono::high_resolution_clock> t5;
std::chrono::time_point<std::chrono::high_resolution_clock> t6;

void step()
{
	Buffer &cBuf = sim.bufferCurrent;

	t1 = std::chrono::high_resolution_clock::now();

	updateTree(cBuf.tree, cBuf.position);

	t2 = std::chrono::high_resolution_clock::now();

	calcOutgoingExpansions(cBuf.tree, cBuf.position);

	t3 = std::chrono::high_resolution_clock::now();

	updateGrid(cBuf.grid, cBuf.position);

	t4 = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < sim.N; i++)
	{
		cBuf.force[i] = zeros<2>();
		cBuf.force[i] += FLongRange(i, cBuf.position[i], sim.bufferCurrent.tree, sim.potentialInfoLongRange);

		cBuf.force[i] += FShortRange(i, cBuf.position[i], sim.bufferCurrent.grid, sim.bufferCurrent.position, sim.potentialInfoShortRange);
		cBuf.velocity[i] = cBuf.velocity[i] + cBuf.force[i] * (sim.dt / cBuf.mass[i]);
		//std::cout << "velocity of " << i << " is " << cBuf.velocity[i] << " = " << "oldVElocity + " << cBuf.force[i] << " * (" << sim.dt << " / " << cBuf.mass[i] << ")" << std::endl;
	}

	t5 = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < sim.N; i++)
	{
		Vec nextPos = cBuf.position[i] + cBuf.velocity[i] * sim.dt;
		Vec newVel = cBuf.velocity[i];
		compute_bounces(nextPos, cBuf.velocity[i], sim.bounds, sim.dt);
		cBuf.position[i] = nextPos;
		cBuf.velocity[i] = newVel;
	}

	t6 = std::chrono::high_resolution_clock::now();
};

std::ofstream timings;
std::binary_semaphore ThreadReady{0};
void loop()
{
	std::cout << "starting worker thread" << std::endl;
	ThreadReady.release();
	while (true)
	{
		SignalRequestUpdate.acquire();
		SignalDataReady.acquire();

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

		SignalRequestAccess.release();
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
		filename << "timings-" << sim.N << ".csv";
		timings.open(filename.str());
		timings << "tree,expansions,grid,forces,positions,total" << std::endl;
		ThreadReady.acquire();
		ThreadReady.release();
	}

	SignalRequestUpdate.release();
	SignalRequestAccess.acquire();
}

void access()
{
	SignalRequestAccess.acquire();
	SignalDataReady.acquire();
	SignalDataReady.release();
}

void release()
{
	SignalRequestAccess.release();
}
