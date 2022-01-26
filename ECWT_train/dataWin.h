#pragma once

#include <algorithm>
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <wtypes.h>

#include "dataWin1.h"

#include "util.h"

#define ADJ_YSCL (WM_USER + 3)
#define ADJ_YRSCL (WM_USER + 4)

class PWvlet;

class dataWin
{
	int WLen;
	dataWin1 chan[3];

public:
	dataWin(int = 0);
	dataWin(int, int, std::ifstream&);
	
	void adjYScale(HWND);
	void draw(HWND hwnd);

	void cycle()
	{
		for (int i = 0; i < 3; i++)
			chan[i].cycle();
	}

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

	dataWin centralise()
	{
		dataWin ret = *this;
		Eigen::Vector3d mu = mean();
		for (int i = 0; i < 3; i++)
			ret.chan[i].Vec = (chan[i].Vec.array() - mu(i)).matrix();
		return ret;
	}

	dataWin normalise()
	{
		dataWin ret = centralise();
		double fac = sqrt(ret.IP(ret));
		for (int i = 0; i < 3; i++)
			ret.chan[i].Vec /= fac;
		return ret;
	}

	float cordMax()
	{
		return (float)std::max<double>(chan[0].Vec.lpNorm<Eigen::Infinity>(),
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

	double GoFFun(const Eigen::MatrixXd& K, const Eigen::MatrixXd& Hi, const Eigen::MatrixXd& P);

	template <typename T>
	friend class ECWT;
	friend class PWvlet;
};

