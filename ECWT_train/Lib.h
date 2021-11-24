#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <list>
#include <thread>
#include <vector>

#include "ECWT.h"

#define DISPLAYLSIZE WM_USER
#define DISPLAYGVAL (WM_USER + 1)

class stats
{
public:
	double muGoF, sig2GoF, muWL, sig2WL;
	int count;
	stats(int WNo) : muGoF(0), sig2GoF(0), muWL(0), sig2WL(0), count(0)
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
class Lib
{
	std::vector<ECWT<T>> LibStore;
	std::list<int> WLs;
	stats prodStats;
	int n;
	int cNo;
	int wCNo;
	std::size_t LSize;
	double GoFThresh;

public:
	Lib<T>(int n1 = 0, int c = -1, int w = -1, double g = -1, std::list<int> W = std::list<int>(), size_t MS = 0) :
		n(n1), cNo(c), wCNo(w), GoFThresh(g), LibStore(), WLs(W), prodStats((int)(W.size())), LSize(MS)
	{
	}

	Lib<T>(wchar_t const* src, int n1, int c, int w, double g, std::list<int> W, std::size_t, int WinStep = 1);

	void build(wchar_t const*, int,  HWND = NULL, 
		std::chrono::duration<double> shFreq = std::chrono::duration<double>::zero());

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
		LSize = M;
	}

	bool condAdd(const ECWT<T>&);
	void dumpGoFStatsIn() const;
	void dumpGoFStats() const
	{
		std::cout << "GoF count = " << prodStats.count << ", mean = " << prodStats.muGoF << 
			", variance = " << prodStats.sig2GoF << "\n\n";
	}

	friend class MainWindow;
};

template <typename T>
Lib<T>::Lib(wchar_t const* src, int n1, int c, int w, double g, std::list<int> W, std::size_t MS, int WinStep) :
	n(n1), cNo(c), wCNo(w), GoFThresh(g), LibStore(), WLs(W), prodStats(W.size()), LSize(MS)
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
bool Lib<T>::condAdd(const ECWT<T>& ECWT1)
{
	if (ECWT1.GoF >= GoFThresh)
	{
		if (LibStore.size() < LSize - 1)	//no check needed
		{
			LibStore.push_back(ECWT1);
			return true;
		}
		if (LibStore.size()== LSize - 1)	//no check needed
		{
			LibStore.push_back(ECWT1);
			make_heap(LibStore.begin(), LibStore.end(), comp());
			return true;
		}

		if (ECWT1 > LibStore.front())//LibStore is minimum heap
		{
			pop_heap(LibStore.begin(), LibStore.end(), comp());	//"decapitates" the heap, reorganises the remainder
																//	into a new heap, places the head at the end
				LibStore.pop_back();	//removes the old head
				LibStore.push_back(ECWT1);	//adds the new member
				push_heap(LibStore.begin(), LibStore.end(), comp());	//moves the last member up the tree until all
																		//	members constitute a heap
				return true;
		}
	}
	return false;
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

template<typename T>
void Lib<T>::build(wchar_t const* src, int WinStep, HWND hwnd, std::chrono::duration<double> shFreq)
{
	auto stime = std::chrono::system_clock::now();
	std::ifstream ifs;
	ECWT<T> last;
	dataWin lastDW;
	bool better;
	for (auto i : WLs)
	{
		int start = 0;
		ifs.open(src);
		dataWin dW(n, i, ifs);
		ECWT<T> ECWT1(dW, n, cNo, wCNo, start, src);
		prodStats.update(ECWT1.GoF, ECWT1.WLen);
		better = condAdd(ECWT1);
		if (better)
		{
			last = ECWT1;
			lastDW = dW;
		}
		if (better && hwnd && (shFreq > std::chrono::duration<double>::zero()))
			{
			//do display stuff
			SendMessage(hwnd, DISPLAYLSIZE, (WPARAM)(LibStore.size()), NULL);
			double GoF = LibStore.front().GoF;
			WPARAM tmp = reinterpret_cast<WPARAM>(&GoF);
			SendMessage(hwnd, DISPLAYGVAL, tmp, NULL);
			//normalise dataWin
			dW = dW.normalise();
			//send message to adjust y-scale for dataWin
			dW.adjYScale(hwnd);
			//send message to re-adjust y-scale for ECWT and draw wavelets
			ECWT1.draw(hwnd);
			//send message to draw dataWin1s
			dW.draw(hwnd); 
			stime = std::chrono::system_clock::now();
		}
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
			better = condAdd(ECWT1);
			if (better)
			{
				last = ECWT1;
				lastDW = dW;
			}
			if (better && hwnd && (shFreq > std::chrono::duration<double>::zero()) &&
				((std::chrono::system_clock::now() - stime) > shFreq))
			{
				//do display stuff
				SendMessage(hwnd, DISPLAYLSIZE, (WPARAM)(LibStore.size()), NULL);
				double GoF = LibStore.front().GoF;
				WPARAM tmp = reinterpret_cast<WPARAM>(&GoF);
				SendMessage(hwnd, DISPLAYGVAL, tmp, NULL);
				//normalise dataWin
				dW = dW.normalise();
				//send message to adjust y-scale for dataWin
				dW.adjYScale(hwnd);
				//send message to re-adjust y-scale for ECWT and draw wavelets
				ECWT1.draw(hwnd);
				//send message to draw dataWin1s
				dW.draw(hwnd);
				stime = std::chrono::system_clock::now();
			}
		}
		ifs.close();
		SendMessage(hwnd, DISPLAYLSIZE, (WPARAM)(LibStore.size()), NULL);
		double GoF = LibStore.front().GoF;
		WPARAM tmp = reinterpret_cast<WPARAM>(&GoF);
		SendMessage(hwnd, DISPLAYGVAL, tmp, NULL);
		//normalise dataWin
		lastDW = lastDW.normalise();
		//send message to adjust y-scale for dataWin
		lastDW.adjYScale(hwnd);
		//send message to re-adjust y-scale for ECWT and draw wavelets
		last.draw(hwnd);
		//send message to draw dataWin1s
		lastDW.draw(hwnd);
	}
}

