#pragma once

#include <Eigen/Dense>
#include <iostream>

#include "drawable.h"

#define DATAWDRAW (WM_USER + 5)

class dataWin1: public drawable
{
	int SR;//per second
	int WLen;
	Eigen::VectorXd Vec;
	UINT msg = DATAWDRAW;

public:
	dataWin1(int = 0, int = 0);

	int getLen() const
	{
		return WLen;
	}

	double mean() const
	{
		int N = getLen();
		double Delta = 1.0 / (N - 1.0);
		return Delta * (Vec.sum() - (Vec(0) + Vec((UINT64)N - 1)) / 2);
	}

	void dump2Scrn() const
	{
		std::cout << "[\n" << Vec.transpose() << "\n]\n\n";
	 }

	double IP(const dataWin1&) const;
	
	Eigen::VectorXd points(int = 0)
	{
		return Vec;
	}


	//friend classes - includes all instantiated wvlet classes
	friend class dataWin;
	friend class PWvlet;
};


