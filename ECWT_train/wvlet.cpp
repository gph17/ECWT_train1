#include "wvlet.h"

using namespace Eigen;
using namespace std;


wvlet::wvlet(int n1, int cNo1, int wNo1):
	n(n1), cNo(cNo1), wCNo(wNo1)
{
	para.setZero(n + 1);
}

wvlet::wvlet(dataWin1 dW, int n1, int cNo1, int wNo1) : 
	n(n1), cNo(cNo1), wCNo(wNo1)//investigate delegation
{

	para.setZero(n + 1);
}


