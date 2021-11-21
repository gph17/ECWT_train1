#pragma once

#include <algorithm>
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
	Eigen::Vector3d mean()
	{
		Eigen::Vector3d ret;
		ret(0) = chan1.mean();
		ret(1) = chan2.mean();
		ret(2) = chan3.mean();
		return ret;
	}
	
	double muMag2()
	{
		Eigen::Vector3d mu = mean();
		return mu.transpose() * mu;
	}

	dataWin normalise()
	{
		dataWin ret(chan1.SR, WLen);
		Eigen::Vector3d mu = mean();
		ret.chan1.Vec = (chan1.Vec.array() - mu(0)).matrix();
		ret.chan2.Vec = (chan2.Vec.array() - mu(1)).matrix();
		ret.chan3.Vec = (chan3.Vec.array() - mu(2)).matrix();
		double fac = sqrt(ret.IP(ret));
		ret.chan1.Vec /= fac;
		ret.chan2.Vec /= fac;
		ret.chan3.Vec /= fac;
		return ret;
	}

	double cordMax()
	{
		return std::max<double>(chan1.Vec.lpNorm<Eigen::Infinity>(),
			std::max<double>(chan2.Vec.lpNorm<Eigen::Infinity>(), chan3.Vec.lpNorm<Eigen::Infinity>()));
	}

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

