#include "Lib.h"

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

