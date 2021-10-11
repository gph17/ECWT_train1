#pragma once

#include <Eigen/Dense>
#include <map>
#include <windows.h>

long long comb(int, int);

void remfac(Eigen::Vector<long long, Eigen::Dynamic>&);

long long gcd(long long, long long);

bool csvCheck(PWSTR);

enum regStat { NEEDPLAIN, NEEDSPECIAL, DONE };

struct compKey
{
	int n, c, w;

	friend bool operator<(const compKey& compKey1, const compKey& compKey2)
	{
		return (compKey1.n == compKey2.n) ?
			((compKey1.c == compKey2.c) ? (compKey1.w < compKey2.w) : (compKey1.c < compKey2.c)) :
			(compKey1.n < compKey2.n);
	}
};


