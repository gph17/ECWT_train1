#pragma once

#include <Eigen/Dense>
#include <map>
#include <windows.h>

long long comb(int, int);

void remfac(Eigen::Vector<long long, Eigen::Dynamic>&);

long long gcd(long long, long long);

bool csvCheck(PWSTR);

enum regStat { NEEDPLAIN, NEEDSPECIAL, DONE };

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

