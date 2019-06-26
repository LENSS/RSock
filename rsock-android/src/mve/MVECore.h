//
// Created by Chen on 7/18/17.
//

#ifndef HRP_MVECORE_H
#define HRP_MVECORE_H

#include <set>
#include <map>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <algorithm>
#include "hrpNode.h"
#include "hrpEexception.h"

namespace hrp{

    class MVECore {
    public:

        /** Default constructor */
        explicit MVECore(const hrpNode &thisNode) : _thisNode(thisNode) {}

        MVECore(const MVECore& core);

        MVECore(const std::map<ect_set, double> &params, const ect_set &allNodes) : _params(params), _allNodes(allNodes) {}

        /**
         * Update the parameter (contact rate) for the encounter set of k.
         * If k is not in the _params, then a new entry will be added.
         * @param k The set of encountered node
         * @param v The new contact rates
         */
        void updateParam(const ect_set& k, double v);

        /**
         * Get the parameter (contact rate) for the encounter set of k
         * @param k The set of encountered node
         * @return The contact rates, or zero if k is not in the _params
         */
        double getParam(const ect_set& k) const ;

        /**
         * Get the marginal distribution of the encounter set of k.
         * @param k
         * @return
         */
        double getMarginal(const ect_set& k) const ;

        void set_thisNode(const hrpNode &_thisNode);

        const std::map<ect_set, double> &get_params() const;
        const hrpNode &get_thisNode() const;
        const ect_set &get_allNodes() const;

        std::string format_string() const;

    private:
        std::map<ect_set, double> _params;
        ect_set _allNodes;
        hrpNode    _thisNode;
    };


    /**
     * A wrapper class that only provides getters
     */
    class MVEDist {
    public:
        explicit MVEDist(MVECore& core) : _core(core) {};

        /**
         * Get the parameter (contact rate) for the encounter set of k
         * @param k The set of encountered node
         * @return The contact rates, or zero if k is not in the _params
         */
        double getParam(const ect_set& k) const { return _core.getParam(k); }

        /**
         * Get the marginal distribution of the encounter set of k.
         * @param k
         * @return
         */
        double getMarginal(const ect_set& k) const { return _core.getMarginal(k); }

    private:
        MVECore& _core;
    };
}



#endif //HRP_MVECORE_H
