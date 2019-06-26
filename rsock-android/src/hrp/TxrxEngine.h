//
// Created by Chen on 7/27/17.
//

#ifndef HRP_TXRXENGINE_H
#define HRP_TXRXENGINE_H

#define HRP_TXRX_MAX_BUF_SIZE 2048

#include <array>
#include <thread>
#include <vector>
#include <iostream>
#include <asio.hpp>
#include "common.h"
#include "hrpMessages.h"
#include "TopologyManager.h"

namespace hrp {
    class TxrxEngine {
    public:
        explicit TxrxEngine(const hrpNode& thisNode, asio::io_service &io_service, unsigned short port, TopologyManager *);

        void init();
        void shutdown();

        void scheduleTransfer(std::shared_ptr<HrpDataHeader> header, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb);

        /**
         * Register the packet handler for a specific packet type
         * @param type  The packet type
         * @param cb    The packet reception handler
         */
        void registerHandler(hrp::hrp_packet_type type, std::function<void(std::shared_ptr<HrpDataHeader>)> cb);

        asio::io_service& get_io_service() { return _io_service; }

        std::thread &get_thread_io_service();

    private:
        void run_io_service();
        void start_receive();
        void handle_receive(const asio::error_code &error, std::size_t size);
        void handle_send(std::shared_ptr<std::string>, std::shared_ptr<HrpDataHeader> header, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb,
                         const asio::error_code &error, std::size_t transferred_size);
        void handle_forward(std::shared_ptr<HrpDataHeader> header);

        void scheduleTransfer(std::shared_ptr<HrpDataHeader> header, hrpNode nextHop, std::function<void(std::shared_ptr<HrpDataHeader>, bool)> cb);

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        /// For statistics recording
        int                     _total_originated_size;

        hrpNode                 _thisNode;
        unsigned short          _port;
        asio::io_service        &_io_service;
        asio::ip::udp::socket   _socket;
        asio::ip::udp::endpoint _remote_endpoint;
        std::vector<char>       _receive_buf;
        std::vector<char>       _send_buf;

        TopologyManager         *_topologyManager;

        std::thread             _thread_io_service;
        std::map<hrp::hrp_packet_type, std::vector<std::function<void(std::shared_ptr<HrpDataHeader>)> > > _handlers;
    };
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    class NeighborDiscoveryAgent {
    public:
        explicit NeighborDiscoveryAgent(const std::string& addr, asio::io_service &io_service) :
                _addr(addr),
                _io_service(io_service), _timer(NULL),
                _recv_socket(_io_service, asio::ip::udp::endpoint(asio::ip::address::from_string("224.0.0.1"), 8081)),
                _send_socket(_io_service, asio::ip::udp::v4()),
                _mcast_endpoint(asio::ip::address::from_string("224.0.0.1"), 8081),
                _receive_buf(HRP_TXRX_MAX_BUF_SIZE), _send_buf(1){
            _recv_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            _recv_socket.set_option(asio::ip::multicast::join_group(asio::ip::address::from_string("224.0.0.1")));
        };
        ~NeighborDiscoveryAgent() { delete(_timer); }
        void init();
        void registerListeners(const std::function<void(const hrp::hrpNode &)> &available, const std::function<void(const hrp::hrpNode &)> &unavailable) {
            _availableListeners.push_back(available);
            _unavailableListeners.push_back(unavailable);
        }

    private:
        void start_receive();
        void handle_send(const std::error_code &error, std::size_t size);
        void handle_receive(const std::error_code &error, std::size_t size);
        void scheduleHello(const std::error_code &error);
        void say_hello();
        void purge_peer();
    private:
        asio::io_service    &_io_service;
        asio::steady_timer  *_timer;
        asio::ip::udp::socket   _send_socket;
        asio::ip::udp::endpoint   _mcast_endpoint;
        asio::ip::udp::socket   _recv_socket;
        asio::ip::udp::endpoint _remote_endpoint;
        std::vector<char>       _receive_buf;
        std::vector<char>       _send_buf;

        std::map<std::string, std::chrono::steady_clock::time_point> _peers;

        std::vector<std::function<void(const hrp::hrpNode &)>> _availableListeners;
        std::vector<std::function<void(const hrp::hrpNode &)>> _unavailableListeners;

        std::string             _addr;
    };
}

#endif //HRP_TXRXENGINE_H
