//
// Created by Chen on 7/18/17.
//

#ifndef HRP_NODE_H
#define HRP_NODE_H

#include <string>
#include <set>
#include <iostream>
#include <arpa/inet.h>

namespace hrp {
    typedef std::string hrpNode;
    typedef std::set<hrpNode> ect_set; /// Encounter set

    inline int node_to_ip(const hrpNode &n) {
        int ret = 0;
        inet_pton(AF_INET, n.c_str(), &ret);
        return ret;
    }

    inline hrpNode ip_to_node(int addr) {
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
        return hrp::hrpNode(str);
    }

    /**
     * Utility function. Convert the sockaddr_in to node
     * @param sa
     * @return
     */
    hrpNode sock2node(const sockaddr_in &sa);

    class hrpEdge {
    public:
        hrpEdge(hrpNode& p1, hrpNode& p2, double l) : peer1(p1), peer2(p2), length(l) {}
        friend bool operator<(const hrpEdge &e1, const hrpEdge &e2) {
            if (e1.peer1 < e2.peer1) return true;
            else if (e1.peer1 > e2.peer1) return false;
            else return e1.peer2 < e2.peer2;
        }
        friend std::ostream& operator<<(std::ostream &stream, const hrpEdge &e) {
            stream << "[" << e.peer1 << " -> " << e.peer2 << ", lq = " << e.length << "]";
            return stream;
        }

        hrpNode peer1, peer2;
        double length;
    };
}

#endif //HRP_NODE_H
