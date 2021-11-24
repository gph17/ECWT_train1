#pragma once

#include <Eigen/Dense>

#include "dataWin1.h"

#define WVLTDRAW (WM_USER + 6)

class wvlet//derive PWvlet initially, TWvlet later
{
protected:
	int n;	//one less than the number of parameters, reduces to degree of polynomial wavelet
	int cNo;	//number of continuity conditions
	int wCNo;	//number of wrapped continuity conditions (first cNo of which are implied by continuity conditions)
	Eigen::VectorXd para;	//parameters

	/*make inner products*/
	virtual double IP(const wvlet&) const = 0;	//L2 IP - analogue in ECWT too
	virtual double IP(const dataWin1&) const = 0;	//L2 IP - analogue in ECWT too

public:
	wvlet(int n1 = 0, int cNo1 = 0, int wNo1 = 0);
	wvlet(dataWin1, int n1, int cNo1 = 0, int wNo1 = 0);

	virtual void populate(const dataWin1&) = 0;


	int getn() const
	{
		return n;
	}
	int getcNo()
	{
		return cNo;
	}
	int getwCNo()
	{
		return wCNo;
	}

	friend class PWvlet;
};
