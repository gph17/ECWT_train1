#pragma once

#include <Eigen/Dense>
#include <vector>

#include "ECWT.h"

template<typename T>
class IPFunctor
{
	std::vector<Eigen::Matrix3d> K;
public:
	IPFunctor(ECWT<T>, ECWT<T>);
	double operator()(double) const;
};

template<typename T>
IPFunctor<T>::IPFunctor(ECWT<T> ECWT1, ECWT<T> ECWT2):K(2 * ECWT1.n + 1)
{
	int n = ECWT1.n, N = 2 * n + 1;
		for (int k = 0; k < 3; k++)
			for (int ell = 0; ell < 3; ell++)
			{
				K[0](k, ell) = ECWT1.wv[k].IP(ECWT2.wv[ell]);
				for (int s = 1; s < N; s++)
					K[s] = ECWT1.wv[k].para.transpose() * T::H[s] * ECWT2.wv[ell].para;
			}
}

template<typename T>
double IPFunctor<T>::operator()(double t) const
{
	int N = K.size();
	Eigen::Matrix3d K1 = K[N - 1];
	for (int s = N - 2; s >= 0; s--)
		K1 = K[s] + t * K1;
	Eigen::JacobiSVD<Eigen::Matrix3d> svd(K1);
	return -svd.singularValues().sum();
}