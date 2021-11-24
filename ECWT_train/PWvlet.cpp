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
MatrixXd PWvlet::H;
Matrix<long long, Dynamic, Dynamic> PWvlet::W;
map<int, Matrix<long long, Dynamic, Dynamic>> PWvlet::Hi;

map<int, MatrixXd> PWvlet::fTrans = map<int, MatrixXd>();

const set<complexKey> PWvlet::valid = { {4, 0, 0}, {4, 0, 1},
	{5, 0, 0}, {5, 0, 1}, {5, 0, 2}, {5, 1, 1},
	{6, 0, 0}, {6, 0, 1}, {6, 0, 2}, {6, 0, 3}, {6, 1, 1}, {6, 1, 2},
	{7, 0, 0}, {7, 0, 1}, {7, 0, 2}, {7, 0, 3}, {7, 1, 1}, {7, 1, 2}, {7, 1, 3}, {7, 2, 2},
	{8, 0, 0}, {8, 0, 1}, {8, 0, 2}, {8, 0, 3}, {8, 1, 1}, {8, 1, 2}, {8, 1, 3}, {8, 2, 2},
	{9, 0, 0}, {9, 0, 1}, {9, 0, 2}, {9, 0, 3}, {9, 1, 1}, {9, 1, 2}, {9, 1, 3}, {9, 2, 2},
	{10, 0, 0}, {10, 0, 1}, {10, 0, 2}, {10, 0, 3}, {10, 1, 1}, {10, 1, 2}, {10, 1, 3},
	{11, 0, 0}, {11, 0, 1}, {11, 0, 2}, {11, 1, 1}, {11, 1, 2},
	{12, 0, 0}, {12, 0, 1}, {12, 0, 2}, {12, 1, 1}, {12, 1, 2},
	{13, 0, 0}, {13, 0, 1}, {13, 0, 2}, {13, 1, 1}, {13, 1, 2},
	{14, 0, 0}, {14, 0, 1}, {14, 0, 2}, {14, 1, 1}, {14, 1, 2},
	{15, 0, 0}, {15, 0, 1}, {15, 0, 2}, {15, 1, 1}, {15, 1, 2},
	{16, 0, 0}, {16, 0, 1}, {16, 0, 2}, {16, 1, 1}, {16, 1, 2}, {16, 1, 3} }; 

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
	makeG(n, cNo, wCNo);
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
	makeG(n, cNo, wCNo);
	maintainFTrans(n, dW.getLen());
	MatrixXd G1 = G[cK];
	MatrixXd K = fTrans[dW.getLen()];
	K.conservativeResize((UINT64)n + 1ULL, dW.getLen());
	para = G1 * (K * dW.Vec);
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
	makeG(n, cNo, wCNo);
	maintainFTrans(n, dW.getLen());
	MatrixXd G1 = G[cK];
	MatrixXd K = fTrans[dW.getLen()];
	K.conservativeResize(n + 1ULL, dW.getLen());
	para = G1 * (K * dW.Vec);
}


/*make static members*/
void PWvlet::makeH(int n1)
{
	if (n1 > 16)
	{
		cerr << "Maximum size of n before long long wraps in calculation of H^{-1} is 16\n";
		exit(-1);
	}
	int i, j;
	int Hn = (int)(H.cols());
	if (Hn>= n1 + 1)
		return;
	if (Hn == 0)
	{
		H.resize(n1 + 1ULL, n1 + 1ULL);
		for (i = 0; i <= n1; i++)
			for (j = 0; j<= n1; j++)
				H(i, j) = 1.0/ (1.0 + i + j);
	}
	else
	{
		H.conservativeResize(n1 + 1ULL, n1 + 1ULL);
		for (i = 0; i <= n1; i++)
			for (j = (i < Hn)? Hn: 0; j <= n1; j++)
				H(i, j) = 1.0 / (1.0 + i + j);
	}
	Matrix<long long, Dynamic, Dynamic> Hitmp(n1 + 1, n1 + 1);
	Hitmp.setZero();
	for (i = 0; i < n1 + 1; i++)
		for (j = i + 1; j < n1 + 1; j++)
		{
			Hitmp(i, j) = (((UINT64)i + j) + 1ULL) *
				comb(n1 + i + 1, n1 - j) * comb(n1 + j + 1, n1 - i) * comb(i + j, i) * comb(i + j, i);
			if ((i + j)!= 2 * ((i + j)/2))
				Hitmp(i, j)*= -1LL;
			Hitmp(j, i) = Hitmp(i, j);
		}
	for (i = 0; i < n1 + 1; i++)
		Hitmp(i, i) = (2LL * i + 1LL) * 
			comb(n1 + i + 1, n1 - i) * comb(n1 + i + 1, n1 - i) * comb(2 * i, i) * comb(2 * i, i);
	Hi[n1] = Hitmp;
}

void PWvlet::makeW(int n1, int w1)
{
	int nW = (int)(W.cols());
	int wRows = (int)(W.rows());
	if ((nW > n1) && (wRows>= w1))
		return;
	int i, j;
	if (nW == 0)
	{
		W.setZero(w1, n1 + 1ULL);
		for (i = 0; i < w1; i++)
		{
			W(i, i) = 1LL;
			for (j = i + 1; j < n1 + 1; j++)
				W(i, j) = j * W(i, j - 1ULL)/((long long)j - i);
		}
		return;
	}
	if (nW < n1 + 1)
	{
		//extend rows of W
		W.conservativeResize(wRows, n1 + 1ULL);
		W.block(0, nW, wRows, n1 + 1 - nW) = Matrix<long long, Dynamic, Dynamic>::Zero(wRows, ((UINT64)n1 - nW) + 1ULL);
		for (i = 0; i < wRows; i++)
			for (j = nW; j < n1 + 1; j++)
				W(i, j) = j * W(i, (UINT64)j - 1) / ((long long)j - i);
	}
	if (wRows < w1)
	{
		W.conservativeResize(w1, n1 + 1ULL);
		W.block(wRows, 0, w1 - wRows, n1 + 1) = 
			Matrix<long long, Dynamic, Dynamic>::Zero((UINT64)w1 - wRows, n1 + 1ULL);
		for (i = wRows; i < w1; i++)
		{
			W(i, i) = 1;
			for (j = i + 1; j < n1 + 1; j++)
				W(i, j) = j * W(i, j - 1ULL) / ((long long)j - i);
		}
	}
}

void PWvlet::makeG(int n1, int c1, int w1)
{
	complexKey cK = {n1, c1, w1};
	if (G.count(cK))
		return;
	int k;
	Matrix<double, Dynamic, Dynamic> B =
		Matrix<double, Dynamic, Dynamic>::Zero((UINT64)c1 + (UINT64)w1 + 1ULL, n1 + 1ULL);
	MatrixXd Hinv = Hi[n1].cast<double>();
	if ((c1!= 0) || (w1!= 0))
	{
		Matrix<long long, Dynamic, Dynamic> B0 = 
			Matrix<long long, Dynamic, Dynamic>::Zero((UINT64)c1 + (UINT64)w1, n1 + 1ULL);

		B0.block(0, 0, c1, c1) = Matrix<long long, Dynamic, Dynamic>::Identity(c1, c1);
		B0.block(c1, 0, w1, n1 + 1) = W.block(0, 0, w1, n1 + 1);
		B0.block(c1, 0, w1, c1) = Matrix<long long, Dynamic, Dynamic>::Zero(w1, c1);
		for (k = c1; k < c1 + w1; k++)
		{
			Vector<long long, Dynamic> tmpv = B0.row(k);
			remfac(tmpv);
			B0.block(k, 0, 1, n1 + 1) = tmpv.transpose();
		}

		B.block(1, 0, c1 + w1, n1 + 1) = B0.cast<double>();
		for (k = 0; k < n1 + 1; k++)
			B(0, k) = 1.0 / (1.0 + k);

		MatrixXd B0HiB0 = (B0 * (Hi[n1] * B0.transpose())).cast<double>();
		MatrixXd B0HiB0i = B0HiB0.inverse();
		B0HiB0i = (B0HiB0i + B0HiB0i.transpose()) / 2;	//enforce symmetry
		MatrixXd BHiBi(c1 + w1 + 1, c1 + w1 + 1);
		BHiBi(0, 0) = 1 / (1 - B0HiB0i(0, 0));
		BHiBi.block(1, 0, c1 + w1, 1) = B0HiB0i.block(0, 0, c1 + w1, 1) / (1 - B0HiB0i(0, 0));
		BHiBi.block(0, 1, 1, c1 + w1) = B0HiB0i.block(0, 0, 1, c1 + w1) / (1 - B0HiB0i(0, 0));
		BHiBi.block(1, 1, c1 + w1, c1 + w1) = B0HiB0i;
		VectorXd vec = B0HiB0i.col(0);
		BHiBi.block(1, 1, c1 + w1, c1 + w1) = BHiBi.block(1, 1, c1 + w1, c1 + w1) +
			(vec * vec.transpose()) / (1 - B0HiB0i(0, 0));

		G[cK] = Hinv * (MatrixXd::Identity(n1 + 1ULL, n1 + 1ULL) - B.transpose() * (BHiBi * (B * Hinv)));
	}
	else
	{
		MatrixXd tmp = Hinv;
		tmp(0, 0)-= 1.0;
		G[cK] = tmp;
	}
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
	int i, j;
	double Delta = 1.0 / (N - 1.0);
	if (nc == 0)
		nr = 0;
	for (i = nr; i <= n1; i++)
	{
		F2tildeF(i, 0) = pow(0.5 * Delta, i + 1) / (1.0 + i);
		for (j = 1; j < N - 1; j++)
			F2tildeF(i, j) = (pow((j - 0.5) * Delta, i + 1) -
				pow((j - 1.5) * Delta, i + 1)) / (1.0 + i);
		F2tildeF(i, N - 1ULL) = (1 - pow(1 - 0.5 * Delta, i + 1)) / (1.0 + i);
	}
	fTrans[N] = F2tildeF;
}

/*provide override for drawable function*/
VectorXd PWvlet::points(int N)	//convert to values to be plotted against time
{
	VectorXd ret(N);
	int i;
	for (i = 0; i < N; i++)
		ret(i) = poly_eval(para, i/(N - 1.0));
	return ret;
}


/*make inner products*/
double PWvlet::IP(const dataWin1& dW) const	//L2 IP
{
	MatrixXd K = fTrans[dW.WLen];
	K.conservativeResize(n + 1ULL, dW.WLen);
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

