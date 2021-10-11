/* either data from a data channel, or a wavelet*/

#pragma once

#include <vector>

class drawable
{
public:
	virtual std::vector<double> points(int) = 0;	//convert to values to be plotted against time
	void draw(int);	//further input arguments needed to identify WHERE to draw
};

