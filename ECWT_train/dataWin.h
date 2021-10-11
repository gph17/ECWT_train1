#pragma once

#include <Eigen/Dense>
#include <fstream>
#include <iostream>

#include "dataWin1.h"

class dataWin
{
	int WLen;
	dataWin1 chan1;
	dataWin1 chan2;
	dataWin1 chan3;

public:
	dataWin(int, int);
	dataWin(int, int, std::ifstream&);

	bool maintain(std::ifstream&, int = 1);

	void dump2Scrn()
	{
		chan1.dump2Scrn();
		chan2.dump2Scrn();
		chan3.dump2Scrn();
	}

	double IP(const dataWin& dW) const
	{
		//assumed mapped to [0, 1)
		return chan1.IP(chan1) + chan2.IP(chan2) + chan3.IP(chan3);
	}

	template <typename T>
	friend class ECWT;
	friend class PWvlet;
};

