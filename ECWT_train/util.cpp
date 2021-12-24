#include "DBMem.h"

#include <cstdlib>
#include <cctype>
#include <ctype.h>
#include <fstream>
#include <limits>
#include <iostream>

#include "util.h"

using namespace Eigen;
using namespace std;

long long comb(int k, int j)
{
	if ((j > k) || (j < 0))
	{
		cerr << "Invalid arguments\n";
		exit(-1);
	}
	if ((j == 0) || (j == k))
		return 1LL;
	if (j > k - j)
		return comb(k, k - j);
	return comb(k - 1, j - 1) + comb(k - 1, j);
}

void remfac(Vector<long long, Dynamic>& v)
{
	long long GDiv = 0;
	int i;
	for (i = 0; i < v.size(); i++)
		if (GDiv == 0)
			GDiv = v(i);
		else
			if (v(i) != 0)
				GDiv = gcd(GDiv, v(i));
	if ((GDiv == 0) || (GDiv == 1))
		return;
	v /= GDiv;
}

long long gcd(long long m, long long n)
{
	if (m == 0)
		return n;
	if (n == 0)
		return m;
	long long r = m % n;
	if (r == 0)
		return n;
	return gcd(n, r);
}

bool csvCheck(PWSTR fName)	//checks if fName is of a (non-trivial) csv file
{
	ifstream ifs;
	ifs.open(fName);
	if (!ifs.good())
		return false;
	char ch;
	ifs >> ch;
	if (!ifs.good())
		return false;
	if (isdigit(ch) || (ch == '.'))
		ifs.putback(ch);
	else
		ifs.ignore((numeric_limits<streamsize>::max)(), '\n');	//throw away header
	if (!ifs.good())
		return false;
	double first;
	ifs >> first;
	ch = ifs.get();
	while (isspace(ch) && (ch != '\n'))
		ch = ifs.get();
	if ((ch != '\n') && (ch != ','))
		return false;
	return true;
}

VectorXd xvals(int N)
{
	VectorXd ret(N + 1);
	double Delta = 1.0 / (N - 1.0);
	ret(0) = 0;
	for (int i = 1; i < N; i++)
		ret(i) = (i - 0.5) * Delta;
	ret(N) = 1.0;
	return ret;
}

double KahanSum(double* terms, int Len)
{
	if (Len == 1)
		return terms[0];
	double sum = 0.0, c = 0.0;

	for (int i = 0; i < Len; i++)
	{
		double y = terms[i] - c;
		volatile double t = sum + y;
		volatile double z = t - sum;
		c = z - y;
		sum = t;
	}

	return sum;
}