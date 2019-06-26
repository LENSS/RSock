//
// Created by Chen on 9/29/17.
//

#ifndef HRP_HRPAPICLIENT_H
#define HRP_HRPAPICLIENT_H

#include "common.h"
#include "hrpNode.h"
#include "HrpAPIPacket.h"
#include "hrpMessages.h"
#include <asio.hpp>
#include <thread>
#include <condition_variable>
#include <spdlog/spdlog.h>
#include <queue>

namespace hrp {



    class HrpAPIClient {
    public:

        explicit HrpAPIClient(const std::string &app);

        ~HrpAPIClient();

        bool init();

        /**
         * Async send function. Push the data into hrp router.
         * @param buf The buffer address
         * @param size Size of the data
         * @param dst Destination of the data
         * @param ttl Time-to-live of the data
         * @return
         */
        long send(char *buf, size_t size, const hrpNode &dst, unsigned long ttl);

        /**
         * Sync call of the reception function. Will block until a data is received.
         * @param buf   The receiving buffer
         * @param max_size  The size of the buffer
         * @param src   The source of the data
         * @param delay The delay of the data (for statistic purpose)
         * @return  The size of the data being read
         */
        long recv(void *buf, size_t max_size, hrpNode &src, long &delay);


    private:
        void register_app();
        void do_connect();
        void do_receive();
        void do_send(char *buf, size_t size);

        void handle_receive(const std::error_code &ec, size_t size);

    private:
        asio::io_service _io_service;
        asio::ip::tcp::socket _socket;
        std::thread _thd;

        std::string _app_name;

        std::condition_variable _cv;
        std::mutex  _mutex;
        bool    _isConnected;

        std::vector<char>   _recv_buf;
        std::queue<HrpAPIPacket>   _queue;
    };


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
    class OlsrTestAPI {
    public:
        OlsrTestAPI(asio::io_service &io_service, int msgInterval, int nrofMsg, int size, hrpNode dst);
        void init();

        void do_receive();

        void scheduleNextTransfer();
        void transferData(const std::error_code &error);


    private:
        void handle_receive(const std::error_code &error, std::size_t size);
        void handle_send(const std::error_code &error, std::size_t transferred_size);

    private:
        unsigned short          _port;
        asio::io_service        &_io_service;
        asio::ip::udp::socket   _socket;
        asio::ip::udp::endpoint _remote_endpoint;
        std::vector<char>       _receive_buf;
        std::vector<char>       _send_buf;

        hrpNode _dst;

        int _msgInterval;
        int _nrofMsg;
        int _current_msg;
        int _size;

        std::map<int, long> _track_table;

        // Timers
        asio::steady_timer  *_olsr_timer;    /// Regret-min timer for PI learner
    };

*/
}



#endif //HRP_HRPAPICLIENT_H
