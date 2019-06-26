//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_HRPAPISERVER_H
#define HRP_HRPAPISERVER_H

#include <hrp/HrpRouter.h>
#include "api/HrpAPIPacket.h"
#include <asio.hpp>
#include "common.h"

namespace hrp
{

	class HrpClientConnection;

	class HrpAPIMultiplexer
	{
		public:
		explicit HrpAPIMultiplexer(HrpRouter &router) : _router(router)
		{
			_logger = spdlog::get("HrpAPIMultiplexer");
			if (!_logger)
				_logger = std::make_shared<spdlog::logger>("HrpAPIMultiplexer", file_sink);
			if (debug_mode)
				_logger->set_level(spdlog::level::debug);
		}
		
		void init();
		bool registerConnection(const std::string &name, std::shared_ptr<HrpClientConnection> conn);
		bool unregisterConnection(const std::string &name);

		// size_t send(const std::string &session, void *buf, size_t size);
		/**
		* Callback function for the router to notify this api that a data is received.
		* @param pkt
		*/
		void recv_cb(std::shared_ptr<HrpDataHeader> pkt);

		size_t send(const std::string &session, void *buf, size_t size, const hrpNode &dst, unsigned long ttl);

		private:
		// Logger
		std::shared_ptr<spdlog::logger> _logger;

		HrpRouter   &_router;
		std::map<std::string, std::shared_ptr<HrpClientConnection>> _connections;
	};

	class HrpClientConnection : public std::enable_shared_from_this<HrpClientConnection>
	{
		public:
		HrpClientConnection(asio::ip::tcp::socket sock, HrpAPIMultiplexer &multiplexer)
                : socket_(std::move(sock)), _multiplexer(multiplexer), _recv_buf(HRP_DEFAULT_DATA_BUF_SIZE), _send_buf(HRP_DEFAULT_DATA_BUF_SIZE)
		{
			_logger = spdlog::get("HrpClientConnection");
			if (!_logger)
				_logger = std::make_shared<spdlog::logger>("HrpClientConnection", file_sink);
			if (debug_mode)
				_logger->set_level(spdlog::level::debug);
		}
		void start();
		void recv_from_daemon(std::shared_ptr<HrpDataHeader> pkt);

		private:
		void do_read();
		void handle_receive(size_t size);
		void handle_register(const std::string &name);
		void handle_send(char *buf, const std::error_code &ec, size_t size);
		void recv_from_client(HrpAPIPacket &pkt);

		// Added by Ala
		void handle_topology_request(HrpAPIPacket &pkt);
		std::mutex  _mutex_table;
		// End Ala


		// Logger
		std::shared_ptr<spdlog::logger> _logger;
		std::string _app_name;
		HrpAPIMultiplexer &_multiplexer;
		asio::ip::tcp::socket socket_;
		std::vector<char>   _recv_buf;
		std::vector<char>   _send_buf;
	};

   
	class HrpAPIServer
	{
		public:
		HrpAPIServer(asio::io_service &io_service, unsigned short port, HrpRouter &router)
                : _io_service(io_service), _port(port), _multiplexer(router), acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), port)),
                socket_(io_service)
		{
			_logger = spdlog::get("HrpAPIServer");
			if (!_logger)
				_logger = std::make_shared<spdlog::logger>("HrpAPIServer", file_sink);
			if (debug_mode)
				_logger->set_level(spdlog::level::debug);
			router.registerCallback(std::bind(&HrpAPIServer::router_cb, this, std::placeholders::_1));
		}

		void init();

		private:
		void do_accept();
		void handle_accept(const std::error_code &err);
		void router_cb(std::shared_ptr<HrpDataHeader> pkt);

		// Logger
		std::shared_ptr<spdlog::logger> _logger;
		unsigned short _port;
		HrpAPIMultiplexer _multiplexer;
		asio::io_service &_io_service;
		asio::ip::tcp::acceptor acceptor_;
		asio::ip::tcp::socket socket_;
	};
}
#endif //HRP_HRPAPISERVER_H
