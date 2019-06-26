//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_METADATAEXCHANGER_H
#define HRP_METADATAEXCHANGER_H

#include "HrpRouter.h"
#include "HrpCore.h"
#include "TxrxEngine.h"
#include "hrpMessages.h"
#include "mve/MVECore.h"


namespace hrp
{
	class MetaDataExchanger
	{
		public:
		MetaDataExchanger(HrpRouter *router, HrpCore *core, TxrxEngine *engine) : _router(router), _hrp_core(core), _engine(engine)
		{
			_logger = std::make_shared<spdlog::logger>("MetaDataExchanger", file_sink);
			if (debug_mode)
				_logger->set_level(spdlog::level::debug);
		}
		
		virtual ~MetaDataExchanger()
		{
		}

		virtual void init();

		/**
		* Outside lib used to notify this module that a new
		* node is available. We need to exchange metadata.
		* @param sa The addr for the new node
		*/
		// void nodeAvailable(sockaddr_in &sa);
		void nodeAvailable(const hrpNode& n);


		/**
		* Outside lib used to notify this module that a
		* node is unavailable now. We need to notify other modules
		* @param sa
		*/
		// void nodeUnavailable(sockaddr_in &sa);
		void nodeUnavailable(const hrpNode& n);

		/**
		* Receive handler. If the received packet is a MVEResponse,
		* we need to add the peer node to _connected_nodes; if
		* the received packet is a MVERequest, we need to send
		* a MVEResponse.
		* @param header The header of the message received
		*/
		void rxReceived(std::shared_ptr<HrpDataHeader> header);

		/**
		* Callback function for scheduling transmission
		* @param pkt The packet that was scheduled
		* @param success If it was a success
		*/
		void TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success);

		private:
	
		/**
		* Schedule a transmission of MVERequest
		* @param peer  The node to send to
		*/
		void scheduleMveRequest(const hrpNode& peer);
		
		void scheduleMveRequestTimer();

		void scheduleMveRequestForDiscovered();

		/**
		* Handler for MVERequest
		* @param request The received MVERequest
		*/
		void processMveRequest(std::shared_ptr<HrpDataHeader> request);

		/**
		* Handler for MVEResponse
		* @param response The received MVEResponse
		*/
		void processMveResponse(std::shared_ptr<HrpDataHeader> response);

		private:
		// Logger
		std::shared_ptr<spdlog::logger> _logger;
		HrpCore     *_hrp_core;
		TxrxEngine  *_engine;
		HrpRouter   *_router;
		std::mutex          _mutex_nodes;
		std::set<hrpNode>      _discovered_nodes;
		std::set<hrpNode>      _connected_nodes;
		asio::steady_timer  *_request_timer;    /// timer for topology fetching
	};

	
	class MetaDataExchangerWithTpManager : public MetaDataExchanger
	{
		public:
		MetaDataExchangerWithTpManager(HrpRouter *router, HrpCore *core, TxrxEngine *engine, TopologyManager *topologyManager)
		:MetaDataExchanger(router, core, engine),_topologyManager(topologyManager)
		{
		}

		virtual void init()
		{
			MetaDataExchanger::init();
			_topologyManager->registerNodeEventListener(std::bind(&MetaDataExchangerWithTpManager::nodeEvent, this, std::placeholders::_1, std::placeholders::_2));
		}

		void nodeEvent(const hrpNode &node, bool connected)
		{
			if (connected)
				MetaDataExchanger::nodeAvailable(node);
			else
				MetaDataExchanger::nodeUnavailable(node);
		}

		private:
		TopologyManager *_topologyManager;
	};

	class MetaDataExchangerWithND : public MetaDataExchanger
	{
		public:
		MetaDataExchangerWithND(HrpRouter *router, HrpCore *core, TxrxEngine *engine)
		: MetaDataExchanger(router, core, engine),
		// Commented and Replaced by Ala
		/*
		_nd_agent(new NeighborDiscoveryAgent(core->get_thisNode(), engine->get_io_service()))
		*/
		_nd_agent(new NeighborDiscoveryAgent(core->get_thisNode(), engine->get_io_service()))
		// End Ala
		{
		}

		virtual ~MetaDataExchangerWithND()
		{
			delete(_nd_agent);
		}
			
		virtual void init()
		{
			MetaDataExchanger::init();
			_nd_agent->init();
			_nd_agent->registerListeners(std::bind(&MetaDataExchangerWithND::nodeAvailable, this, std::placeholders::_1),
			std::bind(&MetaDataExchangerWithND::nodeUnavailable, this, std::placeholders::_1));
		}

		private:
			NeighborDiscoveryAgent *_nd_agent;
	};
}
#endif //HRP_METADATAEXCHANGER_H
