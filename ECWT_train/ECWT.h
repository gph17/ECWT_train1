#pragma once

#include <algorithm>
#include<cmath>
#include<compare>
#include <Eigen/Eigenvalues>
#include <Eigen/Dense>
#include <iostream>
#include<string>

#include "dataWin.h"
#include "IPFunctor.h"
#include "PWvlet.h"
#include "util.h"
#include "wvlet.h"

template<typename T>
class ECWT
{
	T wv[3];
	bool iscanonical;
	int n;
	int cNo;
	int wCNo;
	int WLen;
	int start;
	double GoF;
	std::wstring source;
	void canonicise();
	double signature[3];

	/*make inner products*/
	double IP(const ECWT& ECWT1) const
	{
		return wv[0].IP(ECWT1.wv[0]) + wv[1].IP(ECWT1.wv[1]) + wv[2].IP(ECWT1.wv[2]);
	}

	double IP(const dataWin& dW) const
	{
		return wv[0].IP(dW.chan[0]) + wv[1].IP(dW.chan[1]) + wv[2].IP(dW.chan[2]);
	}

	double rotIP(const ECWT&) const;
	double rotShIP(const ECWT&) const;
	double rotIP(const dataWin&) const;
	double rotShIP(const dataWin&) const;

public:
	ECWT(int n1, int c, int w) : n(n1), cNo(c), wCNo(w), iscanonical(false), WLen(0)
	{
		for (int i = 0; i < 3; i++)
		{
			wv[i].n = n;
			wv[i].cNo = cNo;
			wv[i].wCNo = wCNo;
			wv[i].para.SetZero(n + 1);
		}
	}

	ECWT(dataWin, int n1, int c, int w, int st = 0, const wchar_t* = 0, double GoFLim = 0);

	ECWT() : GoF(0.0), WLen(0), cNo(0), wCNo(0), iscanonical(false), n(0), start(0), signature{ 0.0, 0.0, 0.0}
	{
	}

	void draw(HWND, int = 0);

	auto operator<=>(const ECWT& ECWT1) const
	{
		return GoF <=> ECWT1.GoF;
	}

	template <typename T>
	friend class Lib;

	template <typename T>
	friend class IPFunctor;
};

template<typename T>
double ECWT<T>::rotIP(const ECWT<T>& ECWT1) const
{
	Eigen::Matrix3d K;
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			K(i, j) = wv[i].IP(ECWT1.wv[j]);
	Eigen::JacobiSVD<Eigen::Matrix3d> svd(K);
	return svd.singularValues().sum();
}

template<typename T>
double ECWT<T>::rotIP(const dataWin& dW) const
{
	Eigen::Matrix3d K;
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			K(i, j) = wv[i].IP(dW.chan[j]);
	Eigen::JacobiSVD<Eigen::Matrix3d> svd(K);
	return svd.singularValues().sum();
}

template<typename T>
double ECWT<T>::rotShIP(const dataWin& dW) const
{
	double ret = rotIP(dW);
	dataWin dW1 = dW;
	for (int i = 0; i < dW.WLen - 1; i++)
	{
		dW1.cycle();
		ret = max(ret, rotIP(dW1));
	}
	return ret;
}

template<typename T>
double ECWT<T>::rotShIP(const ECWT& ECWT1) const
{
	//Look at functors - store (2n + 1) 3x3 matrices as a state representing a matrix-valued polynomial in t, map 
	//	from input t to matrix at t, sum eigenvalues. This sum to be maximised
	IPFunctor IPF(*this, ECWT1);
	double t1 = 0, t2 = 0.01, x, val, bVal;
	grSearch(IPF, t1, t2, 1e-10, x, bVal);
	while (t1 < 0.995)
	{
		t1 += 0.01;
		t2 += 0.01;
		grSearch(IPF, t1, t2, 1e-10, x, val);
		if (val < bVal)
			bVal = val;
	}
	return -bVal;
}

template<typename T>
void ECWT<T>::canonicise()
{
	Eigen::MatrixXd H = T::H;
	if (n + 1 < H.cols())
		H.conservativeResize(n + 1, n + 1);
	Eigen::Matrix3d K;
	Eigen::VectorXd vecs[] = { wv[0].para, wv[1].para, wv[2].para };
	Eigen::VectorXd nVecs[] = { Eigen::VectorXd.setZero(n + 1), Eigen::VectorXd.setZero(n + 1), 
		Eigen::VectorXd.setZero(n + 1)};
	int i, j;
	for (i = 0; i < 3; i++)
	{
		K(i, i) = vecs(i).transpose() * (H * vecs(i));
		for (j = i + 1; j < 3; j++)
			K(i, j) = K(j, i) = vecs(i).transpose() * (H * vecs(j));
	}
	Eigen::EigenSolver<Eigen::MatrixXd> es(K);
	Eigen::Vector3d eigval = es.Eigen::eigenvalues().real();
	Eigen::Matrix3d eigvec = es.Eigen::eigvectors();
	Eigen::Array3i idx;
	if ((eigval(0) >= eigval(1)) && (eigval(0) >= eigval(2)))
	{
		idx(0) = 0;
		if (eigval(1) >= eigval(2))
		{
			idx(1) = 1;
			idx[2] = 2;
		}
		else
		{
			idx(1) = 2;
			idx(2) = 1;
		}
	}
	else
		if ((eigval(1) >= eigval(0)) && (eigval(1) >= eigval(2)))
		{
			idx(0) = 1;
			if (eigval(0) >= eigval(2))
			{
				idx(1) = 0;
				idx(2) = 2;
			}
			else
			{
				idx(1) = 2;
				idx(2) = 0;
			}
		}
		else
		{
			idx(0) = 2;
			if (eigval(0) > eigval(1))
			{
				idx(1) = 0;
				idx(2) = 1;
			}
			else
			{
				idx[2] = 0;
				idx(1) = 1;
			}
		}
	signature = eigval(idx);
	eigvec = eigvec(Eigen::all, idx);
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
			nVecs[i] += eigvec(i, j) * vecs[j];
		wv[i].para = nVecs[i];
	}
	iscanonical = true;
}

template<typename T>
ECWT<T>::ECWT(dataWin dW, int n1, int c, int w, int st, const wchar_t* sr, double GoFLim) : 
	n(n1), cNo(c), wCNo(w), iscanonical(false), WLen(dW.WLen), start(st), GoF(0)
{
	double dNrm = dW.IP(dW) - dW.muMag2();
	if (dNrm <= 1e-6)
		return;	//GoF == 0 indicates that wavelets from near flat data should be ignored
	complexKey cK = { n, cNo, wCNo };
	if (!PWvlet::valid.count(cK))
	{
		std::cerr << "Numerical problems with this combination of parameters\n";
		exit(-1);
	}
	int i;
	int N = dW.chan[0].getLen();
	PWvlet::makeP(n1, c, w);
	PWvlet::maintainFTrans(n1, N);
	Eigen::MatrixXd K = PWvlet::fTrans[N];
	K.conservativeResize((UINT64)n + 1ULL, N);

	GoF = dW.GoFFun(K, PWvlet::Hi[n].cast<double>(), PWvlet::P[cK]);
	GoF = GoF > 1.0 ? 1.0 : GoF;
	GoF = GoF < GoFLim ? 0.0 : GoF;	//rounding out of range corrected
	if (GoF == 0)
		return;	//discard ECWT
	/* GoF is the square of the cosine of the angle between fitted wavelet and data - big is good */
	for (i = 0; i < 3; i++)
	{
		wv[i].n = n;
		wv[i].cNo = cNo;
		wv[i].wCNo = wCNo;
		wv[i].populate(dW.chan[i]);
	}
	if (sr != 0)
		source = sr;
	double nrm = IP(*this);
	nrm = std::sqrt(nrm);
	for (int i = 0; i < 3; i++)
		wv[i].para /= nrm;
}

template<typename T>
void ECWT<T>::draw(HWND hwnd, int N)
{
	if (N == 0)
		N = 100;//get width of graph later
	//find co-ordinate value vectors of wvlets in triplet
	int i;
	Eigen::VectorXf curves[3];
	for (i = 0; i < 3; i++)
		curves[i] = wv[i].points(N);
	//find max abs co-ord value over vectors of triplet
	float ySc = std::max<float>(curves[0].lpNorm<Eigen::Infinity>(),
		std::max<float>(curves[1].lpNorm<Eigen::Infinity>(), curves[2].lpNorm<Eigen::Infinity>()));
	//send message updating yscale of window
	SendMessage(hwnd, ADJ_YRSCL, NULL, reinterpret_cast<LPARAM>(&ySc));
	//send messages to draw value vectors
	SendMessage(hwnd, WVLTDRAW, NULL, reinterpret_cast<LPARAM>(&curves));
}