//
// Created by Chen Yang on 9/6/17.
//

#include <hrpNode.h>
#include <map>
#include <hrpEexception.h>
#include <queue>
#include "HrpGraph.h"


namespace hrp {
    void HrpGraph::addNode(const hrpNode &n) {
	 _logger = std::make_shared<spdlog::logger>("HrpGraph", file_sink);
        if (debug_mode)
            _logger->set_level(spdlog::level::debug);
        if (_vertices.count(n) == 0) {
            lemon::ListDigraph::Node addedNode = _graph.addNode();
            _vertexNames[addedNode] = n;
            _vertices[n] = addedNode;
        }
    }

    void HrpGraph::addArc(const hrpNode &n1, const hrpNode &n2, double length) {
        if (length < 0) throw hrpException("HrpGraph::addArc, length < 0");

        if (_vertices.count(n1) != 0 && _vertices.count(n2) != 0) {
            auto arc = lemon::findArc(_graph, _vertices[n1], _vertices[n2]);
            if (arc == lemon::INVALID)
                arc = _graph.addArc(_vertices[n1], _vertices[n2]);
            _arcLength[arc] = length;
        } else
            throw hrpException("HrpGraph::addArc, one of the node is not in this graph");
    }

    void HrpGraph::delNode(const hrpNode &n) {
        if (_vertices.count(n) != 0) {
            _graph.erase(_vertices[n]);
            _vertices.erase(n);
        } else
            throw hrpException("HrpGraph::delNode node is not in the graph");
    }

    void HrpGraph::delEdge(const hrpNode &n1, const hrpNode &n2) {
        if (_vertices.count(n1) == 0 || _vertices.count(n2) == 0)
            throw hrpException("HrpGraph::delEdge nodes are not in the graph");

        lemon::ListDigraph::Arc arc = lemon::findArc(_graph, _vertices[n1], _vertices[n2]);
        if (arc != lemon::INVALID) {
            _graph.erase(arc);
        } else
            throw hrpException("HrpGraph::delEdge the edge is not in the graph");
    }

    /*
     * Sample code for temporally remove edge
     *
    std::vector<hrpNode> HrpGraph::dijkstra(const hrpNode &src, const hrpNode &dst, const hrpNode &hide1, const hrpNode &hide2) {
        lemon::ListGraph::Edge edge = lemon::findEdge(_graph, _vertices[hide1], _vertices[hide2]);
        lemon::ListGraph::EdgeMap<bool> filter(_graph, true);
        filter[edge] = false;
        lemon::FilterEdges<lemon::ListGraph> subgraph(_graph, filter);


        std::vector<hrpNode> ret;
        lemon::Dijkstra<lemon::FilterEdges<lemon::ListGraph>, lemon::ListGraph::EdgeMap<double>> dij(subgraph, _arcLength);
        dij.run(_vertices[src], _vertices[dst]);

        auto p = dij.path(_vertices[dst]);
        for (lemon::ListGraph::Node v = _vertices[dst]; v != _vertices[src]; v = dij.predNode(v)) {
            ret.push_back(_vertexNames[v]);
        }
        ret.push_back(src);

        return ret;
    }
     */



    std::vector<HrpGraph::hrpPathEntry> HrpGraph::findKShortestPath(const hrpNode &src, const hrpNode &dst, int K) {
        std::lock_guard<std::mutex> lg(_graph_mutex);

        if (src.length() == 0 || dst.length() == 0 || _vertices.count(src) == 0 || _vertices.count(dst) == 0)
            throw hrpException("HrpGraph::findKShortestPath Invalid argument, src or dst is not in the graph");
        if (src == dst)
            throw hrpException("HrpGraph::findKShortestPath Invalid argument, src and dst should not be the same");

        /// This algorithm is implemented based on Yen's algorithm for finding K shortest paths
        /// The pseudocode was based on Wiki page: https://en.wikipedia.org/wiki/Yen%27s_algorithm

        // Declare compare object for priority_queue
        auto compare = [] (hrpPathEntry e1, hrpPathEntry e2) { return e1.second > e2.second; };

        // Initialize A, B container
        // A Track the current k shortest paths
        std::vector<hrpPathEntry> A;
        // B is the priority_queue ranked by the distance of the paths, smaller at front
        std::priority_queue<hrpPathEntry, std::vector<hrpPathEntry>, decltype(compare)> B(compare);


        // Determine the shortest path from the source to the sink.
        // A[0] = Dijkstra(Graph, source, sink);
        lemon::ListDigraph::NodeMap<bool> nf(_graph, true);
        lemon::ListDigraph::ArcMap<bool> ef(_graph, true);
        lemon::SubDigraph<lemon::ListDigraph> sg(_graph, nf, ef);
        hrpPathEntry p1 = dijkstra(src, dst, sg);

	//commented out by Ala
        //_logger->info("call dijkstra for: {}, {}", src, dst);

	//_logger->info("src: {}", src );
        //_logger->info("dst: {}", dst);
        // If we cannot find a path
        if (p1.second == HRPGRAPH_UNREACHABLE_PATH) throw hrpException("HrpGraph::findKShortestPath cannot find path to dst");

        // otherwise, start the algorithm
        A.push_back(p1);


        for (int k=1; k<K; k++) {
            const hrpPathEntry& entry = A[k-1];
            const hrpPath& p = entry.first;
            double prev_dist = entry.second;

            // sum tracks the residual path length of p, i.e. length(p) - length(roothPath)
            double sum = 0;

            // The spur hrpNode ranges from the first hrpNode to the next to last hrpNode in the previous k-shortest path.
            // Here we traverse reversely to easily track rootPath length
            for (int i=p.size()-2; i>=0; i--) {

                // Declare hrpNode, edge filter for removing nodes/edges
                lemon::ListDigraph::NodeMap<bool> nodeFilter(_graph, true);
                lemon::ListDigraph::ArcMap<bool> arcFilter(_graph, true);

                // Spur hrpNode is retrieved from the previous k-shortest path, k-1.
                hrpNode spurNode = p[i];
                // Get the distance between i-th hrpNode to (i+1)-th hrpNode
                double arcDist = _arcLength[lemon::findArc(_graph, _vertices[p[i]], _vertices[p[i+1]])];
                sum += arcDist;

                // The sequence of nodes from the source to the spur hrpNode of the previous k-shortest path
                hrpPath rootPath = hrpPath(p.begin(), p.begin() + i + 1);
                // The length of the rootPath
                double rootPathDist = prev_dist - sum;

                // for each path in A
                for (hrpPathEntry entryj : A) {
                    const hrpPath& pathj = entryj.first;
                    if (pathj.size() > i+1) {
                        // if rootPath == pathj.nodes(0,i)
                        if (std::equal(pathj.begin(), pathj.begin() + i + 1, rootPath.begin())) {
                            // Remove the links that are part of the previous shortest paths which share the same root path
                            const lemon::ListDigraph::Node &del1 = _vertices[*(pathj.begin()+i)];
                            const lemon::ListDigraph::Node &del2 = _vertices[*(pathj.begin()+i+1)];
                            lemon::ListDigraph::Arc arc = lemon::findArc(_graph, del1, del2);
                            arcFilter[arc] = false;
                        }
                    }
                }

                // for each hrpNode in rootPath except spurNode
                for (hrpNode ith : rootPath) {
                    if (ith == spurNode) continue;
                    // remove the hrpNode from the graph
                    nodeFilter[_vertices[ith]] = false;
                }

                // Now calculate the spur path from the spurNode to the dst
                lemon::SubDigraph<lemon::ListDigraph> subGraph(_graph, nodeFilter, arcFilter);
                hrpPathEntry spurPathEntry = dijkstra(spurNode, dst, subGraph);

                // If such path exists
                if (spurPathEntry.second != HRPGRAPH_UNREACHABLE_PATH) {
                    hrpPath spurPath = spurPathEntry.first;

                    // total path is made up of the root path and the spur path
                    rootPath.insert(rootPath.end(), spurPath.begin()+1, spurPath.end());
                    double totalPathDist = rootPathDist + spurPathEntry.second;

                    // add the potential k-shortest path to the heap
                    B.push(hrpPathEntry(rootPath, totalPathDist));
                }

            }

            // if there's no spur paths left, just break
            if (B.empty()) break;

            // else pick the one with shortest length, added it to A
            A.push_back(B.top());
            B.pop();
        }

        if (A.empty()) throw hrpException("HrpGraph::findKShortestPath no route to dst");
        return A;
    }

    HrpGraph::succMap HrpGraph::convertKPathToSuccMap(const std::vector<HrpGraph::hrpPathEntry> &A) {
        std::map <hrpNode, std::set<hrpNode>> retmap;
        for (hrpPathEntry entry : A) {
            const hrpPath &path = entry.first;
            for (int i=0; i<path.size()-1; i++) {
                if (retmap.count(path[i]) == 0) retmap[path[i]] = std::set<hrpNode>();
                retmap[path[i]].insert(path[i+1]);
            }
        }
        return retmap;
    }

    HrpGraph::hrpPathEntry HrpGraph::dijkstra(const hrpNode &src, const hrpNode &dst, lemon::SubDigraph<lemon::ListDigraph> &subgraph) {
        hrpPath ret;
        lemon::Dijkstra<lemon::SubDigraph<lemon::ListDigraph>, lemon::ListDigraph::ArcMap<double>> dij(subgraph, _arcLength);

        // Run Dijkstra's algorithm on this subgraph. It returns true if it's reachable
        if (dij.run(_vertices[src], _vertices[dst])) {
            // Get the path
            auto p = dij.path(_vertices[dst]);
            // Trace back to the source, and construct the return value
            for (lemon::ListDigraph::Node v = _vertices[dst]; v != _vertices[src]; v = dij.predNode(v)) {
                ret.insert(ret.begin(), _vertexNames[v]);
            }
            ret.insert(ret.begin(), src);

            hrpPathEntry entry(ret, dij.dist(_vertices[dst]));
            return entry;
        }

        hrpPathEntry entry(ret, HRPGRAPH_UNREACHABLE_PATH);
        return entry;

    }


    const lemon::ListDigraph &HrpGraph::get_graph() const {
        return _graph;
    }

    const std::map<hrpNode, lemon::ListDigraphBase::Node> &HrpGraph::get_vertices() const {
        return _vertices;
    }

    std::string HrpGraph::pretty_print() const {
        std::stringstream ss;
        ss << "nodes: ";
        for (auto itr = _vertices.begin(); itr != _vertices.end(); itr++) {
            ss << itr->first << " ";
        }
        ss << "\nedges: ";
        for (lemon::ListDigraph::ArcIt it(_graph); it != lemon::INVALID; ++it) {
            ss << _vertexNames[_graph.source(it)] << " -> " << _vertexNames[_graph.target(it)] << " lq: " << _arcLength[it] << std::endl;
        }
        return ss.str();
    }
}
