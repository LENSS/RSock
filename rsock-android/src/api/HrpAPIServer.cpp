//
// Created by Chen Yang on 9/28/17.
// Modified by Ala Altaweel
//

#include "HrpAPIServer.h"
#include "HrpAPIPacket.h"

namespace hrp {
    void HrpAPIServer::init() {
        do_accept();
    }

    void HrpAPIServer::do_accept() {
        acceptor_.async_accept(socket_, std::bind(&HrpAPIServer::handle_accept, this, std::placeholders::_1));
    }

    void HrpAPIServer::handle_accept(const std::error_code &err) {
        if (!err) {
            std::make_shared<HrpClientConnection>(std::move(socket_), _multiplexer)->start();
            do_accept();
        } else
            _logger->debug("ERROR, reason: {}", err.message());
    }

    void HrpClientConnection::start() {
        do_read();
    }

    void HrpClientConnection::do_read() {
        auto self(shared_from_this());

        asio::async_read(socket_, asio::buffer(_recv_buf, sizeof(uint64_t)), [this, self](std::error_code ec, std::size_t size)
        {
           if (!ec) {
               uint64_t sz_of_data = 0;
               memcpy(&sz_of_data, _recv_buf.data(), sizeof(uint64_t));
               //_logger->info("sz_of_data= {}", sz_of_data,);

               asio::async_read(socket_, asio::buffer(_recv_buf, sz_of_data), [this, self](std::error_code error_code, std::size_t sz){
                    if (!error_code) {
                        handle_receive(sz);
                    } else {
                        _logger->error("ERROR when receiving data body, reason: {}", error_code.message());
                    }
               });
           } else {
               if (ec == asio::error::eof) {
                   _logger->info("client closed connection");
                   if (!_app_name.empty())
                       if (_multiplexer.unregisterConnection(_app_name))
                           _logger->info("unregistered from multiplexer");
               }
               else _logger->debug("ERROR, reason: {}", ec.message());
           }
        });
    }

    void HrpAPIServer::router_cb(std::shared_ptr<HrpDataHeader> pkt) {
        _multiplexer.recv_cb(pkt);
    }

    void HrpClientConnection::handle_receive(size_t size) {
        HrpAPIPacket packet = HrpAPIPacketHelper::parseFromBuffer(_recv_buf.data(), size);
        switch (packet.getType()) {
            case api_register:
                handle_register(std::string(packet.getPayload(), packet.getSz_of_payload()));
                break;
            case api_data:
                recv_from_client(packet);
                break;
            default:
                _logger->debug("ERROR, unknown command received from client: {}", _recv_buf[0]);
        }

        do_read();
    }

    void HrpClientConnection::handle_register(const std::string &name) {
        if (_app_name.empty()) {
            if (_multiplexer.registerConnection(name, shared_from_this())) {
                _logger->info("app {} registered", name);
                _app_name = name;
            } else
                _logger->error("fail to register app {} to multiplexer", name);
        } else
            _logger->error("app {} was already registered", name);
    }

    void HrpClientConnection::recv_from_client(HrpAPIPacket &pkt) {
        if (_app_name.empty()) {
            _logger->error("no app have registered yet, the received data is dropped");
            return;
        }

        _multiplexer.send(_app_name, pkt.getPayload(), pkt.getSz_of_payload(), pkt.getNode(), pkt.getTime());

    }

    void HrpClientConnection::recv_from_daemon(std::shared_ptr<HrpDataHeader> pkt) {

//        auto *buf = new char[sizeof(int32_t) + sizeof(uint64_t) + pkt->get_payload_size()];
//        int32_t src_ip_raw = node_to_ip(pkt->get_src());

        int64_t delay = ms_from_epoch() - pkt->get_gen_time();

//        memcpy(buf, &src_ip_raw, sizeof(src_ip_raw));
//        memcpy(buf + sizeof(src_ip_raw), &delay, sizeof(delay));
//        memcpy(buf + sizeof(src_ip_raw) + sizeof(delay), pkt->get_payload(), pkt->get_payload_size());

        uint32_t out_size;
        auto *buf = HrpAPIPacketHelper::dataPacket(pkt->get_src(), delay, pkt->get_payload_size(), (const char *) pkt->get_payload(), out_size);

        asio::async_write(socket_, asio::buffer(buf, out_size),
                                 std::bind(&HrpClientConnection::handle_send, this, buf,
                                           std::placeholders::_1, std::placeholders::_2));

    }

    void HrpClientConnection::handle_send(char *buf, const std::error_code &ec, size_t size) {
        if (ec) {
            _logger->error("ERROR, reason: {}", ec.message());
        }
        delete []buf;
    }


    void HrpAPIMultiplexer::init() {
        _router.registerCallback(std::bind(&HrpAPIMultiplexer::recv_cb, this, std::placeholders::_1));
    }

    bool HrpAPIMultiplexer::registerConnection(const std::string &name, std::shared_ptr<HrpClientConnection> conn) {
        if (_connections.count(name) != 0)
            return false;
        _connections.insert(std::pair<std::string, std::shared_ptr<HrpClientConnection>>(name, conn));
        return true;
    }

    bool HrpAPIMultiplexer::unregisterConnection(const std::string &name) {
        if (_connections.count(name) == 0)
            return false;
        _connections.erase(name);
        return true;
    }

//    size_t HrpAPIMultiplexer::send(const std::string &session, void *buf, size_t size) {
//        uint32_t ip_raw = *((uint32_t *)buf);
//        uint64_t ttl = *((uint64_t *)((char *)buf + sizeof(ip_raw)));
//        hrpNode dst = ip_to_node(ip_raw);
//        size_t hdr_size = sizeof(ip_raw) + sizeof(ttl);
//        return send(session, ((char *)buf) + hdr_size, size - hdr_size, dst, ttl);
//    }

    void HrpAPIMultiplexer::recv_cb(std::shared_ptr<HrpDataHeader> pkt) {
        std::string app = pkt->get_app();
        auto con = _connections.find(app);
        if (con == _connections.end()) {
            _logger->info("received data for unregistered app {}", app);
            return;
        }

        con->second->recv_from_daemon(pkt);

    }

    size_t
    HrpAPIMultiplexer::send(const std::string &session, void *buf, size_t size, const hrpNode &dst, unsigned long ttl) {
        std::shared_ptr<HrpDataHeader> pkt = _router.newMessage(buf, size, dst, session, ttl);
        if (pkt) return pkt->get_payload_size();
        return 0;
    }

}
