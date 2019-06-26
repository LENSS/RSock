//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <hrpNode.h>
#include "hrp/HrpGraph.h"

int main() {


    hrp::HrpGraph graph;
    hrp::hrpNode nodes[] = {hrp::hrpNode("n0"), hrp::hrpNode("n1"), hrp::hrpNode("n2"), hrp::hrpNode("n3"), hrp::hrpNode("n4")};
    graph.addNode(nodes[0]);
    graph.addNode(nodes[1]);
    graph.addNode(nodes[2]);
    graph.addNode(nodes[3]);
    graph.addNode(nodes[4]);

    graph.addArc(nodes[0], nodes[1], 1.0);
    graph.addArc(nodes[0], nodes[2], 1.0);
    graph.addArc(nodes[1], nodes[4], 1.0);
    graph.addArc(nodes[2], nodes[3], 1.0);
    graph.addArc(nodes[1], nodes[2], 1.0);
    graph.addArc(nodes[3], nodes[4], 1.0);

    std::map<hrp::hrpNode, std::set<hrp::hrpNode>> succMap = hrp::HrpGraph::convertKPathToSuccMap(graph.findKShortestPath(nodes[0], nodes[1], 3));

    int cnt = 0;
    std::cout << "{";
    for (auto itr : succMap) {
        if (cnt != 0) std::cout << ", " << std::endl;
        std::cout << itr.first << ": [";
        int inner_cnt = 0;
        for (auto n : itr.second) {
            if (inner_cnt != 0) std::cout << ", ";
            std::cout << n;
            inner_cnt++;
        }
        std::cout << "]";
        cnt++;
    }
    std::cout << "}" << std::endl;

    return 0;
}
