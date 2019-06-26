//
// Created by Chen on 7/18/17.
//

#include "MVECore.h"

namespace hrp {

    MVECore::MVECore(const MVECore &core) : _params(core._params), _allNodes(core._allNodes), _thisNode(core._thisNode) {
    }

    void MVECore::updateParam(const ect_set &k, double v) {
        if (v < 0) throw hrpException("Invalid MVE parameter (negative) for contact rate.");
        _params[k] = v;
        _allNodes.insert(k.begin(), k.end());
    }

    double MVECore::getParam(const ect_set &k) const {
        auto itr = _params.find(k);
        if (itr != _params.end()) return itr->second;
        else return 0;
    }

    double MVECore::getMarginal(const ect_set &k) const {
        double retval = 0;
        for (auto itr = _params.begin(); itr!= _params.end(); itr++) {
            if (std::includes(itr->first.begin(), itr->first.end(), k.begin(), k.end()))
                retval += itr->second;
        }
        return retval;
    }

    const std::map<ect_set, double> &MVECore::get_params() const {
        return _params;
    }

    const hrpNode &MVECore::get_thisNode() const {
        return _thisNode;
    }

    const ect_set &MVECore::get_allNodes() const {
        return _allNodes;
    }

    void MVECore::set_thisNode(const hrpNode &_thisNode) {
        MVECore::_thisNode = _thisNode;
    }

    std::string MVECore::format_string() const {
        std::stringstream ss;
        for (auto itr : _params) {
            ss << "[";
            for (hrpNode n : itr.first) ss << n << ", ";
            ss << "] -> " << itr.second << std::endl;
        }
        return ss.str();
    }
}