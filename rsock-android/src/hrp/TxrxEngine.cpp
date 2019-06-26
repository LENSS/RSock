#include "TxrxEngine.h"


namespace hrp {
    TxrxEngine::TxrxEngine(const hrpNode &thisNode, asio::io_service &io_service, unsigned short port, TopologyManager *manager)
            : _thisNode(thisNode), _io_service(io_service), _port(port), _receive_buf(HRP_DEFAULT_DATA_BUF_SIZE), _send_buf(HRP_DEFAULT_DATA_BUF_SIZE),
          _socket(_io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), _topologyManager(manager), _total_originated_size(0) {
        _logger = std::make_shared<spdlog::logger>("TxrxEngine", file_sink);
        if (debug_mode)
            _logger->set_level(spdlog::level::debug);
        start_receive();
    }

  
    void TxrxEngine::scheduleTransfer(std::shared_ptr<HrpDataHeader> header, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb) {

        try {
            // fetch the routes
            hrpNode nextCarrier = header->get_next_carrier();
            hrpNode dst = header->get_dst();
          
           // _logger->info("_thisNode = {} ", _thisNode);
           // _logger->info("nextCarrier ={} ", nextCarrier);
	   // _logger->info("dst= {} ", dst);
		
		
            HrpGraph::succMap routes = _topologyManager->findKShortestPath(_thisNode, nextCarrier == "0.0.0.0" ? dst:nextCarrier, dst==nextCarrier?header->get_remain_repl():1);
            header->set_routes(routes);

            // something wrong if no next hop exist
            if (routes.count(_thisNode) == 0) {
                _logger->error("scheduleTransfer, cannot find next hop at the source");
            } else {
                std::set<hrpNode> nextHops = routes[_thisNode];
                for (auto node : nextHops) scheduleTransfer(header, node, cb);
            }
        } catch (hrpException &e) {
            _logger->error("{}", e.what());
            _logger->debug("{}", _topologyManager->get_hrpGraph().pretty_print());
        } catch (...) {
            throw;
    }

    }


    void TxrxEngine::scheduleTransfer(std::shared_ptr<HrpDataHeader> header, hrpNode nextHop,
                                      std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb) {

        //_logger->debug("scheduleTransfer sending to: {}", nextHop);


        std::shared_ptr<std::string> tmpbuf = std::make_shared<std::string>(std::string());
        header->SerializeToString(tmpbuf.get());

       
        
       // Added by Ala Altaweel
       int socket_buffer_size;
       asio::socket_base::send_buffer_size orig_option;
       _socket.get_option(orig_option);
       socket_buffer_size= orig_option.value();
       //_logger->info("Before socket_buffer_size: {}", socket_buffer_size);
    
       asio::socket_base::send_buffer_size option_mod(1048576);
       _socket.set_option(option_mod);
       _socket.get_option(orig_option);
       socket_buffer_size= orig_option.value();
       //_logger->info("After socket_buffer_size: {}", socket_buffer_size);


	
       // End by Ala Altaweel

	if(tmpbuf->size()>1024)
	        _logger->info("scheduleTransfer sending to: {}, id={}, seqnum={}", nextHop, header->get_hash(), header->get_seqnum());

       _socket.async_send_to(asio::buffer(tmpbuf->c_str(), tmpbuf->size()),
                              asio::ip::udp::endpoint(asio::ip::address::from_string(nextHop), _port),
                              std::bind(&TxrxEngine::handle_send,
                                        this, tmpbuf, header, cb,
                                        std::placeholders::_1,
                                        std::placeholders::_2));
        

    }
    void
    TxrxEngine::handle_send(std::shared_ptr<std::string> buf,
                            std::shared_ptr<HrpDataHeader> header,
                            std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb,
                            const asio::error_code &error,
                            std::size_t transferred_size) {


    if(error){

        _logger->error("returned error: {}", error.message());
            return;
    }
    
    //if(transferred_size>1000)
    //          _logger->info("inside handle_send");
    //          _logger->info("transferred_size: {}", transferred_size);    
            
    
     cb(header, static_cast<bool>(!error));

    }


    void TxrxEngine::registerHandler(hrp::hrp_packet_type type, std::function<void(std::shared_ptr<HrpDataHeader>)> cb) {
        auto itr = _handlers.find(type);
        if (itr == _handlers.end()) {
            std::vector<std::function<void(std::shared_ptr<HrpDataHeader> header)> > callbacks;
            callbacks.push_back(cb);
            _handlers.insert(std::pair<hrp_packet_type, std::vector<std::function<void(std::shared_ptr<HrpDataHeader>)>>>(type, callbacks));
        } else {
            std::vector<std::function<void(std::shared_ptr<HrpDataHeader> header)> > &callbacks = itr->second;
            callbacks.push_back(cb);
        }
    }

    void TxrxEngine::init() {
        _thread_io_service = std::thread(&TxrxEngine::run_io_service, this);
    }

    void TxrxEngine::shutdown() {
        _logger->info("total_transferred {}", _total_originated_size);
        _io_service.stop();
    }

    void TxrxEngine::start_receive() {
	    //_logger->info("start_receive begin");


	    // Added by Ala Altaweel
	    int socket_buffer_size;
	    asio::socket_base::receive_buffer_size orig_option;
	    _socket.get_option(orig_option);
	    socket_buffer_size= orig_option.value();
	    // _logger->info("Before socket_buffer_size: {}", socket_buffer_size);


	    
	    asio::socket_base::receive_buffer_size option_mod(1048576);
	    _socket.set_option(option_mod);
	    _socket.get_option(orig_option);
	    socket_buffer_size= orig_option.value();
	    //_logger->info("After socket_buffer_size: {}", socket_buffer_size);


	   
	    // End by Ala Altaweel



	    _socket.async_receive_from(asio::buffer(_receive_buf),
		                           _remote_endpoint,
		                           std::bind(&TxrxEngine::handle_receive,
		                                     this, std::placeholders::_1, /* error code */
		                                     std::placeholders::_2) /* bytes_transferred */);
	    //_logger->info("start_receive end");

    }

    void TxrxEngine::handle_receive(const asio::error_code &error, std::size_t size) {
        if (error) {
            _logger->error("handle_receive, error = {}", error.message());
            _logger->info("handle_receive, error = {}", error.message());
            return;
        }
    
       //_logger->debug("handle_receive received size = {} ", size);
       //_logger->info("handle_receive received size = {} ", size);

        std::string tmpbuf(_receive_buf.begin(), _receive_buf.begin() + size);
        std::shared_ptr<HrpDataHeader> header = std::make_shared<HrpDataHeader>(HrpDataHeader(tmpbuf));

	if (header->get_next_carrier() != _thisNode && header->get_dst() != _thisNode)
	{
             _logger->info("handle_receive packet received to be forwarded, next_carrier={}, seqnum={}", header->get_next_carrier(),header->get_seqnum());
            handle_forward(header);
	}

        else {
        if(size>1000)
                _logger->info("A packet for this node with size: {}", size);
            hrp_packet_type type = header->get_type();
            auto itr = _handlers.find(type);
            if (itr == _handlers.end()) {
                _logger->error("handle_receive, cannot find type: {} in _handlers", type);
            } else {
                std::vector<std::function<void(std::shared_ptr<HrpDataHeader>)> > &callbacks = itr->second;
                for (std::function<void(std::shared_ptr<HrpDataHeader>)> cb : callbacks)
                    cb(header);
            }
        }
    
               
    	start_receive();
    }

    void TxrxEngine::handle_forward(std::shared_ptr<HrpDataHeader> header) {
        std::set<hrpNode> nextHops = header->get_next_hop_for(_thisNode);

        // something wrong if no next hop exist
        if (nextHops.empty()) {
            _logger->warn("handle_forward, nextHop is empty, dst: {}", header->get_dst());
        } else {
            for (const auto &node : nextHops){
                scheduleTransfer(header, node, [node, this](std::shared_ptr<HrpDataHeader> h, bool succ){
                           /*          if (succ && h->get_type() == data)
                                         _logger->info("handle_forward, forwarded msg = {} to {}", h->get_hash(), node);
                                     else
                                         _logger->info("handle_forward, cannot forward msg = {} to {}", h->get_hash(), node);
                             */
			    }
                );
//                if (header->get_type() == data)
//                    _logger->info("handle_forward, forward msg = {}, to {}", header->get_hash(), node);
            }
        }
    }

    

    
    void TxrxEngine::run_io_service() {
        _io_service.run();
        _logger->info("run_io_service called");
    }

    std::thread &TxrxEngine::get_thread_io_service() {
        return _thread_io_service;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    void NeighborDiscoveryAgent::init() {
        // initialize the pi_timer
        _timer = new asio::steady_timer(_io_service);
        if (_timer == NULL) {
            std::cout << "ERROR in HrpCore::init(), cannot initialize _pi_timer" << std::endl;
            exit(1);
        }
        _timer->expires_from_now(std::chrono::seconds((long long)1));
        _timer->async_wait(std::bind(&NeighborDiscoveryAgent::scheduleHello, this, std::placeholders::_1));
        start_receive();
    }

    void NeighborDiscoveryAgent::start_receive() {
        _recv_socket.async_receive_from(asio::buffer(_receive_buf),
                                   _remote_endpoint,
                                   std::bind(&NeighborDiscoveryAgent::handle_receive,
                                             this, std::placeholders::_1, /* error code */
                                             std::placeholders::_2) /* bytes_transferred */);
    }

    void NeighborDiscoveryAgent::handle_send(const std::error_code &error, std::size_t size) {

    }

    void NeighborDiscoveryAgent::handle_receive(const std::error_code &error, std::size_t size) {
        if (error) {
            std::cout << "ERROR in " << __PRETTY_FUNCTION__ << " error: " << error.message() << std::endl;
        } else {
            const std::string& remote_peer = _remote_endpoint.address().to_string();
            if (remote_peer != _addr) {
                // std::cout << "receive " << _cnt++ << " hello from " << _remote_endpoint.address() << ":" << _remote_endpoint.port() << std::endl;
                auto itr = _peers.find(remote_peer);
                if (itr == _peers.end()) {
                    std::cout << "peer connected: " << remote_peer << std::endl;
                    // Call all the callbacks
                    for (auto cb : _availableListeners) cb(remote_peer);

                    _peers.insert(std::pair<std::string, std::chrono::steady_clock::time_point>(
                            remote_peer, std::chrono::steady_clock::now()));
                } else
                    itr->second = std::chrono::steady_clock::now();
            }
        }
        start_receive();
    }

    void NeighborDiscoveryAgent::scheduleHello(const std::error_code &error) {
        if (error) {
            std::cout << "ERROR in " << __PRETTY_FUNCTION__ << " error: " << error.message() << std::endl;
            return;
        }

        say_hello();
        purge_peer();

        _timer->expires_from_now(std::chrono::seconds((long long)1));
        _timer->async_wait(std::bind(&NeighborDiscoveryAgent::scheduleHello, this, std::placeholders::_1));
    }

    void NeighborDiscoveryAgent::say_hello() {
        _send_socket.async_send_to(asio::buffer(_send_buf), _mcast_endpoint, std::bind(&NeighborDiscoveryAgent::handle_send, this, std::placeholders::_1, std::placeholders::_2));
    }

    void NeighborDiscoveryAgent::purge_peer() {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        for (auto itr = _peers.begin(); itr != _peers.end(); ) {
            if (now - itr->second > std::chrono::seconds(5)) {
                std::cout << "peer disconnected: " << itr->first << std::endl;
                // Call all the callbacks
                for (auto cb : _unavailableListeners) cb(itr->first);
                itr = _peers.erase(itr);
            } else
                itr++;
        }
    }
}

