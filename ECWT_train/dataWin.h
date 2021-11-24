#pragma once

#include <algorithm>
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <wtypes.h>

#include "dataWin1.h"

#define ADJ_YSCL (WM_USER + 3)
#define ADJ_YRSCL (WM_USER + 4)

class dataWin
{
	int WLen;
	dataWin1 chan[3];

public:
	dataWin(int = 0);
	dataWin(int, int, std::ifstream&);
	
	void adjYScale(HWND);
	void draw(HWND hwnd);

	Eigen::Vector3d mean() const
	{
		Eigen::Vector3d ret;
		for (int i = 0; i < 3; i++)
			ret(i) = chan[i].mean();
		return ret;
	}
	
	double muMag2()
	{
		Eigen::Vector3d mu = mean();
		return mu.transpose() * mu;
	}

	dataWin normalise()
	{
		dataWin ret(WLen);
		Eigen::Vector3d mu = mean();
		for (int i = 0; i < 3; i++)
		{
			ret.chan[i].Vec = (chan[i].Vec.array() - mu(i)).matrix();
			ret.chan[i].WLen = WLen;
		}
		double fac = sqrt(ret.IP(ret));
		for (int i = 0; i < 3; i++)
			ret.chan[i].Vec /= fac;
		return ret;
	}

	double cordMax()
	{
		return std::max<double>(chan[0].Vec.lpNorm<Eigen::Infinity>(),
			std::max<double>(chan[1].Vec.lpNorm<Eigen::Infinity>(), chan[2].Vec.lpNorm<Eigen::Infinity>()));
	}

	bool maintain(std::ifstream&, int = 1);

	void dump2Scrn()
	{
		chan[0].dump2Scrn();
		chan[1].dump2Scrn();
		chan[2].dump2Scrn();
	}

	double IP(const dataWin& dW) const
	{
		//assumed mapped to [0, 1)
		return chan[0].IP(chan[0]) + chan[1].IP(chan[1]) + chan[2].IP(chan[2]);
	}

	template <typename T>
	friend class ECWT;
	friend class PWvlet;
};

