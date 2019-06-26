//
// Created by Chen Yang on 9/8/17.
//

#ifndef HRP_TOPOLOGYMANAGER_H
#define HRP_TOPOLOGYMANAGER_H

#include <asio/steady_timer.hpp>
#include <asio/impl/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <spdlog/spdlog.h>
#include "HrpGraph.h"
#include "common.h"
#include "rapidjson/document.h"

namespace hrp {

    class OlsrConnector;

    class TopologyManager {
    public:
        friend OlsrConnector;
        TopologyManager(const hrpNode &thisNode, asio::io_service &io_service);
        ~TopologyManager();
        void init();

        /**
         * Find the KShortest paths from _thisNode to dst
         * @param dst The destination
         * @param K The number of paths
         * @return A successor map representing the paths
         */
        HrpGraph::succMap findKShortestPath(const hrpNode& dst, int K) { return findKShortestPath(_thisNode, dst, K); }
        HrpGraph::succMap findKShortestPath(const hrpNode& src, const hrpNode& dst, int K);

        /**
         * Register callback functions for node events
         */
        void registerNodeEventListener(std::function<void(const hrp::hrpNode &, bool)>);

        /**
         * Callback function for outside library for a topology update
         * @param nodes The new set of nodes
         * @param edges The new set of edges (technically these are arcs, i.e. directed edges)
         */
        void topologyChange(std::set<hrpNode> nodes, std::set<hrpEdge> edges);

        /**
         * Method for notify topologyManager that a node is now hrp ready. Only
         * hrp ready nodes are eligible for finding K shortest paths.
         * @param node  The node that becomes hrp ready
         */
        void notifyHrpReadyNodeConnected(const hrpNode &node);
        void notifyHrpReadyNodeDisconnected(const hrpNode &node);

        const HrpGraph &get_hrpGraph() const;

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        hrpNode _thisNode;
        OlsrConnector *_olsrConnector;
        std::vector<std::function<void(const hrp::hrpNode &, bool)>> _nodeListeners;
        HrpGraph _hrpGraph;

        std::map<hrpNode, std::vector<HrpGraph::hrpPathEntry>> _kPathCache;
    };

    class OlsrConnector {
    public:
        explicit OlsrConnector(asio::io_service &io_service, TopologyManager *);
        ~OlsrConnector() { delete _fetch_timer; }
        void init();

    private:
        /**
         * Schedule the transmissions of PI probes for each neighbor, and
         * schedule the next timer to fire
         * @param error
         */
        void scheduleTopologyFetch(const std::error_code &error);

        /**
         * Schedule the next timer for a PI probe event
         */
        void scheduleTopologyFetchTimer();

        /**
         * Fecth Json from olsrd daemon using command
         * @param cmd The command pass to the olsrd
         * @return String representation of the json object
         */
        std::string fetchJsonByCmd(const std::string& cmd);

        std::set<hrpNode> fetchNodesFromJson(rapidjson::Document &j);
        std::set<hrpEdge> fetchLinksFromJson(rapidjson::Document &j, const std::set<hrpNode> &nodes);

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        TopologyManager *_topologyManager;
        asio::io_service &_io_service;
        asio::ip::tcp::socket _socket;
        asio::ip::tcp::endpoint _endpoint;
        asio::steady_timer  *_fetch_timer;    /// timer for topology fetching
    };
}

#endif //HRP_TOPOLOGYMANAGER_H
