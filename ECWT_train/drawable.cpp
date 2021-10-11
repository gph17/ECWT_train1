#include <iostream>

#include "drawable.h"

using namespace std;

void drawable::draw(int len)	//further input arguments needed to identify WHERE to draw
{
	vector<double> vals = points(len);
	int i;
	cout << "\n[";
	for (i = 0; i < len - 1; i++)
		cout << vals[i] << ", ";
	cout << vals[i] << "\n]\n";
}
