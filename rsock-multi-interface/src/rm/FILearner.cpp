//
// Created by Chen on 7/19/17.
//

#include "FILearner.h"

namespace hrp
{
	FILearner::FILearner(RMCore &core, double alpha, const MVECore &srcMVE, std::mutex &mutex_srcMVE, MVECore_ptr *dstMVE, std::mutex &mutex_dstMVE)
	: _core(core), _alpha(alpha), _srcMVE(srcMVE), _mutex_srcMVE(mutex_srcMVE), _dstMVE(dstMVE), _mutex_dstMVE(mutex_dstMVE)
	{

	}

	void FILearner::updateCore()
	{

		// Get the delays, construct a loss vector, then call the receive loss method
		MVECore *srcMVE, *dstMVE;
		{
			std::lock_guard<std::mutex> lg(_mutex_srcMVE);
			srcMVE = new MVECore(_srcMVE);
		}
		{
			std::lock_guard<std::mutex> lg(_mutex_dstMVE);
			dstMVE = new MVECore(**_dstMVE);
		}

		std::vector<double> delays = calcDelays(_core.get_rmax(), *srcMVE, *dstMVE);
		std::vector<double> loss(_core.get_rmax());
		bool meaningful = false;
		for (unsigned int i=0; i<_core.get_rmax(); i++)
		{
			loss[i] = RMCore::calcLoss(_alpha, i+1, _core.get_rmax(), delays[i], delays[0]);
			if (delays[i] != std::numeric_limits<double>::max())
				meaningful = true;
		}

		if (meaningful)
			_core.receiveLossVec(loss);

		delete srcMVE;
		delete dstMVE;

		//std::stringstream ss;
		//if (meaningful) ss << "updated, ";
		//else ss << "not updated, ";
		//
		//for (int i=0; i<delays.size(); i++) {
		//ss << delays[i] << " ";
		//}
		//ss << std::endl;
		//return ss.str();
	}
}
