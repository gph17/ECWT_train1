#include <algorithm>
#include <cmath>
#include <Eigen/Core>
#include <iostream>
#include <unsupported/Eigen/Polynomials>

#include "PWvlet.h"
#include "util.h"

using namespace Eigen;
using namespace std;

/*initialise static members*/
map<complexKey, MatrixXd> PWvlet::G;
map<complexKey, MatrixXd> PWvlet::P;
MatrixXd PWvlet::H;
Matrix<long long, Dynamic, Dynamic> PWvlet::W;
map<int, Matrix<long long, Dynamic, Dynamic>> PWvlet::Hi;

map<int, MatrixXd> PWvlet::fTrans = map<int, MatrixXd>();

const set<complexKey> PWvlet::valid = { {4, 0, 0},
	{5, 0, 0}, {5, 0, 1},
	{6, 0, 0}, {6, 0, 1}, {6, 0, 2}, {6, 1, 1},
	{7, 0, 0}, {7, 0, 1}, {7, 0, 2}, {7, 0, 3}, {7, 1, 1}, {7, 1, 2},
	{8, 0, 0}, {8, 0, 1}, {8, 0, 2}, {8, 0, 3}, {8, 1, 1}, {8, 1, 2}, {8, 1, 3}, {8, 2, 2},
	{9, 0, 0}, {9, 0, 1}, {9, 0, 2}, {9, 0, 3}, {9, 1, 1}, {9, 1, 2}, {9, 1, 3}, {9, 2, 2},
	{10, 0, 0}, {10, 0, 1}, {10, 0, 2}, {10, 0, 3}, {10, 1, 1}, {10, 1, 2}, {10, 1, 3},
	{11, 0, 0}, {11, 0, 1}, {11, 0, 2}, {11, 1, 1}, {11, 1, 2},
	{12, 0, 0}, {12, 0, 1}, {12, 0, 2}, {12, 1, 1}, {12, 1, 2},
	{13, 0, 0}, {13, 0, 1}, {13, 0, 2}, {13, 1, 1}, {13, 1, 2},
	{14, 0, 0}, {14, 0, 1}, {14, 0, 2}, {14, 1, 1}, {14, 1, 2},
	{15, 0, 0}, {15, 0, 1}, {15, 0, 2}, {15, 1, 1}, {15, 1, 2},
	{16, 0, 0}, {16, 0, 1}, {16, 0, 2}, {16, 1, 1}, {16, 1, 2} , {16, 1, 3} };

PWvlet::PWvlet(int n1, int cNo1, int wCNo1) :
	wvlet(n1, cNo1, wCNo1)
{
	if (n1 - wCNo - cNo - 1 < 3)
	{
		cerr << "Fewer than 3 degrees of freedom after conditions\n";
		exit(-1);
	}
	complexKey cK = { n, cNo, wCNo };
	if (!valid.count(cK))
	{
		cerr << "Numerical problems with this combination of parameters\n";
		exit(-1);
	}
	makeH(n);
	makeW(n, wCNo);
	makeP(n, cNo, wCNo);
}

PWvlet::PWvlet(dataWin1 dW, int n1, int cNo1, int wCNo1) : 
	wvlet(dW, n1, cNo1, wCNo1)
{
	if (n1 - wCNo - cNo - 1 < 3)
	{
		cerr << "Fewer than 3 degrees of freedom after conditions\n";
		exit(-1);
	}
	complexKey cK = { n, cNo, wCNo };
	if (!valid.count(cK))
	{
		cerr << "Numerical problems with this combination of parameters\n";
		exit(-1);
	}
	makeH(n);
	makeW(n, wCNo);
	makeP(n, cNo, wCNo);
	int WLen = dW.getLen();
	maintainFTrans(n, WLen);
	MatrixXd Hi1 = Hi[n].cast<double>();
	MatrixXd K = fTrans[WLen];
	K.conservativeResize((UINT64)n + 1ULL, WLen);
	para = P[cK] * (Hi1 * (K * dW.Vec));
}

void PWvlet::populate(const dataWin1& dW)
{
	if (n - wCNo - cNo - 1 < 3)
	{
		cerr << "Fewer than 3 degrees of freedom after conditions\n";
		exit(-1);
	}
	complexKey cK = { n, cNo, wCNo };
	if (!valid.count(cK))
	{
		cerr << "Numerical problems with this combination of parameters\n";
		exit(-1);
	}
	makeH(n);
	makeW(n, wCNo);
	makeP(n, cNo, wCNo);
	int WLen= dW.getLen();
	maintainFTrans(n, WLen);
	MatrixXd K = fTrans[dW.getLen()];
	K.conservativeResize(n + 1ULL, WLen);
	para = P[cK] * (Hi[n].cast<double>() * (K * dW.Vec));
}


/*make static members*/
void PWvlet::makeH(int n1)
{
	if (n1 > 16)
	{
		cerr << "Maximum size of n before long long wraps in calculation of H^{-1} is 16\n";
		exit(-1);
	}
	long long i, j;
	long long Hn = H.cols();
	if (Hn>= n1 + 1LL)
		return;
	if (Hn == 0)
	{
		H.resize(n1 + 1LL, n1 + 1LL);
		for (i = 0; i <= n1; i++)
			for (j = 0; j<= n1; j++)
				H(i, j) = 1.0/ (1.0 + i + j);
	}
	else
	{
		H.conservativeResize(n1 + 1LL, n1 + 1LL);
		for (i = 0; i <= n1; i++)
			for (j = (i < Hn)? Hn: 0; j <= n1; j++)
				H(i, j) = 1.0 / (1.0 + i + j);
	}
	Matrix<long long, Dynamic, Dynamic> Hitmp(n1 + 1, n1 + 1);
	Hitmp.setZero();
	Hitmp(0, 1) = -(n1 * (n1 + 1) * (n1 + 1) * (n1 + 2)) / 2;
	Hitmp(1, 0) = Hitmp(0, 1);
	long long nfac, dfac, g;
	for (i = 0; i < n1; i++)
	{
		if (i > 0)
		{
			nfac = (n1 + 1LL + i) * (n1 + 2LL + i) * ((long long)n1 - i) * (n1 + 1LL - i);
			dfac = i * (i + 1LL) * (i + 1LL) * (i + 1LL);
			g = gcd(nfac, dfac);
			nfac /= g;
			dfac /= g;
			Hitmp(i, i + 1LL) = nfac * (Hitmp(i - 1LL, i) / dfac);
			Hitmp(i + 1LL, i) = Hitmp(i, i + 1LL);
		}
		for (j = i + 2; j <= n1; j++)
		{
			nfac = (i + j) * (n1 + 1LL - j) * (n1 + 1LL + j);
			dfac = j * j * (i + j + 1LL);
			g = gcd(nfac, dfac);
			nfac /= g;
			dfac /= g;
			Hitmp(i, j) = -nfac * (Hitmp(i, j - 1LL)/dfac);
			Hitmp(j, i) = Hitmp(i, j);
		}
	}
	Hitmp(0, 0) = (n1 + 1LL) * (n1 + 1LL);
	for (i = 1; i < n1 + 1LL; i++)
	{
		nfac = (2LL * i - 1LL) * (n1 + 1LL - i) * (n1 + 1LL + i) * (n1 + 1LL - i) * (n1 + 1LL + i);
		dfac = (2LL * i + 1LL) * i * i * i * i;
		g = gcd(nfac, dfac);
		nfac /= g;
		dfac /= g;
		Hitmp(i, i) = nfac * (Hitmp(i - 1LL, i - 1LL) /dfac);
	}
	Hi[n1] = Hitmp;
}

void PWvlet::makeW(int n1, int w1)
{
	long long nW = W.cols(), wRows = W.rows();
	if ((nW > n1) && (wRows>= w1))
		return;
	long long i, j;
	if (nW == 0)
	{
		W.setZero(w1, n1 + 1ULL);
		for (i = 0; i < w1; i++)
		{
			if (i == 0)
				W(0, 1) = 1;
			else
				W(i, i + 1LL) = (i + 1LL) * W(i - 1LL, i);
			for (j = i + 2; j < n1 + 1LL; j++)
				W(i, j) = j * W(i, j - 1LL)/(j - i);
		}
		return;
	}
	if (nW < n1 + 1LL)
	{
		//extend rows of W
		W.conservativeResize(wRows, n1 + 1ULL);
		W.block(0, nW, wRows, n1 + 1LL - nW) = Matrix<long long, Dynamic, Dynamic>::Zero(wRows, (n1 - nW) + 1ULL);
		for (i = 0; i < wRows; i++)
			for (j = nW; j < n1 + 1LL; j++)
				W(i, j) = j * W(i, j - 1LL) / (j - i);
	}
	if (wRows < w1)
	{
		W.conservativeResize(w1, n1 + 1ULL);
		W.block(wRows, 0, w1 - wRows, n1 + 1) = 
			Matrix<long long, Dynamic, Dynamic>::Zero(w1 - wRows, n1 + 1ULL);
		for (i = wRows; i < w1; i++)
		{
			W(i, i) = 1;
			for (j = i + 1; j < n1 + 1LL; j++)
				W(i, j) = j * W(i, j - 1ULL) / (j - i);
		}
	}
}

void PWvlet::makeG(int n1, int c1, int w1)
{
	makeH(n1);
	makeW(n1, w1);
	complexKey cK = { n1, c1, w1 };
	if (G.count(cK))
		return;

	MatrixXd Hinv = Hi[n1].cast<double>();

	if ((c1 == 0) && (w1 == 0))
	{
		MatrixXd tmp = Hinv;
		tmp(0, 0) -= 1.0;
		G[cK] = tmp;
		return;
	}

	int i;
	Matrix<double, Dynamic, Dynamic> B =
		Matrix<double, Dynamic, Dynamic>::Zero((UINT64)c1 + (UINT64)w1 + 1ULL, n1 + 1ULL);
	long long j0 = Hi[n1](0, 0);
	Matrix<double, Dynamic, Dynamic> HiB = Matrix<double, Dynamic, Dynamic>::Zero(n1 + 1LL, (1LL + w1) + c1);
	Matrix<double, Dynamic, Dynamic> BHiBi = Matrix<double, Dynamic, Dynamic>::Zero((1LL + w1) + c1, (1LL + w1) + c1);
	if (c1 == 0)
	{
		Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1);
		Matrix<long long, Dynamic, Dynamic> J = (Hi[n1]).block(1, 1, n1, n1);
		Vector<long long, Dynamic> j = (Hi[n1]).block(1, 0, n1, 1);
		Wtilde.block(0, 0, w1, n1) = W.block(0, 1, w1, n1);
		for (i = 0; i < w1; i++)
		{
			Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
			remfac(tmp);
			Wtilde.row(i) = tmp.transpose();
		}
		Matrix<long long, Dynamic, Dynamic> HiB_LL = Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, w1 + 1LL);
		HiB_LL(0, 0) = 1;
		HiB_LL.block(1, 1, n1, w1) = (Wtilde * J).transpose();
		HiB_LL.block(0, 1, 1, w1) = (Wtilde * j).transpose();
		Matrix<long long, Dynamic, Dynamic> tmp = Wtilde * J;
		BHiBi(0, 0) = 1.0;
		Matrix<double, Dynamic, Dynamic> Jtilde = (Wtilde * J * (Wtilde.transpose())).cast<double>();
		BHiBi.block(1, 1, w1, w1) = Jtilde.inverse();
		HiB = HiB_LL.cast<double>();
	}
	else
		if (c1 == 1)
		{
			Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1);
			Matrix<long long, Dynamic, Dynamic> J = (Hi[n1]).block(1, 1, n1, n1);
			Vector<long long, Dynamic> j = (Hi[n1]).block(1, 0, n1, 1);
			Wtilde.block(0, 0, w1, n1) = W.block(0, 1, w1, n1);
			for (i = 0; i < w1; i++)
			{
				Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
				remfac(tmp);
				Wtilde.row(i) = tmp.transpose();
			}
			Matrix<long long, Dynamic, Dynamic> HiB_LL = Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, w1 + 2LL);
			HiB_LL(0, 0) = 1;
			HiB_LL(0, 1) = j0;
			HiB_LL.block(1, 1, n1, 1) = j;
			HiB_LL.block(0, 2, 1, c1 + w1 - 1) = (Wtilde * j).transpose();
			HiB_LL.block(1, 2, n1, c1 + w1 - 1) = (Wtilde * J).transpose();
			Matrix<double, Dynamic, Dynamic> K = (Wtilde * J * Wtilde.transpose()).cast<double>().inverse();
			Vector<double, Dynamic> k = (Wtilde * j).cast<double>();
			double k0 = j0 - (k.transpose()) * (K * k);
			k = K * k;
			BHiBi(0, 0) = k0;
			BHiBi(0, 1) = -1.0;
			BHiBi(1, 0) = -1.0;
			BHiBi(1, 1) = 1;
			BHiBi.block(0, 2, 1, w1) = k.transpose();
			BHiBi.block(2, 0, w1, 1) = k;
			BHiBi.block(1, 2, 1, w1) = -k.transpose();
			BHiBi.block(2, 1, w1, 1) = -k;
			BHiBi.block(2, 2, w1, w1) = k * k.transpose();
			BHiBi /= k0 - 1;
			BHiBi.block(2, 2, w1, w1) += K;

			HiB = HiB_LL.cast<double>();
		}
		else
		{
			Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1 + 1LL - c1);
			Vector<long long, Dynamic> j1 = (Hi[n1]).block(1, 0, c1 - 1, 1),
				j2 = (Hi[n1]).block(c1, 0, n1 + 1 - c1, 1);
			Matrix<long long, Dynamic, Dynamic> J11 = (Hi[n1]).block(1, 1, c1 - 1, c1 - 1),
				J21 = (Hi[n1]).block(c1, 1, n1 + 1 - c1, c1 - 1),
				J22 = (Hi[n1]).block(c1, c1, n1 + 1 - c1, n1 + 1 - c1);
			Wtilde.block(0, 0, w1, n1 + 1 - c1) = W.block(0, c1, w1, n1 + 1 - c1);
			for (i = 0; i < w1; i++)
			{
				Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
				remfac(tmp);
				Wtilde.row(i) = tmp.transpose();
			}
			Matrix<long long, Dynamic, Dynamic> HiB_LL =
				Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, (c1 + 1LL) + w1);

			HiB_LL(0, 0) = 1;
			HiB_LL(0, 1) = j0;
			HiB_LL.block(0, 2, 1, c1 - 1) = j1.transpose();
			HiB_LL.block(0, c1 + 1LL, 1, w1) = (Wtilde * j2).transpose();

			HiB_LL.block(1, 1, c1 - 1, 1) = j1;
			HiB_LL.block(1, 2, c1 - 1LL, c1 - 1LL) = J11;
			HiB_LL.block(1, c1 + 1LL, c1 - 1LL, w1) = (Wtilde * J21).transpose();

			HiB_LL.block(c1, 1, n1 - c1 + 1, 1) = j2;
			HiB_LL.block(c1, 2, n1 - c1 + 1, c1 - 1) = J21;
			HiB_LL.block(c1, c1 + 1, n1 + 1 - c1, w1) = J22 * Wtilde.transpose();

			Matrix<long long, Dynamic, Dynamic> J_LL(w1 + c1 - 1, w1 + c1 - 1);
			Vector<long long, Dynamic> j_LL(w1 + (c1 - 1LL));
			J_LL.block(0, 0, c1 - 1LL, c1 - 1LL) = J11;
			J_LL.block(c1 - 1LL, 0, w1, c1 - 1LL) = Wtilde * J21;
			J_LL.block(0, c1 - 1LL, c1 - 1LL, w1) = J_LL.block(c1 - 1LL, 0, w1, c1 - 1).transpose();
			J_LL.block(c1 - 1LL, c1 - 1LL, w1, w1) = (Wtilde * J22) * Wtilde.transpose();
			j_LL.head(c1 - 1LL) = j1;
			j_LL.tail(w1) = Wtilde * j2;
			Matrix<double, Dynamic, Dynamic> K = J_LL.cast<double>().inverse();
			Vector<double, Dynamic> j = j_LL.cast<double>();
			Vector<double, Dynamic> k = K * j;
			double k0 = j0 - j.transpose() * k;
			BHiBi(0, 0) = k0;
			BHiBi(0, 1) = -1.0;
			BHiBi(1, 0) = -1.0;
			BHiBi(1, 1) = 1;
			BHiBi.block(0, 2, 1, w1 + c1 - 1) = k.transpose();
			BHiBi.block(2, 0, w1 + c1 - 1, 1) = k;
			BHiBi.block(1, 2, 1, w1 + c1 - 1) = -k.transpose();
			BHiBi.block(2, 1, w1 + c1 - 1, 1) = -k;
			BHiBi.block(2, 2, w1 + c1 - 1, w1 + c1 - 1) = k * k.transpose();
			BHiBi /= k0 - 1;
			BHiBi.block(2, 2, w1 + c1 - 1, w1 + c1 - 1) += K;

			HiB = HiB_LL.cast<double>();
		}
	MatrixXd G0 = Hinv - HiB * (BHiBi * (HiB.transpose())), G1 = G0.transpose();
	G[cK] = (G0 + G1) / 2;
}

void PWvlet::makeP(int n1, int c1, int w1)
{
	makeH(n1);
	makeW(n1, w1);
	complexKey cK = { n1, c1, w1 };
	if (P.count(cK))
		return;

	int i;
	if ((c1 == 0) && (w1 == 0))
	{
		MatrixXd tmp = MatrixXd::Identity(n1 + 1, n1 + 1);
		for (i = 0; i < n1 + 1; i++)
			tmp(0, i) -= H(0, i);
		P[cK] = tmp;
		return;
	}

	MatrixXd Hinv = Hi[n1].cast<double>();

	Matrix<double, Dynamic, Dynamic> B =
		Matrix<double, Dynamic, Dynamic>::Zero((UINT64)c1 + (UINT64)w1 + 1ULL, n1 + 1ULL);
	for (i = 0; i < n1 + 1; i++)
		B(0, i) = 1.0 / (i + 1.0);
	long long j0 = Hi[n1](0, 0);
	Matrix<double, Dynamic, Dynamic> HiB = Matrix<double, Dynamic, Dynamic>::Zero(n1 + 1LL, (1LL + w1) + c1);
	Matrix<double, Dynamic, Dynamic> BHiBi = Matrix<double, Dynamic, Dynamic>::Zero((1LL + w1) + c1, (1LL + w1) + c1);
	if (c1 == 0)
	{
		Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1);
		Matrix<long long, Dynamic, Dynamic> J = (Hi[n1]).block(1, 1, n1, n1);
		Vector<long long, Dynamic> j = (Hi[n1]).block(1, 0, n1, 1);
		Wtilde.block(0, 0, w1, n1) = W.block(0, 1, w1, n1);
		for (i = 0; i < w1; i++)
		{
			Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
			remfac(tmp);
			Wtilde.row(i) = tmp.transpose();
		}
		B.block(1, 1, w1, n1) = Wtilde.cast<double>();
		Matrix<long long, Dynamic, Dynamic> HiB_LL = Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, w1 + 1LL);
		HiB_LL(0, 0) = 1;
		HiB_LL.block(1, 1, n1, w1) = (Wtilde * J).transpose();
		HiB_LL.block(0, 1, 1, w1) = (Wtilde * j).transpose();
		Matrix<long long, Dynamic, Dynamic> tmp = Wtilde * J;
		BHiBi(0, 0) = 1.0;
		Matrix<double, Dynamic, Dynamic> Jtilde = (Wtilde * J * (Wtilde.transpose())).cast<double>();
		BHiBi.block(1, 1, w1, w1) = Jtilde.inverse();
		HiB = HiB_LL.cast<double>();
	}
	else
		if (c1 == 1)
		{
			Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1);
			Matrix<long long, Dynamic, Dynamic> J = (Hi[n1]).block(1, 1, n1, n1);
			Vector<long long, Dynamic> j = (Hi[n1]).block(1, 0, n1, 1);
			Wtilde.block(0, 0, w1, n1) = W.block(0, 1, w1, n1);
			for (i = 0; i < w1; i++)
			{
				Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
				remfac(tmp);
				Wtilde.row(i) = tmp.transpose();
			}
			B(1, 0) = 1;
			B.block(2, 1, w1, n1) = Wtilde.cast<double>();
			Matrix<long long, Dynamic, Dynamic> HiB_LL = Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, w1 + 2LL);
			HiB_LL(0, 0) = 1;
			HiB_LL(0, 1) = j0;
			HiB_LL.block(1, 1, n1, 1) = j;
			HiB_LL.block(0, 2, 1, c1 + w1 - 1) = (Wtilde * j).transpose();
			HiB_LL.block(1, 2, n1, c1 + w1 - 1) = (Wtilde * J).transpose();
			Matrix<double, Dynamic, Dynamic> K = (Wtilde * J * Wtilde.transpose()).cast<double>().inverse();
			Vector<double, Dynamic> k = (Wtilde * j).cast<double>();
			double k0 = j0 - (k.transpose()) * (K * k);
			k = K * k;
			BHiBi(0, 0) = k0;
			BHiBi(0, 1) = -1.0;
			BHiBi(1, 0) = -1.0;
			BHiBi(1, 1) = 1;
			BHiBi.block(0, 2, 1, w1) = k.transpose();
			BHiBi.block(2, 0, w1, 1) = k;
			BHiBi.block(1, 2, 1, w1) = -k.transpose();
			BHiBi.block(2, 1, w1, 1) = -k;
			BHiBi.block(2, 2, w1, w1) = k * k.transpose();
			BHiBi /= k0 - 1;
			BHiBi.block(2, 2, w1, w1) += K;

			HiB = HiB_LL.cast<double>();
		}
		else
		{
			Matrix<long long, Dynamic, Dynamic> Wtilde = Matrix<long long, Dynamic, Dynamic>::Zero(w1, n1 + 1LL - c1);
			Vector<long long, Dynamic> j1 = (Hi[n1]).block(1, 0, c1 - 1, 1),
				j2 = (Hi[n1]).block(c1, 0, n1 + 1 - c1, 1);
			Matrix<long long, Dynamic, Dynamic> J11 = (Hi[n1]).block(1, 1, c1 - 1, c1 - 1),
				J21 = (Hi[n1]).block(c1, 1, n1 + 1 - c1, c1 - 1),
				J22 = (Hi[n1]).block(c1, c1, n1 + 1 - c1, n1 + 1 - c1);
			Wtilde.block(0, 0, w1, n1 + 1 - c1) = W.block(0, c1, w1, n1 + 1 - c1);
			for (i = 0; i < w1; i++)
			{
				Vector<long long, Dynamic> tmp = Wtilde.row(i).transpose();
				remfac(tmp);
				Wtilde.row(i) = tmp.transpose();
			}
			B(1, 0) = 1;
			B.block(2, 1, c1 - 1, c1 - 1) = MatrixXd::Identity(c1 - 1, c1 - 1);
			B.block(c1 + 1, c1, w1, n1 + 1 - c1) = Wtilde.cast<double>();
			Matrix<long long, Dynamic, Dynamic> HiB_LL =
				Matrix<long long, Dynamic, Dynamic>::Zero(n1 + 1LL, (c1 + 1LL) + w1);

			HiB_LL(0, 0) = 1;
			HiB_LL(0, 1) = j0;
			HiB_LL.block(0, 2, 1, c1 - 1) = j1.transpose();
			HiB_LL.block(0, c1 + 1LL, 1, w1) = (Wtilde * j2).transpose();

			HiB_LL.block(1, 1, c1 - 1, 1) = j1;
			HiB_LL.block(1, 2, c1 - 1LL, c1 - 1LL) = J11;
			HiB_LL.block(1, c1 + 1LL, c1 - 1LL, w1) = (Wtilde * J21).transpose();

			HiB_LL.block(c1, 1, n1 - c1 + 1, 1) = j2;
			HiB_LL.block(c1, 2, n1 - c1 + 1, c1 - 1) = J21;
			HiB_LL.block(c1, c1 + 1, n1 + 1 - c1, w1) = J22 * Wtilde.transpose();

			Matrix<long long, Dynamic, Dynamic> J_LL(w1 + c1 - 1, w1 + c1 - 1);
			Vector<long long, Dynamic> j_LL(w1 + (c1 - 1LL));
			J_LL.block(0, 0, c1 - 1LL, c1 - 1LL) = J11;
			J_LL.block(c1 - 1LL, 0, w1, c1 - 1LL) = Wtilde * J21;
			J_LL.block(0, c1 - 1LL, c1 - 1LL, w1) = J_LL.block(c1 - 1LL, 0, w1, c1 - 1).transpose();
			J_LL.block(c1 - 1LL, c1 - 1LL, w1, w1) = (Wtilde * J22) * Wtilde.transpose();
			j_LL.head(c1 - 1LL) = j1;
			j_LL.tail(w1) = Wtilde * j2;
			Matrix<double, Dynamic, Dynamic> K = J_LL.cast<double>().inverse();
			Vector<double, Dynamic> j = j_LL.cast<double>();
			Vector<double, Dynamic> k = K * j;
			double k0 = j0 - j.transpose() * k;
			BHiBi(0, 0) = k0;
			BHiBi(0, 1) = -1.0;
			BHiBi(1, 0) = -1.0;
			BHiBi(1, 1) = 1;
			BHiBi.block(0, 2, 1, w1 + c1 - 1) = k.transpose();
			BHiBi.block(2, 0, w1 + c1 - 1, 1) = k;
			BHiBi.block(1, 2, 1, w1 + c1 - 1) = -k.transpose();
			BHiBi.block(2, 1, w1 + c1 - 1, 1) = -k;
			BHiBi.block(2, 2, w1 + c1 - 1, w1 + c1 - 1) = k * k.transpose();
			BHiBi /= k0 - 1;
			BHiBi.block(2, 2, w1 + c1 - 1, w1 + c1 - 1) += K;

			HiB = HiB_LL.cast<double>();
		}
		P[cK] = MatrixXd::Identity(n1 + 1, n1 + 1) - HiB * (BHiBi * B);
}

void PWvlet::maintainFTrans(int n1, int N)
{
	if (N < 3)
	{
		cerr << "Too few points in data window\n";
		exit(3);
	}
	int nr, nc;
	MatrixXd F2tildeF;
	if (fTrans.count(N))
	{
		F2tildeF = fTrans.at(N);
		nr = (int)F2tildeF.rows();
		if (nr == (n1 + 1))
			return;
		nc = N;
	}
	else
		nr = nc = 0;
	F2tildeF.conservativeResize(n1 + 1ULL, N);
	int i, j, k;
	double Delta = 1.0 / (N - 1.0);
	if (nc == 0)
		nr = 0;
	double* terms = new double[n1 + 1LL];
	for (i = nr; i <= n1; i++)
	{
		if (i)
		{
			F2tildeF(i, 0) = pow(0.5 * Delta, i + 1) / (1.0 + i);
			for (j = 1; j < N - 1; j++)
			{
				terms[0] = pow(Delta, 1.0 + i) / (1.0 + i);
				for (k = 1; k <= i; k++)
					terms[k] = ((i + 2LL - k) * (j - 0.5) * terms[k - 1]) /k;
				F2tildeF(i, j) = KahanSum(terms, i + 1);
			}
			terms[0] = Delta / 2;
			for (k = 1; k <= i; k++)
				terms[k] = -(i + 1 - k) * Delta * terms[k - 1] / 2 / (k + 1.0);
			int i1 = i / 2, i2 = i - i1 - 1;
			double* terms1 = new double[i1 + 1], *terms2 = new double[i2 + 1];
			for (k = 0; k <= i1; k++)
				terms1[k] = terms[2 * k];
			for (k = 0; k <= i2; k++)
				terms2[k] = terms[2 * k + 1];
			F2tildeF(i, N - 1ULL) = KahanSum(terms1, i1 + 1) + KahanSum(terms2, i2 + 1);
			delete[] terms1, terms2;
		}
		else
		{
			F2tildeF(i, 0) = Delta / 2;
			for (j = 1; j < N - 1; j++)
				F2tildeF(i, j) = Delta;
			F2tildeF(i, N - 1) = Delta / 2;
		}
	}
	delete[] terms;
	fTrans[N] = F2tildeF;
}

/*provide override for drawable function*/
VectorXf PWvlet::points(int N)	//convert to values to be plotted against time
{
	VectorXf ret(N);
	int i;
	for (i = 0; i < N; i++)
		ret(i) = poly_eval(para, i/(N - 1.f));
	return ret;
}


/*make inner products*/
double PWvlet::IP(const dataWin1& dW) const	//L2 IP
{
	int N = dW.WLen;
	maintainFTrans(n, N);
	MatrixXd K = fTrans[N];
	K.conservativeResize((UINT64)n + 1ULL, dW.getLen());
	return dW.Vec.transpose() * (K.transpose() * para);
}

set<int> PWvlet::getValid()
{
	set<int> ret;
	for (auto item : valid)
		ret.insert(item.n);
	return ret;
}

set<pair<int, int>> PWvlet::getValid(int deg)
{
	set<pair<int, int>> ret;
	pair<int, int> val;
	for (auto item : valid)
		if (item.n== deg)
		{
			val.first = item.c;
			val.second = item.w;
			ret.insert(val);
		}
	return ret;
}

