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


	void dump2Scrn()
	{
		std::cout << "[\n" << Vec.transpose() << "\n]\n\n";
	 }

	double IP(const dataWin1&) const;

	//friend classes - includes all instantiated wvlet classes
	friend class dataWin;
	friend class PWvlet;
};


