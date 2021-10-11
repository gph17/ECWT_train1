#pragma once

#include <algorithm>
#include <fstream>
#include <list>
#include <vector>

#include "ECWT.h"

class stats
{
public:
	double muGoF, sig2GoF, muWL, sig2WL;
	int count;
	stats(int WNo) : muGoF(0), sig2GoF(0), count(0)
	{
	}
	void update(double GoF, double WL);
};

struct comp
{
	bool operator()(const ECWT<PWvlet>& ECWT1, const ECWT<PWvlet>& ECWT2)
	{
		return ECWT1 > ECWT2;
	}
};

void stats::update(double GoF, double WL)
{
	if (count)
	{
		muGoF = (count * muGoF + GoF) / (1.0 + count);
		sig2GoF = count * sig2GoF / (1.0 + count) + (muGoF - GoF) * (muGoF - GoF) / count;
		muWL = (count * muWL + WL) / (1.0 + count);
		sig2WL = count * sig2WL / (1.0 + count) + (muWL - WL) * (muWL - WL) / count;
	}
	else
	{
		muGoF = GoF;
		muWL = WL;
	}
	count++;
}

template <typename T>
class Lib
{
	std::vector<ECWT<T>> LibStore;
	std::list<int> WLs;
	stats prodStats;
	int n;
	int cNo;
	int wCNo;
	std::size_t MSize;
	double GoFThresh;

public:
	Lib<T>(int n1, int c, int w, double g, std::list<int>W, size_t MS) :
		n(n1), cNo(c), wCNo(w), GoFThresh(g), LibStore(), WLs(W), prodStats(W.size()), MSize(MS)
	{
	}

	Lib<T>(char const* src, int n1, int c, int w, double g, std::list<int>W, std::size_t, int WinStep = 1);
	void condAdd(const ECWT<T>&);
	void dumpGoFStatsIn() const;
	void dumpGoFStats() const
	{
		std::cout << "GoF count = " << prodStats.count << ", mean = " << prodStats.muGoF << 
			", variance = " << prodStats.sig2GoF << "\n\n";
	}
};


template <typename T>
Lib<T>::Lib(char const* src, int n1, int c, int w, double g, std::list<int> W, std::size_t MS, int WinStep) :
	n(n1), cNo(c), wCNo(w), GoFThresh(g), LibStore(), WLs(W), prodStats(W.size()), MSize(MS)
{
	std::ifstream ifs;
	for (auto i : WLs)
	{
		int start = 0;
		ifs.open(src);
		dataWin dW(n, i, ifs);
		ECWT<T> ECWT1(dW, n, cNo, wCNo, start, src);
		prodStats.update(ECWT1.GoF, ECWT1.WLen);
		condAdd(ECWT1);
		bool dataLeft = !ifs.eof();
		while (dataLeft)
		{
			dW.maintain(ifs, WinStep);
			dataLeft = !ifs.eof();
			if (!dataLeft)
				break;
			start += WinStep;
			ECWT<T> ECWT1(dW, n, cNo, wCNo, start, src);
			prodStats.update(ECWT1.GoF, ECWT1.WLen);
			condAdd(ECWT1);
		}
		ifs.close();
		if (!LibStore.empty())
			std::cout << "Best GoF: " << LibStore.front().GoF << "\n" <<
			"Another GoF: " << LibStore[1].GoF << "\n";
		else
			std::cout << "Library is empty\n";
	}
}

template<typename T>
void Lib<T>::condAdd(const ECWT<T>& ECWT1)
{
	if (ECWT1.GoF <= GoFThresh)
	{
		if (LibStore.size() < MSize - 1)	//no check needed
		{
			LibStore.push_back(ECWT1);
			return;
		}
		if (LibStore.size()== MSize - 1)	//no check needed
		{
			LibStore.push_back(ECWT1);
			make_heap(LibStore.begin(), LibStore.end(), comp());
			return;
		}

		if (ECWT1 < LibStore.back())
		{
				LibStore.pop_back();
				LibStore.push_back(ECWT1);
				push_heap(LibStore.begin(), LibStore.end(), comp());
				return;
		}
	}
}

template <typename T>
void Lib<T>::dumpGoFStatsIn() const
{
	int count = LibStore.size();
	double GSum = 0, G2Sum = 0, WSum = 0, W2Sum = 0;
	for (auto i : LibStore)
	{
		GSum += i.GoF;
		G2Sum += i.GoF * i.GoF;
		WSum += i.WLen;
		W2Sum += i.WLen * i.WLen;
	}
	std::cout << "Included count = " << count << ", mean = " << GSum / count <<
		", variance = " << G2Sum / count - (GSum * GSum) / count / count << "\n\n";
	std::cout << "Included WLen mean = " << WSum / count <<
		", variance = " << W2Sum / count - (WSum * WSum) / count / count << "\n\n";
}
