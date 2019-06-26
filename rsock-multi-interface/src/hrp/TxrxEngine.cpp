//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "TxrxEngine.h"
#include "common.h"

namespace hrp
{

	// Commented and replaced by Ala
	/*	
	TxrxEngine::TxrxEngine(const hrpNode &thisNode, asio::io_service &io_service, unsigned short port, TopologyManager *manager):
	_thisNode(thisNode),
	*/	
	TxrxEngine::TxrxEngine(asio::io_service &io_service, unsigned short port, TopologyManager *manager):
	_io_service(io_service),
	_port(port),
	_receive_buf(HRP_DEFAULT_DATA_BUF_SIZE),
	_send_buf(HRP_DEFAULT_DATA_BUF_SIZE),
	_socket(_io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
	_topologyManager(manager),
	_total_originated_size(0)
	{
		_logger = std::make_shared<spdlog::logger>("TxrxEngine", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);


		// Added by Ala to increase the asio buffer size		   
		asio::socket_base::receive_buffer_size option_mod(1048576);
		_socket.set_option(option_mod);

		// Print the size of socket buffer
		asio::socket_base::receive_buffer_size socket_buffer_size;		
		_socket.get_option(socket_buffer_size);
		_logger->info("Initialize _socket buffers size: {}", socket_buffer_size.value());
		// End by Ala

		start_receive();
	}



	std::vector<std::string> TxrxEngine::get_node_IPs(hrpNode& node)
	{
		std::vector<std::string> IPAddresses;
		if(node.size()==0)
		{
			_logger->warn("get_node_IPs empty node");			
		}
		else if(node.size()==1)
		{
			
			// create an object of gns client
			hrp::GnsServiceClient gnsObj;
			// get node GUID
			std::string GUID = node.at(0);
		
			if(GUID.size() != 0)			
			{		
				// get IPs for this GUID
				IPAddresses = gnsObj.getIPbyGUID(GUID);
				if(IPAddresses.size() == 0)
					_logger->warn("get_node_IPs: no IP addresses returned for GUID: {}", GUID);					
			}
			else
			{
					_logger->warn("get_node_IPs node has no GUID");					
			}
		}
		return IPAddresses;
	}
  
	void TxrxEngine::scheduleTransfer(std::shared_ptr<HrpDataHeader> header, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb)
	{
		try
		{
			//_logger->debug("TxrxEngine::scheduleTransfer Start");			
			// fetch the routes

			hrpNode nextCarrier = header->get_next_carrier_by_guid();
			hrpNode dst = header->get_dst_by_guid();
			/*
			_logger->debug(printHrpNode("_thisNode ", _thisNode));
			_logger->debug(printHrpNode("nextCarrier ", nextCarrier));
			_logger->debug(printHrpNode("dst ", dst));
			*/
			
			
			
			

			// Commented and Replaced by Ala
			/*
			//HrpGraph::succMap routes = _topologyManager->findKShortestPath(_thisNode, nextCarrier == "0.0.0.0" ? dst:nextCarrier, dst==nextCarrier?header->get_remain_repl():1);
			*/
			
			

			
			// create an object of gns client
			GnsServiceClient gnsObj;
			// get GUID by IP
			string ownGUID =  gnsObj.getOwnGUID();
			hrpNode hrpNodeObj;
			if(ownGUID.size() != 0)			
			{	
				hrpNodeObj.push_back(ownGUID);
			}
			else
			{
				_logger->error("no GUID returned from getOwnGUID() function");			

			}
				
			// commented out to pass nodes without IPs
			HrpGraph::succMap routes = _topologyManager->findKShortestPath(_thisNode, nextCarrier == hrpNodeObj ? dst:nextCarrier, dst==nextCarrier?header->get_remain_repl():1);		


			
			//HrpGraph::succMap routes = _topologyManager->findKShortestPath(_thisNode, nextCarrier==_thisNode? dst:nextCarrier, dst==nextCarrier?header->get_remain_repl():1);	
			// End Ala

			header->set_routes(routes);
			
			// something wrong if no next hop exist
			if (routes.count(_thisNode) == 0)
			{
				_logger->error("scheduleTransfer, cannot find next hop at the source");
			}
			else
			{
				std::set<hrpNode> nextHops = routes[_thisNode];
				
				for (auto node : nextHops)
				{
					//_logger->debug(printHrpNode("node in the nextHops after adding IPs: ", node));
					scheduleTransfer(header, node, cb);
				}
			}
			//_logger->debug("TxrxEngine::scheduleTransfer End");			

		}
		
		catch (hrpException &e)
		{
			_logger->error("TxrxEngine::scheduleTransfer cath exception: {}", e.what());		
			_logger->info("skipping the exception");	
			_logger->debug("print current hrpGraph: {}", _topologyManager->get_hrpGraph().pretty_print());
		}
		catch (...)
		{
			throw;
		}
	}


	void TxrxEngine::scheduleTransfer(std::shared_ptr<HrpDataHeader> header, hrpNode nextHop, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb)
	{

		//_logger->debug("TxrxEngine::scheduleTransfer start");

		//_logger->debug("TxrxEngine::scheduleTransfer sending to: {}", printHrpNode("",nextHop));

		std::shared_ptr<std::string> tmpbuf = std::make_shared<std::string>(std::string());
		header->SerializeToString(tmpbuf.get());

		// Add IPs to nextHop so we can send packets to
		std::vector<std::string>  _IPs = get_node_IPs(nextHop);

		
		// Print the size of socket buffer
		/*
		asio::socket_base::receive_buffer_size socket_buffer_size;		
		_socket.get_option(socket_buffer_size);
		_logger->info("_socket buffers size for sending: {}", socket_buffer_size.value());
		*/
		
		/*
		if(tmpbuf->size()>1024)
		 	_logger->debug("scheduleTransfer sending to: {}, id={}, seqnum={}", printHrpNode("",nextHop), header->get_hash(), header->get_seqnum());
		*/



	
		//_logger->debug("_IPs.size(): {}", _IPs.size());

		if(_IPs.size() == 0)
		{
			_logger->debug("No IP address to send to");
		}

		// below code send to all available IPs
		/*
		for(int i=0; i<_IPs.size(); i++)
		{
			if(_IPs.at(i).size() != 0)
			{
				//_logger->debug("send to endpoint IP address: {}", _IPs.at(i));
				_socket.async_send_to(asio::buffer(tmpbuf->c_str(), tmpbuf->size()),
				asio::ip::udp::endpoint(asio::ip::address::from_string(_IPs.at(i)), _port),
				std::bind(&TxrxEngine::handle_send, this, tmpbuf, header, cb, std::placeholders::_1, std::placeholders::_2));
			}
		}		//_logger->debug("TxrxEngine::scheduleTransfer end");
		*/
		// only send to one IP address
		for(int i=0; i<1; i++)
		{
			if(_IPs.at(i).size() != 0)
			{
				//_logger->debug("send to endpoint IP address: {}", _IPs.at(i));
				_socket.async_send_to(asio::buffer(tmpbuf->c_str(), tmpbuf->size()),
				asio::ip::udp::endpoint(asio::ip::address::from_string(_IPs.at(i)), _port),
				std::bind(&TxrxEngine::handle_send, this, tmpbuf, header, cb, std::placeholders::_1, std::placeholders::_2));
			}
			else
				_logger->debug("scheduleTransfer: first IP address is empty");
		}		

	}

	void TxrxEngine::handle_send(std::shared_ptr<std::string> buf, 
	std::shared_ptr<HrpDataHeader> header,
	std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb,
	const asio::error_code &error,
	std::size_t transferred_size)
	{
		/*
		_logger->debug("handle_send header->get_src(): {}", printHrpNode("",header->get_src()));
		_logger->debug("handle_send header->get_dst_by_guid(): {}", printHrpNode("",header->get_dst_by_guid()));
		_logger->debug("handle_send header->get_current_carrier(): {}", printHrpNode("",header->get_current_carrier()));
		_logger->debug("handle_send header->get_next_carrier(): {}", printHrpNode("",header->get_next_carrier()));
		*/
		if(error)
		{
		 	
			_logger->error("returned error message and error code: {}, {}", error.message(), error.value());
			return;
		}
    
    		cb(header, static_cast<bool>(!error));
	}


	void TxrxEngine::registerHandler(hrp::hrp_packet_type type, std::function<void(std::shared_ptr<HrpDataHeader>)> cb)
	{
		//_logger->info("TxrxEngine::registerHandler begin");
		auto itr = _handlers.find(type);
		if (itr == _handlers.end())
		{
			std::vector<std::function<void(std::shared_ptr<HrpDataHeader> header)> > callbacks;
			callbacks.push_back(cb);
			_handlers.insert(std::pair<hrp_packet_type, std::vector<std::function<void(std::shared_ptr<HrpDataHeader>)>>>(type, callbacks));
		}
		else
		{
			std::vector<std::function<void(std::shared_ptr<HrpDataHeader> header)> > &callbacks = itr->second;
			callbacks.push_back(cb);
		}
		//_logger->info("TxrxEngine::registerHandler end");
	}



	void TxrxEngine::init()
	{
		_thread_io_service = std::thread(&TxrxEngine::run_io_service, this);
	}

	void TxrxEngine::shutdown()
	{
		//_logger->info("total_transferred {}", _total_originated_size);
		_io_service.stop();
	}


	void TxrxEngine::start_receive()
	{

		//_logger->debug("TxrxEngine::start_receive begin");			
		// Print the size of socket buffer
		/*
		asio::socket_base::receive_buffer_size socket_buffer_size;		
		_socket.get_option(socket_buffer_size);
		_logger->info("_socket buffers size for receivng: {}", socket_buffer_size.value());
		*/
		_socket.async_receive_from(asio::buffer(_receive_buf),
		_remote_endpoint,
		std::bind(&TxrxEngine::handle_receive,
		this, std::placeholders::_1, /* error code */
                std::placeholders::_2) /* bytes_transferred */);
		//_logger->debug("TxrxEngine::start_receive end");
	}

	void TxrxEngine::handle_receive(const asio::error_code &error, std::size_t size)
	{

		try
		{
			if (error)
			{
				_logger->error("handle_receive, error = {}", error.message());
				return;
			}
	    
	
			std::string tmpbuf(_receive_buf.begin(), _receive_buf.begin() + size);
			std::shared_ptr<HrpDataHeader> header = std::make_shared<HrpDataHeader>(HrpDataHeader(tmpbuf));
						
			/*
			_logger->info(printHrpNode("header->get_next_carrier()",header->get_next_carrier()));	
			_logger->info(printHrpNode("header->get_dst_by_guid()", header->get_dst_by_guid()));	
			_logger->info(printHrpNode("_thisNode",_thisNode));	
			*/	
	
			if (header->get_next_carrier_by_guid() != _thisNode && header->get_dst_by_guid() != _thisNode)			
			{
				_logger->info("handle_receive packet received to be forwarded, next_carrier={}, seqnum={}", printHrpNode("",header->get_next_carrier_by_guid()),header->get_seqnum());
				// End Ala
				handle_forward(header);
			}

			else
			{				
				hrp_packet_type type = header->get_type();
				auto itr = _handlers.find(type);
				if (itr == _handlers.end())
				{
					_logger->error("handle_receive, cannot find type: {} in _handlers", type);
				}
				else
				{
					std::vector<std::function<void(std::shared_ptr<HrpDataHeader>)> > &callbacks = itr->second;
					for (std::function<void(std::shared_ptr<HrpDataHeader>)> cb : callbacks)
						cb(header);
				}
			}
			
			start_receive();
		}
		catch (hrpException &e)
		{
			_logger->error("TxrxEngine::handle_receive cath exception: {}", e.what());
			_logger->info("skipping the exception");
			// continue to receive
			start_receive();
		}
		catch (...)
		{
			throw;
			// continue to receive
			start_receive();
		}
	}
	

	void TxrxEngine::handle_forward(std::shared_ptr<HrpDataHeader> header)
	{

		try
		{
			std::set<hrpNode> nextHops = header->get_next_hop_for(_thisNode);
			// something wrong if no next hop exist
			if (nextHops.empty())
			{
				// Commented and Replaced by Ala
				/*
				_logger->warn("handle_forward, nextHop is empty, dst: {}", header->get_dst_by_guid());
				*/
				_logger->warn("handle_forward, nextHop is empty, dst: {}", printHrpNode("",header->get_dst_by_guid()));
			
				// End Ala
			}
			else
			{
				for (const auto &node : nextHops)
				{
					scheduleTransfer(header, node, [node, this](std::shared_ptr<HrpDataHeader> h, bool succ){});
				}
			}
		}

		catch (hrpException &e)
		{
			_logger->error("TxrxEngine::handle_forward cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}

	}

    
	void TxrxEngine::run_io_service()
	{
		_io_service.run();
		_logger->info("run_io_service called");
	}
	
	std::thread &TxrxEngine::get_thread_io_service()
	{
		return _thread_io_service;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	void NeighborDiscoveryAgent::init()
	{
		// initialize the pi_timer

		_logger = std::make_shared<spdlog::logger>("NeighborDiscoveryAgent", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);

		_timer = new asio::steady_timer(_io_service);
		if (_timer == NULL)
		{
			_logger->error("ERROR in HrpCore::init(), cannot initialize _pi_timer");
			exit(1);
		}
		_timer->expires_from_now(std::chrono::seconds((long long)1));
		_timer->async_wait(std::bind(&NeighborDiscoveryAgent::scheduleHello, this, std::placeholders::_1));
		start_receive();
	}

	void NeighborDiscoveryAgent::start_receive()
	{
		_recv_socket.async_receive_from(asio::buffer(_receive_buf),
		_remote_endpoint,
		std::bind(&NeighborDiscoveryAgent::handle_receive,
		this, std::placeholders::_1, /* error code */
		std::placeholders::_2) /* bytes_transferred */);
	}

	void NeighborDiscoveryAgent::handle_send(const std::error_code &error, std::size_t size)
	{}

	void NeighborDiscoveryAgent::handle_receive(const std::error_code &error, std::size_t size)
	{
		if (error)
		{
			_logger->error("ERROR in {}, error: {}", __PRETTY_FUNCTION__ , error.message());
		}
		else
		{

			// Commented and Replaced by Ala
			/*
			const std::string& remote_peer = _remote_endpoint.address().to_string();
			if (remote_peer != _addr)
			{
				// std::cout << "receive " << _cnt++ << " hello from " << _remote_endpoint.address() << ":" << _remote_endpoint.port() << std::endl;
				auto itr = _peers.find(remote_peer);
				if (itr == _peers.end())
				{
					std::cout << "peer connected: " << remote_peer << std::endl;
					// Call all the callbacks
					for (auto cb : _availableListeners) 
						cb(remote_peer);

					_peers.insert(std::pair<std::string, std::chrono::steady_clock::time_point>(remote_peer, std::chrono::steady_clock::now()));
				}
				else
					itr->second = std::chrono::steady_clock::now();
			}
			*/

			const std::string& remote_peer = _remote_endpoint.address().to_string();
			// create an object of gns client
			GnsServiceClient gnsObj;
			hrpNode hrpNodeObj;
			// get GUID by IP
			std::vector<std::string> GUID = gnsObj.getGUIDbyIP(remote_peer);
			if(GUID.size() != 0)			
			{
				hrpNodeObj.push_back(GUID.at(0));
			}
			else
			{
				_logger->error("handle_receive no GUID returned for node with ip: {}",remote_peer);
			}
	
			if (hrpNodeObj != _addr)
			{
				//_logger->debug("hrpNodeObj != _addr, receive: hello from {}", printHrpNode("",hrpNodeObj));
				auto itr = _peers.find(hrpNodeObj);
				if (itr == _peers.end())
				{
					_logger->info("peer connected: {}", printHrpNode("",hrpNodeObj));
					// Call all the callbacks
					for (auto cb : _availableListeners) 
						cb(hrpNodeObj);

					_peers.insert(std::pair<hrpNode, std::chrono::steady_clock::time_point>(hrpNodeObj, std::chrono::steady_clock::now()));
				}
				else
					itr->second = std::chrono::steady_clock::now();
			}
			//else
			//	_logger->debug("hrpNodeObj == _addrr");
			// End Ala
		}
		start_receive();
	}


	void NeighborDiscoveryAgent::scheduleHello(const std::error_code &error)
	{
		if (error)
		{
			_logger->error("ERROR in {}, error: {}",__PRETTY_FUNCTION__ ,error.message());
			return;
		}

		say_hello();
		purge_peer();
		_timer->expires_from_now(std::chrono::seconds((long long)1));
		_timer->async_wait(std::bind(&NeighborDiscoveryAgent::scheduleHello, this, std::placeholders::_1));
	}

	void NeighborDiscoveryAgent::say_hello()
	{
		_send_socket.async_send_to(asio::buffer(_send_buf), _mcast_endpoint, std::bind(&NeighborDiscoveryAgent::handle_send, this, std::placeholders::_1, std::placeholders::_2));
	}

	void NeighborDiscoveryAgent::purge_peer()
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		for (auto itr = _peers.begin(); itr != _peers.end(); )
		{
			if (now - itr->second > std::chrono::seconds(5))
			{
				// Commented and Replaced by Ala
				/*
				std::cout << "peer disconnected: " << itr->first << std::endl;
				*/
				_logger->info("peer disconnected: {}", printHrpNode("",itr->first));
				// End Ala
				// Call all the callbacks
				for (auto cb : _unavailableListeners)
					cb(itr->first);
				itr = _peers.erase(itr);
			}
			else
				itr++;
		}
	}
}

