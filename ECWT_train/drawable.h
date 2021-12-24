/* either data from a data channel, or a wavelet*/

#pragma once

#include <Eigen/Dense>
#include <vector>
#include <wtypes.h>

class drawable
{
	UINT msg = 0;
public:
	virtual Eigen::VectorXf points(int) = 0;	//convert to values to be plotted against time
	virtual void draw(HWND hwnd, Eigen::VectorXf vals)
	{
		SendMessage(hwnd, this->msg, NULL, reinterpret_cast<LPARAM>(&vals));
	}
};

