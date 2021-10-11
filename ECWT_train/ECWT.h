#pragma once

#include<cmath>
#include <Eigen/Eigenvalues>
#include <Eigen/Dense>
#include<string>

#include "dataWin.h"
#include "PWvlet.h"

template<typename T>
class ECWT
{
	T wv1, wv2, wv3;
	bool iscanonical;
	int n;
	int cNo;
	int wCNo;
	int WLen;
	int start;
	double GoF;
	std::string source;
	void canonicise();
	double signature[3];

	/*make inner products*/
	double IP(const ECWT& ECWT1) const
	{
		return wv1.IP(ECWT1.wv1) + wv2.IP(ECWT1.wv2) + wv3.IP(ECWT1.wv3);
	}

	double IP(const dataWin& dW) const
	{
		return wv1.IP(dW.chan1) + wv2.IP(dW.chan2) + wv3.IP(dW.chan3);
	}

	double rotIP(ECWT&);
	double rotShIP(ECWT&);
	double rotIP(dataWin&);
	double rotShIP(dataWin&);

public:
	ECWT(int n1, int c, int w) : n(n1), cNo(c), wCNo(w), iscanonical(false),
		wv1(n1, c, w), wv2(n1, c, w), wv3(c, w, n1), WLen(0)
	{
	}

	ECWT(dataWin, int n1, int c, int w, int st = 0, const char* = 0);

	bool operator>(const ECWT& ECWT1) const
	{
		return GoF > ECWT1.GoF;
	}

	bool operator<(const ECWT& ECWT1) const
	{
		return GoF < ECWT1.GoF;
	}

	bool operator==(const ECWT& ECWT1) const
	{
		return GoF== ECWT1.GoF;
	}

	template <typename T>
	friend class Lib;
};

template<typename T>
void ECWT<T>::canonicise()
{
	Eigen::MatrixXd H = T::H;
	if (n + 1 < H.cols())
		H.conservativeResize(n + 1, n + 1);
	Eigen::Matrix3d K;
	Eigen::VectorXd vecs[] = { wv1.para, wv2.para, wv3.para };
	Eigen::VectorXd nVecs[] = { Eigen::VectorXd.setZero(n), Eigen::VectorXd.setZero(n), 
		Eigen::VectorXd.setZero(n)};
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
		for (j = 0; j < 3; j++)
			nVecs[i] += eigvec(i, j) * vecs[j];
	wv1.para = nVecs[0];
	wv2.para = nVecs[1];
	wv3.para = nVecs[2];
	iscanonical = true;
}

template<typename T>
ECWT<T>::ECWT(dataWin dW, int n1, int c, int w, int st, const char* sr) : n(n1), cNo(c), wCNo(w), iscanonical(false), 
wv1(dW.chan1, n1, c, w), wv2(dW.chan2, n1, c, w), wv3(dW.chan3, n1, c, w), WLen(dW.WLen), start(st)

{
	if (sr != 0)
		source = sr;
	double nrm = IP(*this);
	GoF = nrm - 2 * IP(dW);
	GoF /= dW.IP(dW);
	GoF += 1;
	GoF = GoF > 1.0 ? 1.0 : GoF;
	GoF = GoF < 0.0 ? 0.0 : GoF;
	nrm = std::sqrt(nrm);
	wv1.para /= nrm;
	wv2.para /= nrm;
	wv3.para /= nrm;
}

