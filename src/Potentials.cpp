#include "Potentials.hpp"

typedef Vector<2> Vec;
typedef Matrix<2,2> Mat;



double fastExp(double base, int exponent) { 
    double k = base;
	double result = 1;

    while(exponent != 0) {
        if (exponent % 2 == 1)
            result = (result * k);
        k = (k * k);

        exponent = exponent / 2;
    }

    return result;
};



double potential(Vec d, PotentialInfo& pot)
{
	double len = d.length();
	double result = 0;
	for (int i = 0; i < pot.potentialTerms; i++)
	{
		result -= pot.potentialCoefficients[i] / fastExp(len, pot.potentialExponents[i]);
	}
	return result;
}


Vec firstTerm(Vec d, PotentialInfo& pot)
{
	double len = d.length();
	double result = 0;
	for (int i = 0; i < pot.potentialTerms; i++)
	{
		result += pot.potentialExponents[i] * pot.potentialCoefficients[i] / fastExp(len, pot.potentialExponents[i] + 2);
	}
	return d * result;
}

Mat secondTerm(Vec d, PotentialInfo& pot) {
	double len = d.length();
	Mat i = identity<2>();
	Mat dd = d * d.transpose();
	double iFac = 0;
	double ddFac = 0;
	for (int i = 0; i < pot.potentialTerms; i++)
	{
		iFac += pot.potentialExponents[i] * pot.potentialCoefficients[i] / fastExp(len, pot.potentialExponents[i] + 2);
		ddFac -= (pot.potentialExponents[i] + 2) * (pot.potentialExponents[i]*pot.potentialCoefficients[i]) / fastExp(len, pot.potentialExponents[i] + 4);
	}
	return (i * iFac) + (dd * ddFac);
}


Vec F_Tilde(int& n, Vec& u, Vec d, PotentialInfo& pot) {
	Vec t1 = firstTerm(d, pot);
	Mat t2 = secondTerm(d, pot);
	Vec result = (t1 * n + t2 * u) * 2; // dont know why but factor 2 neccessary

	//std::cout << "F_tilde(" << d << ", " << u << ") = " << t1 << " * n  +" << t2 << " * " << u  << "=" << result << std::endl;
	return result;
}