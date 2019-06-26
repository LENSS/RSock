//
// Created by Chen Yang on 9/8/17.
//

#include <asio/connect.hpp>
#include <hrpEexception.h>
#include "TopologyManager.h"

namespace hrp {

    TopologyManager::TopologyManager(const hrpNode &thisNode, asio::io_service &io_service)
            : _thisNode(thisNode), _olsrConnector(new OlsrConnector(io_service, this)) {
        _logger = std::make_shared<spdlog::logger>("TopologyManager", file_sink);
        if (debug_mode)
            _logger->set_level(spdlog::level::debug);
    }

    TopologyManager::~TopologyManager() {
        delete _olsrConnector;
    }

    void TopologyManager::registerNodeEventListener(std::function<void(const hrp::hrpNode &, bool)> cb) {
        _nodeListeners.push_back(cb);
    }

    void TopologyManager::init() {
        _olsrConnector->init();
    }

    void TopologyManager::notifyHrpReadyNodeConnected(const hrpNode &node) {

    }

    void TopologyManager::notifyHrpReadyNodeDisconnected(const hrpNode &node) {

    }

    HrpGraph::succMap TopologyManager::findKShortestPath(const hrpNode &src, const hrpNode &dst, int K) {
        if (_kPathCache.count(dst) != 0) {
            if (_kPathCache[dst].size() >= K) {
                std::vector<HrpGraph::hrpPathEntry> paths(_kPathCache[dst].begin(), _kPathCache[dst].begin() + K);
                return HrpGraph::convertKPathToSuccMap(paths);
            }
        }
        try {
            _kPathCache[dst] = _hrpGraph.findKShortestPath(src, dst, K);
        } catch (const hrpException &e) {
            throw e;
        }

        return HrpGraph::convertKPathToSuccMap(_kPathCache[dst]);
    }

    void TopologyManager::topologyChange(std::set<hrpNode> nodes, std::set<hrpEdge> edges) {
        std::map<hrpNode, lemon::ListDigraphBase::Node> &m = _hrpGraph.get_mutable_vertices();
        lemon::ListDigraph &g = _hrpGraph.get_mutable_graph();
        std::set<hrpNode> oldNodes;
        for (auto itr : m) oldNodes.insert(itr.first);

        // get difference
        std::set<hrpNode> availableNodes, unavailableNodes;
        std::set_difference(nodes.begin(), nodes.end(), oldNodes.begin(), oldNodes.end(), std::inserter(availableNodes, availableNodes.begin()));
        std::set_difference(oldNodes.begin(), oldNodes.end(), nodes.begin(), nodes.end(), std::inserter(unavailableNodes, unavailableNodes.begin()));

        // reconstruct the graph
        g.clear();
        m.clear();
        _kPathCache.clear();
        for (const hrpNode& node : nodes) _hrpGraph.addNode(node);
        for (hrpEdge edge : edges) _hrpGraph.addArc(edge.peer1, edge.peer2, edge.length);

        // notify others of node changes
        for (const hrpNode& node : unavailableNodes) {
            if (node == _thisNode) continue;
            for (std::function<void(const hrp::hrpNode &, bool)> cb : _nodeListeners)
                cb(node, false);
        }

        for (const hrpNode& node : availableNodes) {
            if (node == _thisNode) continue;
            for (std::function<void(const hrp::hrpNode &, bool)> cb : _nodeListeners)
                cb(node, true);
        }

        // _hrpGraph.pretty_print();
    }

    const HrpGraph &TopologyManager::get_hrpGraph() const {
        return _hrpGraph;
    }

    OlsrConnector::OlsrConnector(asio::io_service &io_service, TopologyManager *manager)
            : _topologyManager(manager), _io_service(io_service),
              _socket(_io_service), _endpoint(asio::ip::address::from_string("127.0.0.1"), 9090) {
        _logger = std::make_shared<spdlog::logger>("OlsrConnector", file_sink);
        if (debug_mode)
            _logger->set_level(spdlog::level::debug);
    }

    void OlsrConnector::init() {

        using namespace asio::ip;

        // conenct to the olsr server
        try {
            _socket.connect(_endpoint);
            _logger->info("init, connected to olsr daemon");
            _socket.close();
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }


        _fetch_timer = new asio::steady_timer(_io_service);
        if (_fetch_timer == NULL) {
            _logger->error("init, cannot initialize _fetch_timer");
            exit(1);
        }

        scheduleTopologyFetchTimer();
    }

    std::string OlsrConnector::fetchJsonByCmd(const std::string &cmd) {
        std::string str;

        try {
            _socket.connect(_endpoint);
            _socket.send(asio::buffer(cmd, cmd.length()));
            std::vector<char> buf(1024);
            asio::error_code socket_err;


            for (;;) {

                size_t len = _socket.read_some(asio::buffer(buf), socket_err);

                if (socket_err == asio::error::eof)
                    break; // Connection closed cleanly by peer.

                if (socket_err)
                    throw asio::system_error(socket_err); // Some other error.

                std::string nstr(buf.begin(), buf.begin() + len);

                /*
                // this if statement was used when using olsrv2 OONF
                if (nstr.find(">") != std::string::npos) {
                    str = str + nstr.substr(0, nstr.find(">"));
                    break;
                }
                 */

                str += nstr;
            }
            _socket.close();
        } catch (std::exception &e) {
            _logger->error("{}", e.what());
            exit(1);
        }

        return str;
    }

    std::set<hrpNode> OlsrConnector::fetchNodesFromJson(rapidjson::Document &j) {
        std::set<hrpNode> nodes;
        const rapidjson::Value& routes = j["routes"];
        for (const auto& v : routes.GetArray()) {
            nodes.insert(v["destination"].GetString());
        }
        // fetch the routes first, ensure that all nodes are routable

        // also add this node
        nodes.insert(_topologyManager->_thisNode);

        return nodes;
    }

    std::set<hrpEdge> OlsrConnector::fetchLinksFromJson(rapidjson::Document &j, const std::set<hrpNode> &nodes) {
        std::set<hrpEdge> edges;
        const rapidjson::Value& routes = j["topology"];
        for (const auto& v : routes.GetArray()) {
            hrpNode p1(v["lastHopIP"].GetString());
            hrpNode p2(v["destinationIP"].GetString());
            if (nodes.count(p1)!=0 && nodes.count(p2)!=0)
                edges.insert(hrpEdge(p1, p2, v["linkQuality"].GetDouble()));
        }
        // fetch the topology, i.e. edges

        return edges;
    }


    void OlsrConnector::scheduleTopologyFetch(const std::error_code &error) {
        //char sendbuf[] = "netjsoninfo filter graph ipv4_0\x0d\x0a";
        std::string str = fetchJsonByCmd("/topology/routes\x0d\x0a");

//        std::cout << str << std::endl;

        rapidjson::Document j;
        j.Parse(str.c_str());
        std::set<hrpNode> nodes = fetchNodesFromJson(j);
        std::set<hrpEdge> edges = fetchLinksFromJson(j, nodes);

//        std::cout << "nodes: ";
//        for (auto node : nodes) std::cout << node << " ";
//        std::cout << "\nedges: ";
//        for (auto edge : edges) std::cout << edge << " ";
//        std::cout << std::endl;

        _topologyManager->topologyChange(nodes, edges);

        scheduleTopologyFetchTimer();
    }

    void OlsrConnector::scheduleTopologyFetchTimer() {
        _fetch_timer->expires_from_now(std::chrono::seconds((long long)HRP_DEFAULT_OLSR_FETCH_EPOCH));
        _fetch_timer->async_wait(std::bind(&OlsrConnector::scheduleTopologyFetch, this, std::placeholders::_1));
    }
}