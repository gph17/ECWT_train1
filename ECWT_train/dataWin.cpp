#include <string>

#include "dataWin.h"

using namespace Eigen;
using namespace std;

dataWin::dataWin(int WL) : WLen(WL)
{
}

dataWin::dataWin(int SR1, const int WL, ifstream& source): dataWin::dataWin(WL)
{
	char line[256];
	char tmp[256];
	int i, j;
	for (i = 0; i < 3; i++)
	{
		chan[i].SR = SR1;
		chan[i].WLen = WLen;
		chan[i].Vec.setZero(WLen);
	}
	source.getline(line, 256);
	for (i = 0; line[i]; i++)
		if (!isdigit(line[i]) && (line[i] != '-') && (line[i] != '.') && (line[i] != ',') && (line[i] != ' ')
			&& (line[i] != '+') && (line[i] != 'E') && (line[i] != 'e'))
		{
			source.getline(line, 256);
			break;
		}
	for (i = 0; i < WLen; i++)
	{
		if (source.eof())
		{
			cerr << "Input file too short\n";
			exit(-1);
		}
		char* ptr1 = line;
		char* ptr2 = tmp;
		double values[3];
		for (j = 0; j < 2; j++)
		{
			while (*ptr1 != ',')
				*(ptr2++) = *(ptr1++);
			ptr1++;
			*ptr2 = '\0';
			values[j] = stod(tmp);
			ptr2 = tmp;
		} 
		while (*ptr1)
			*(ptr2++) = *(ptr1++);
		*ptr2 = '\0';
		values[2] = stod(tmp);
		for (int j = 0; j < 3; j++)
			chan[j].Vec(i) = values[j];
		if (i < WLen - 1)
			source.getline(line, 256);
	}
}

bool dataWin::maintain(std::ifstream& source, int k)
{
	if (k <= 0)
	{
		cerr << "Step has to be positive\n";
		exit(-1);
	}
	if (source.eof())
		return false;
	int j;
	for (j = 0; j < k; j++)
		chan[j].Vec(seq(0, WLen - k - 1)) = chan[j].Vec(lastN(WLen - k));
	for (j = 0; j < k; j++)
	{
		int idx = WLen - k + j;
		source >> chan[0].Vec(idx);
		source.ignore(10, ',');
		if (source.eof())
			return false;
		source >> chan[1].Vec(idx);
		source.ignore(10, ',');
		if (source.eof())
			return false;
		source >> chan[2].Vec(idx);
	}
	return true;
}

void dataWin::adjYScale(HWND hwnd)
{
	float ySc = (float)cordMax();
	SendMessage(hwnd, ADJ_YSCL, NULL, reinterpret_cast<LPARAM>(&ySc));
}

void dataWin::draw(HWND hwnd)
{
	//find co-ordinate value vectors of wvlets in triplet
	int i;
	Eigen::VectorXf pwConst[3];
	for (i = 0; i < 3; i++)
		pwConst[i] = chan[i].points();
	//find max abs co-ord value over vectors of triplet
	float ySc = std::max<float>(pwConst[0].lpNorm<Eigen::Infinity>(),
		std::max<float>(pwConst[1].lpNorm<Eigen::Infinity>(), pwConst[2].lpNorm<Eigen::Infinity>()));
	//send message updating yscale of window
	SendMessage(hwnd, ADJ_YRSCL, NULL, reinterpret_cast<LPARAM>(&ySc));
	//send messages to draw value vectors
	SendMessage(hwnd, DATAWDRAW, NULL, reinterpret_cast<LPARAM>(&pwConst));
}

double dataWin::GoFFun(const MatrixXd& K, const MatrixXd& Hi, const MatrixXd& P)
{
	double GoF = 0;
	dataWin tmpDw = centralise();
	VectorXd tmp;
	for (int i = 0; i < 3; i++)
	{
		tmp = P.transpose() * (K * chan[i].Vec);
		GoF += tmp.transpose() * (Hi * tmp);
	}
	GoF /= tmpDw.IP(tmpDw);
	return GoF;
}
