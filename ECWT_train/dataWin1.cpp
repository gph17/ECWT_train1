#include "dataWin1.h"

using namespace Eigen;

dataWin1::dataWin1(int SR1, int WLen1): SR(SR1), WLen(WLen1)
{
	Vec.setZero(WLen);
}

double dataWin1::IP(const dataWin1& dW) const
{
	if (WLen != dW.WLen)
	{
		std::cerr << "IP between windows of different lengths not yet implemented\n";
		exit(-1);
	}
	double ret = (Vec(0) * dW.Vec(0) + Vec(WLen - 1) * dW.Vec(WLen - 1))/2;
	double Delt = 1.0 / (WLen - 1.0);
	for (int i = 1; i < WLen - 1; i++)
		ret += Vec(i) * dW.Vec(i);
	return ret * Delt;

};
