/* either data from a data channel, or a wavelet*/

#pragma once

#include <Eigen/Dense>
#include <vector>

class drawable
{
public:
	virtual Eigen::VectorXd points(int) = 0;	//convert to values to be plotted against time
};

