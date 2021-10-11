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
		{
			if (v(i) != 0)
				GDiv = v(i);
		}
		else
			if (v(i) != 0)
				GDiv = gcd(GDiv, v(i));
	if (GDiv == 0)
		return;
	for (i = 0; i < v.size(); i++)
		v(i) /= GDiv;
}

long long gcd(long long m, long long n)
{
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
