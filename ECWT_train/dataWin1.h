#pragma once

#include <Eigen/Dense>
#include <iostream>

class dataWin1
{
	int SR;//per second
	int WLen;
	Eigen::VectorXd Vec;

public:
	dataWin1(int, int);
	int getLen()
	{
		return WLen;
	}

	double mean()
	{
		int N = getLen();
		double Delta = 1.0 / (N - 1.0);
		return Delta * (Vec.sum() - (Vec(0) + Vec(N - 1)) / 2);
	}

	void dump2Scrn()
	{
		std::cout << "[\n" << Vec.transpose() << "\n]\n\n";
	 }

	double IP(const dataWin1&) const;
	
	Eigen::VectorXd points(int)
	{
		return Vec;
	}


	//friend classes - includes all instantiated wvlet classes
	friend class dataWin;
	friend class PWvlet;
};


