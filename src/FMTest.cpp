#include <cfloat>
#include <cfenv>
#include <iostream>
#include "FastMultipole.hpp"




void test(int M)
{
	int n = M * M;
	int area = 5 * M;
	double dt = 0.03;

	Vec *p = new Vec[n];
	Vec *v = new Vec[n];
	Vec *f = new Vec[n];
	double *m = new double[n];

	for (int i = 0; i < n; i++)
	{
		double vec[]{((double)(0.9 * 2 * area) / M) * (double)(i / M - M / 2), ((double)(0.9 * 2 * area) / M) * (double)(i % M - M / 2)};
		// vec[0] += static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 0.5;
		// vec[1] += static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 0.5;

		p[i] = Vec(vec);
		v[i] = zeros<2>();
		f[i] = zeros<2>();
		m[i] = 1.0;
	}

	setup(n, area, dt);

	double *potCo = new double[]{-4, 20};
	int *potEx = new int[]{1, 2};
	setup_potentials(2, potCo, potEx);

	setup_values((double *)p, (double *)v, (double *)f, m);

	delete[] p;
	delete[] v;
	delete[] f;
	delete[] m;

	for (int i = 0; i < 1000; i++)
	{
		//std::cout << "Calculating frame " << i << std::endl;
		update();
	}
}

int main(int argc, char const *argv[])
{
	int x = std::stoi(argv[1]);
	std::cout << x << std::endl;
	
	if (x<=0)
		return 0;


	test(x);
	return 0;
}


/*
int main()
{
#ifdef DEBUG
	feenableexcept( FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
	// fesetmode(FE_DOWNWARD);
#endif

}
*/