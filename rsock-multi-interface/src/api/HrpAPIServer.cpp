//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "HrpAPIServer.h"
#include "HrpAPIPacket.h"
#include "common.h"
namespace hrp
{
	void HrpAPIServer::init()
	{
		do_accept();
	}

	void HrpAPIServer::do_accept()
	{
		acceptor_.async_accept(socket_, std::bind(&HrpAPIServer::handle_accept, this, std::placeholders::_1));
	}

	void HrpAPIServer::handle_accept(const std::error_code &err)
	{
		if (!err)
		{
			std::make_shared<HrpClientConnection>(std::move(socket_), _multiplexer)->start();
			do_accept();
		}
		else
			_logger->info("ERROR, reason: {}", err.message());
	}


	void HrpClientConnection::start()
	{
		do_read();
	}

	void HrpClientConnection::do_read()
	{
		auto self(shared_from_this());
		asio::async_read(socket_, asio::buffer(_recv_buf, sizeof(uint64_t)), [this, self](std::error_code ec, std::size_t size)
		{
			if (!ec)
			{
				uint64_t sz_of_data = 0;
				memcpy(&sz_of_data, _recv_buf.data(), sizeof(uint64_t));
				//_logger->info("read data with size= {}", sz_of_data,);
				asio::async_read(socket_, asio::buffer(_recv_buf, sz_of_data), [this, self](std::error_code error_code, std::size_t sz)
				{
					if (!error_code)
					{
						handle_receive(sz);
					}
					else
					{
						_logger->error("ERROR when receiving data body, reason: {}", error_code.message());
					}
				});
			}
			else
			{
				if (ec == asio::error::eof)
				{
					_logger->info("client closed connection");
					if (!_app_name.empty())
						if (_multiplexer.unregisterConnection(_app_name))
							_logger->info("unregistered from multiplexer");
				}
				else
					_logger->debug("ERROR, reason: {}", ec.message());
			}
		});
	}


	void HrpAPIServer::router_cb(std::shared_ptr<HrpDataHeader> pkt)
	{
		_multiplexer.recv_cb(pkt);
	}


	void HrpClientConnection::handle_receive(size_t size)
	{
		HrpAPIPacket packet = HrpAPIPacketHelper::parseFromBuffer(_recv_buf.data(), size);
		switch (packet.getType())
		{
			case api_register:
				handle_register(std::string(packet.getPayload(), packet.getSz_of_payload()));
			break;
			case api_data:
				recv_from_client(packet);
			break;
			// added by Ala
			case api_topology:
				handle_topology_request(packet);
			break;
		
			// End Ala
			default:
				_logger->error("unknown command received from client: {}", _recv_buf[0]);
		}
		do_read();
	}


	// Added by Ala
	void HrpClientConnection::handle_topology_request(HrpAPIPacket &pkt)
	{
		_logger->info("handle_topology_request called");
		//_logger->info("handle_topology_request send back to: {}", printHrpNode("",pkt.getNode()));

		int64_t delay = 0;
		uint32_t out_size;
		string jDocStr;

		// create an object of gns client
		hrp::GnsServiceClient gnsObj;
		hrpNode hrpNodeObj;
		// get GUID by IP
		string ownGUID =  gnsObj.getOwnGUID();

		if(ownGUID.size() != 0)			
		{	
			hrpNodeObj.push_back(ownGUID);
		}
		else
		{
			cout<<"no GUID returned from getOwnGUID() function"<<endl;			
		}



		{
			std::lock_guard<std::mutex> lck (_mutex_table);
			// assign the global json object, guidDoc, into string
			rapidjson::StringBuffer buffer;

			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			guidDoc.Accept(writer);
			jDocStr = buffer.GetString();
			_logger->debug("sent json with GUIDSs: {}", jDocStr);
		}
		
		
		// send guidDoc 
		auto *buf = HrpAPIPacketHelper::replyTopologyPacket(hrpNodeObj, delay, jDocStr.size(), (const char *) jDocStr.c_str(), out_size);
		asio::async_write(socket_, asio::buffer(buf, out_size),
		std::bind(&HrpClientConnection::handle_send, this, buf,
		std::placeholders::_1, std::placeholders::_2));
		
	}
	// End Ala
	
	void HrpClientConnection::handle_register(const std::string &name)
	{
		if (_app_name.empty())
		{
			if (_multiplexer.registerConnection(name, shared_from_this()))
			{
				_logger->info("app {} registered", name);
				_app_name = name;
			}
			else
				_logger->error("fail to register app {} to multiplexer", name);
		}
		else
			_logger->error("app {} was already registered", name);
	}


	void HrpClientConnection::recv_from_client(HrpAPIPacket &pkt)
	{
		if (_app_name.empty())
		{
			_logger->error("no app have registered yet, the received data is dropped");
			return;
		}

		_multiplexer.send(_app_name, pkt.getPayload(), pkt.getSz_of_payload(), pkt.getNode(), pkt.getTime());

	}


	void HrpClientConnection::recv_from_daemon(std::shared_ptr<HrpDataHeader> pkt)
	{

		try
		{

			//_logger->debug("recv_from_daemon starts");

			// Added by Ala for debugging and packets tracking, you can comment the whole block
			// create an object of gns client
			/*
			hrp::GnsServiceClient gnsObj;
			hrpNode hrpNodeObj;

			string ownGUID =  gnsObj.getOwnGUID();
			if(ownGUID.size() != 0)			
			{	
				hrpNodeObj.push_back(ownGUID);
			}
			else
			{
				cout<<"no GUID returned from getOwnGUID() function"<<endl;			
			}
			if (pkt->get_dst_by_guid() == hrpNodeObj)
				_logger->debug("packet received for app and sent to it");
			*/
			
			// End Ala
			int64_t delay = s_from_epoch() - pkt->get_gen_time();
			uint32_t out_size;
			auto *buf = HrpAPIPacketHelper::dataPacket(pkt->get_src_by_guid(), delay, pkt->get_payload_size(), (const char *) pkt->get_payload(), out_size);
			asio::async_write(socket_, asio::buffer(buf, out_size),
			std::bind(&HrpClientConnection::handle_send, this, buf,
			std::placeholders::_1, std::placeholders::_2));
		}
		catch (hrpException &e)
		{
			_logger->error("HrpClientConnection::recv_from_daemon cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}
	}


	void HrpClientConnection::handle_send(char *buf, const std::error_code &ec, size_t size)
	{
		if (ec)
		{
			_logger->error("ERROR, reason: {}", ec.message());
		}
		delete []buf;
	}


	
	void HrpAPIMultiplexer::init()
	{
		_router.registerCallback(std::bind(&HrpAPIMultiplexer::recv_cb, this, std::placeholders::_1));
	}


	bool HrpAPIMultiplexer::registerConnection(const std::string &name, std::shared_ptr<HrpClientConnection> conn)
	{
		if (_connections.count(name) != 0)
			return false;
		_connections.insert(std::pair<std::string, std::shared_ptr<HrpClientConnection>>(name, conn));
		return true;
	}

	bool HrpAPIMultiplexer::unregisterConnection(const std::string &name)
	{
		_logger->debug("unregisterConnection called for {}", name);
		if (_connections.count(name) == 0)
			return false;
		_connections.erase(name);
		return true;
	}


	void HrpAPIMultiplexer::recv_cb(std::shared_ptr<HrpDataHeader> pkt)
	{
		std::string app = pkt->get_app();
		auto con = _connections.find(app);
		if (con == _connections.end())
		{
			// commented and replaced by Ala
			/*
			_logger->error("received data for unregistered app {}", app);
			return;
			*/		
			_logger->warn("received data for unregistered/unconnected app: {}", app);
			//_logger->info("However, push the data and wait until the app is connected back!");
			// for now we returned, but what should we do later to handle this
			return;
			// End Ala
	
		}
		con->second->recv_from_daemon(pkt);
	}

	size_t HrpAPIMultiplexer::send(const std::string &session, void *buf, size_t size, const hrpNode &dst, unsigned long ttl)
	{
		//_logger->info("HrpAPIMultiplexer::send");
		//_logger->info(printHrpNode("HrpAPIMultiplexer::send ", dst));
		_logger->info("HrpAPIMultiplexer::send to {} size = {}, ttl = {}", printHrpNode("HrpAPIMultiplexer::send ", dst), size, ttl);
		std::shared_ptr<HrpDataHeader> pkt = _router.newMessage(buf, size, dst, session, ttl);
		if (pkt)
			return pkt->get_payload_size();
		return 0;
	}

}
