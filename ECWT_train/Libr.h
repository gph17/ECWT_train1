#pragma once

#include <algorithm>
#include <fstream>
#include <list>
#include <vector>

#include "ECWT.h"

class MainWindow;

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

template <typename T>
class Libr
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
	Libr<T>(int n1 = 0, int c = 0, int w = 0, std::list<int> W = std::list<int>(), size_t MS = 0) :
		n(n1), cNo(c), wCNo(w), GoFThresh(-1), LibStore(), WLs(W), prodStats(W.size()), MSize(MS)
	{
	}

	Libr<T>(char const* src, int n1, int c, int w, double g, std::list<int> W, std::size_t, int WinStep = 1);

	void setDeg(int n1)
	{
		n = n1;
	}
	void setCNo(int c)
	{
		cNo = c;
	}
	void setWCNo(int w)
	{
		wCNo = w;
	}
	void setGoFThresh(double g)
	{
		GoFThresh = g;
	}
	void setWLs(std::list<int> W)
	{
		WLs = W;
	}
	void setMS(std::size_t M)
	{
		MSize = M;
	}

	void condAdd(const ECWT<T>&);
	void dumpGoFStatsIn() const;
	void dumpGoFStats() const
	{
		std::cout << "GoF count = " << prodStats.count << ", mean = " << prodStats.muGoF << 
			", variance = " << prodStats.sig2GoF << "\n\n";
	}
	friend class MainWindow;	//if a method to forward declare a member function is discovered, replace this
								//	by friend class MainWindow::populateLib
};


template <typename T>
Libr<T>::Libr(char const* src, int n1, int c, int w, double g, std::list<int> W, std::size_t MS, int WinStep) :
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
void Libr<T>::condAdd(const ECWT<T>& ECWT1)
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
void Libr<T>::dumpGoFStatsIn() const
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
