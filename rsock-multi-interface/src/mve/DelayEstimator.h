//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_DELAYESTIMATOR_H
#define HRP_DELAYESTIMATOR_H

#include <vector>
#include <limits>
#include "MVECore.h"
#include "common.h"

namespace hrp
{

	// double getTotalIndirectRate(const MVECore &srcCore, const hrpNode &dst, const MVECore &dstCore);
	std::vector<double> calcDelays(unsigned int rmax, const MVECore &srcCore, const MVECore &dstCore);
}

#endif //HRP_DELAYESTIMATOR_H
