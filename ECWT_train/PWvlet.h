#pragma once

#include <Eigen/Dense>
#include <map>
#include <list>
#include <set>
#include <utility>

#include "drawable.h"
#include "dataWin1.h"
#include "ECWT.h"
#include "util.h"
#include "wvlet.h"

class PWvlet :
    public wvlet, public drawable
{
	UINT msg = WVLTDRAW;
public:
	static Eigen::MatrixXd H;	//generate for highest value of n encountered - for lower values, it can be used in 
								//	truncated form
	static Eigen::Matrix<long long, Eigen::Dynamic, Eigen::Dynamic> W;	
		//lists for WRAPPED continuity conditions - set to zero size, then generate for largest n and wCNo
		//	encountered - truncate for smaller n and wCNo (for CONTINUITY conditions, its partner is too simple to
		//  be worth storing, i.e., identity matrix padded); b also too simple to be worth storing
	static std::map<int, Eigen::Matrix<long long, Eigen::Dynamic, Eigen::Dynamic>> Hi;	//generate for each n 
																						//	encountered
	static std::map<complexKey, Eigen::MatrixXd> G;	//modified H^{-1} - generate for each (n, cNo, wNo) encountered

	static std::map<int, Eigen::MatrixXd> fTrans;

	static const std::set<complexKey> valid;
	static std::set<int> getValid();
	static std::set<std::pair<int, int>> getValid(int);
	/*make static members*/
	static void makeH(int);
	static void makeG(int, int, int);
	static void makeW(int, int);
	static void maintainFTrans(int, int);
	/*make inner products*/
	double IP(const wvlet& wv) const	//L2 IP
	{
		return wv.para.transpose() * (H(Eigen::seq(0, wv.getn()), Eigen::seq(0, n)) * para);
	}

	double IP(const dataWin1&) const;	//L2 IP

	PWvlet(int n1, int cNo1 = 0, int wNo1 = 0);
	PWvlet(dataWin1, int n1, int cNo1 = 0, int wCNo1 = 0);
	PWvlet() = default;

	void populate(const dataWin1&);
	
	Eigen::VectorXf points(int);

	friend class ECWT<PWvlet>;

};

