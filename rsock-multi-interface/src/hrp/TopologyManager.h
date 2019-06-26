//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_TOPOLOGYMANAGER_H
#define HRP_TOPOLOGYMANAGER_H

#include <asio/steady_timer.hpp>
//#include <asio/impl/io_service.hpp>
#include <asio/io_service.hpp>

#include <asio/ip/tcp.hpp>
#include <spdlog/spdlog.h>
#include "HrpGraph.h"
#include "common.h"


// Android does not have getifaddr() in NDK, so we have to provide it.
// Code is from a Mozila site.
#ifdef __ANDROID__
	#include "ifaddrs-android.h"
	#include <android/log.h>
#else
	#include <ifaddrs.h>
#endif


namespace hrp
{


	class OlsrConnector;

	class TopologyManager
	{
		public:
		// OlsrConnector is a friend class of TopologyManager
		friend OlsrConnector;
		// Commented and replaced by Ala
		/*
		TopologyManager(const hrpNode &thisNode, asio::io_service &io_service);
		*/
		TopologyManager(asio::io_service &io_service);
		~TopologyManager();
		void init();

		/**
		* Find the KShortest paths from _thisNode to dst
		* @param dst The destination
		* @param K The number of paths
		* @return A successor map representing the paths
		*/
		HrpGraph::succMap findKShortestPath(const hrpNode& dst, int K)
		{
			return findKShortestPath(_thisNode, dst, K);
		}
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
		// commented out since it becomes global to keep track of interfaces change
		//hrpNode _thisNode;
		OlsrConnector *_olsrConnector;
		std::vector<std::function<void(const hrp::hrpNode &, bool)>> _nodeListeners;
		HrpGraph _hrpGraph;
		std::map<hrpNode, std::vector<HrpGraph::hrpPathEntry>> _kPathCache;
	};

	// This class is responsible for fetching the topology from olsr
	class OlsrConnector
	{
		public:
		explicit OlsrConnector(asio::io_service &io_service, TopologyManager *);
		~OlsrConnector()
		{
			delete _fetch_timer;
		}

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
		* A refresh function to call GNS for caching
		*/
		void refreshGNS(rapidjson::Document &j);
		
		/**
		* Fecth Json from olsrd daemon using command
		* @param cmd The command pass to the olsrd
		* @return String representation of the json object
		*/
		std::string fetchJsonByCmd(const std::string& cmd);
		std::set<hrpNode> fetchNodesFromJson(rapidjson::Document &j);
		std::set<hrpEdge> fetchLinksFromJson(rapidjson::Document &j, const std::set<hrpNode> &nodes);

		// A function to replace IPs into GUIDs in JsonObject
		void replaceIPstoGUIDsJsonDoc();
		

		private:
		/// Logger
		std::shared_ptr<spdlog::logger> _logger;
		TopologyManager *_topologyManager;
		asio::io_service &_io_service;
		asio::ip::tcp::socket _socket;
		asio::ip::tcp::endpoint _endpoint;
		// this is the timer for topology fetching from olsr
		asio::steady_timer  *_fetch_timer;    

		std::mutex  _mutex_table;
		std::mutex  _mutex_table_for_thisNode;

	};
}
#endif //HRP_TOPOLOGYMANAGER_H
