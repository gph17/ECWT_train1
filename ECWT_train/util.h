#pragma once

#include <cmath>
#include <Eigen/Dense>
#include <map>
#include <windows.h>

static double gr = (1 + std::sqrt(5.0))/2;

long long comb(int, int);

void remfac(Eigen::Vector<long long, Eigen::Dynamic>&);

long long gcd(long long, long long);

bool csvCheck(PWSTR);

double KahanSum(double* terms, int Len);

enum class regStat { NEEDPLAIN, NEEDSPECIAL, DONE };

struct complexKey
{
	int n, c, w;

	friend bool operator<(const complexKey& complexKey1, const complexKey& complexKey2)
	{
		return (complexKey1.n == complexKey2.n) ?
			((complexKey1.c == complexKey2.c) ? (complexKey1.w < complexKey2.w) : (complexKey1.c < complexKey2.c)) :
			(complexKey1.n < complexKey2.n);
	}
};

Eigen::VectorXd xvals(int);

void grSearch(double (*fn)(double), double, double, double, double&, double&);
void grSearch(double (*fn)(double), double, double, double, double, double, double, double, double&, double&);
