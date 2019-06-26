//
// Created by Chen on 7/19/17.
//

#ifndef HRP_FILEARNER_H
#define HRP_FILEARNER_H

#include <mutex>
#include "RMCore.h"
#include "mve/MVECore.h"
#include "mve/DelayEstimator.h"

namespace hrp {
    class FILearner {
    public:
        typedef MVECore* MVECore_ptr;
        FILearner(RMCore &core, double alpha, const MVECore &srcMVE, std::mutex &mutex_srcMVE, MVECore_ptr *dstMVE, std::mutex &mutex_dstMVE);

        /**
         * Update the RMCore this learner is maintaining.
         */
        void updateCore();

        std::string format_probs() const { return _core.format_probs(); }

    private:
        RMCore &_core;
        const MVECore &_srcMVE;
        std::mutex    &_mutex_srcMVE;
        MVECore_ptr     *_dstMVE;
        std::mutex    &_mutex_dstMVE;
        double _alpha;
    };
}

#endif //HRP_FILEARNER_H
