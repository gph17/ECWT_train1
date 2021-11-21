#include <string>

#include "dataWin.h"

using namespace Eigen;
using namespace std;

dataWin::dataWin(int SR1, int WL) : WLen(WL), chan1(SR1, WLen), chan2(SR1, WLen), chan3(SR1, WLen)
{
}

dataWin::dataWin(int SR1, const int WL, ifstream& source): dataWin::dataWin(SR1, WL)
{
	char line[256];
	char tmp[256];
	int i, j;
	for (i = 0; i < WLen; i++)
	{
		source.getline(line, 256);//ignore header later
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
		chan1.Vec(i) = values[0];
		chan2.Vec(i) = values[1];
		chan3.Vec(i) = values[2];
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
	chan1.Vec(seq(0, WLen - k - 1)) = chan1.Vec(lastN(WLen - k));
	chan2.Vec(seq(0, WLen - k - 1)) = chan2.Vec(lastN(WLen - k));
	chan3.Vec(seq(0, WLen - k - 1)) = chan3.Vec(lastN(WLen - k));
	int j;
	for (j = 0; j < k; j++)
	{
		source >> chan1.Vec(WLen - k + j);
		source.ignore(10, ',');
		if (source.eof())
			return false;
		source >> chan2.Vec(WLen - k + j);
		source.ignore(10, ',');
		if (source.eof())
			return false;
		source >> chan3.Vec(WLen - k + j);
	}
	return true;
}
