//
// Created by Chen on 9/29/17.
//

#include "HrpAPIClient.h"
#include "HrpAPIPacket.h"

namespace hrp {
    HrpAPIClient::HrpAPIClient(const std::string &app) : _app_name(app), _socket(_io_service), _recv_buf(HRP_DEFAULT_DATA_BUF_SIZE) {
        assert(!app.empty());
        do_connect();
    }

    HrpAPIClient::~HrpAPIClient() {

        if (!_io_service.stopped())
            _io_service.stop();

        if (_socket.is_open()) {
            _socket.close();
        }

        _thd.join();

    }

    bool HrpAPIClient::init() {
        _thd = std::thread([this](){ _io_service.run(); });

        std::unique_lock<std::mutex> lk(_mutex);
        _cv.wait(lk);
        lk.unlock();

        if (_isConnected) {
            std::cout << "connected, registering app" << std::endl;
            register_app();
        } else
            std::cout << "not connected" << std::endl;

        lk.lock();
        _cv.wait(lk);
        lk.unlock();

        return _isConnected;
    }

    void HrpAPIClient::do_connect() {
        _socket.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), HRP_DEFAULT_API_PORT),
                              [this](const std::error_code &ec) {
                                  {
                                      std::lock_guard<std::mutex> lk(_mutex);
                                      _isConnected = ec ? false : true;
                                  }
                                  if (ec) std::cout << ec.message() << std::endl;
                                  else {
                                      std::cout << "connected to daemon" << std::endl;
                                      do_receive();
                                  }
                                  _cv.notify_all();
                              });
    }

    void HrpAPIClient::do_receive() {
        asio::async_read(_socket, asio::buffer(_recv_buf, sizeof(uint64_t)),
                                std::bind(&HrpAPIClient::handle_receive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void HrpAPIClient::handle_receive(const std::error_code &ec, size_t size) {
        if (!ec) {
//	    std::cout << "HrpAPIClient::handle_receive() Start ";

            uint64_t sz_of_data;
            memcpy(&sz_of_data, _recv_buf.data(), sizeof(sz_of_data));

            asio::async_read(_socket, asio::buffer(_recv_buf, sz_of_data),
                             [this, sz_of_data](const std::error_code &error_code, size_t size) {
                                 auto *buf = new char[size];
                                 memcpy(buf, _recv_buf.data(), size);

                                 HrpAPIPacket packet = HrpAPIPacketHelper::parseFromBuffer(buf, size);

                                 {
                                     std::lock_guard<std::mutex> lk(_mutex);
                                     _queue.push(packet);
                                 }
                                 _cv.notify_one();

                                 do_receive();
                             });
  //         std::cout << "HrpAPIClient::handle_receive() After asio::async_read... ";

        } else
            std::cout << "HrpAPIClient::handle_receive error, " << ec.message() << std::endl;
    }


long HrpAPIClient::recv(void *buf, size_t max_size, hrpNode &src, long &delay) {
//	std::cout << "HrpAPIClient::recv() Start ";
        std::unique_lock<std::mutex> lk(_mutex);
        _cv.wait(lk, [this]{ return !_queue.empty();});

        HrpAPIPacket &pkt = _queue.front();
        if (pkt.getSz_of_payload() > max_size) {
            delete []pkt.getPayload();
            return -1;
        }

        memcpy(buf, pkt.getPayload(), pkt.getSz_of_payload());
        src = pkt.getNode();
        delay = pkt.getTime();

        delete []pkt.getPayload();
        _queue.pop();
        lk.unlock();
//      std::cout << "HrpAPIClient::recv() End ";

        return pkt.getSz_of_payload();
    }



    void HrpAPIClient::do_send(char *buf, size_t size) {
//	std::cout<< "before HrpAPIClient::do_send\n";


	///Added by Ala to send many buffers
	/*
	std::vector<asio::const_buffer> buffers;
	buffers.push_back(asio::const_buffer(buf, 65300));
        buffers.push_back(asio::const_buffer(buf, 65300));
        buffers.push_back(asio::const_buffer(buf, 65300));
        buffers.push_back(asio::const_buffer(buf, 65300));



	asio::async_write(_socket, buffers, [this, buffers](const std::error_code &ec, size_t sz) {
           if (ec) {
               std::cout <<"HrpAPIClient::do_send, error in sending, error: " << ec.message() << std::endl;
           }
        });
        std::cout<< "after HrpAPIClient::do_send\n";

	///End of Added code by Ala

	*/

	// below is commented out by Ala to send many buffers (i.e., the code above)	
        asio::async_write(_socket, asio::buffer(buf, size), [this, buf](const std::error_code &ec, size_t sz) {
           if (ec) {
               std::cout <<"HrpAPIClient::do_send, error in sending, error: " << ec.message() << std::endl;
           }
            delete[] buf;
        });
//      std::cout<< "after HrpAPIClient::do_send\n";
	
    }

    long HrpAPIClient::send(char *buf, size_t size, const hrpNode &dst, unsigned long ttl) {
        uint32_t buf_sz;
        auto *to_send = HrpAPIPacketHelper::dataPacket(dst, ttl, size, buf, buf_sz);
        if (buf_sz > HRP_DEFAULT_DATA_BUF_SIZE) {
            std::cout << "HrpAPIClient::send cannot send data larger than " << HRP_DEFAULT_DATA_BUF_SIZE << " bytes\n";
            return -1;
        }

        do_send(to_send, buf_sz);
        return buf_sz;
    }

    void HrpAPIClient::register_app() {
        uint32_t size;
        auto *buf = HrpAPIPacketHelper::registerPacket(_app_name, size);

        asio::async_write(_socket, asio::buffer(buf, size), [this, buf](const std::error_code &ec, size_t sz) {
            if (ec) {
                std::cout <<"HrpAPIClient::register_app, error in sending, error: " << ec.message() << std::endl;
            }
            delete[] buf;
            _cv.notify_all();
        });
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    OlsrTestAPI::OlsrTestAPI(asio::io_service &io_service, int msgInterval, int nrofMsg, int size, hrpNode dst) :
            _io_service(io_service), _port(17777), _receive_buf(HRP_DEFAULT_DATA_BUF_SIZE), _send_buf(HRP_DEFAULT_DATA_BUF_SIZE),
            _socket(_io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), _port)), _msgInterval(msgInterval), _nrofMsg(nrofMsg),
            _current_msg(0), _size(size), _dst(dst)
    {

    }

    void OlsrTestAPI::init() {
        _olsr_timer = new asio::steady_timer(_io_service);
        if (_olsr_timer == nullptr) {
            std::cout << "init, cannot initialize _olsr_timer" << std::endl;
            exit(1);
        }

        scheduleNextTransfer();
        do_receive();
    }

    void OlsrTestAPI::do_receive() {
        _socket.async_receive_from(asio::buffer(_receive_buf),
                                   _remote_endpoint,
                                   std::bind(&OlsrTestAPI::handle_receive,
                                             this, std::placeholders::_1, // error code
                                             std::placeholders::_2)); // bytes_transferred 
    }

    void OlsrTestAPI::scheduleNextTransfer() {
        if (_nrofMsg > _current_msg) {
            _olsr_timer->expires_from_now(std::chrono::milliseconds(_msgInterval));
            _olsr_timer->async_wait(std::bind(&OlsrTestAPI::transferData, this, std::placeholders::_1));
        }
    }

    void OlsrTestAPI::transferData(const std::error_code &error) {
        if (!error) {
            memcpy(_send_buf.data(), &_current_msg, sizeof(_current_msg));
            _track_table.insert(std::pair<int, long>(_current_msg, ms_from_epoch()));
            _socket.async_send_to(asio::buffer(_send_buf.data(), _size),
                                  asio::ip::udp::endpoint(asio::ip::address::from_string(_dst), _port),
                                  std::bind(&OlsrTestAPI::handle_send,
                                            this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));


            _current_msg++;
            scheduleNextTransfer();
        } else
            std::cout << "transferData ERROR: " << error.message() << std::endl;
    }

    void OlsrTestAPI::handle_receive(const std::error_code &error, std::size_t size) {
        if (!error) {
            if (_remote_endpoint.address().to_string() == _dst) {
                // this is an ack
                int seq;
                memcpy(&seq, _receive_buf.data(), sizeof(seq));
                auto itr = _track_table.find(seq);
                if (itr != _track_table.end()) {
                    auto now_time = ms_from_epoch();
                    long rtt = now_time - itr->second;
                    std::cout << "received ack for " << seq << " delay " << rtt/2 << " ms" << std::endl;
                    _track_table.erase(itr);

                } else {
                    std::cout << "received a duplicate ack" << std::endl;
                }
            } else {
                // this is a data

                std::cout << "received data from " << _remote_endpoint.address().to_string() << std::endl;
                int seq;
                memcpy(&seq, _receive_buf.data(), sizeof(seq));
                memcpy(_send_buf.data(), &seq, sizeof(seq));

                _socket.async_send_to(asio::buffer(_send_buf.data(), size),
                                      _remote_endpoint,
                                      std::bind(&OlsrTestAPI::handle_send,
                                                this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
            }

            do_receive();
        } else
            std::cout << "handle_receive ERROR: " << error.message() << std::endl;
    }

    void OlsrTestAPI::handle_send(const std::error_code &error, std::size_t transferred_size) {
        if (!error) {

        } else
            std::cout << "handle_send ERROR: " << error.message() << std::endl;
    }
*/
}


