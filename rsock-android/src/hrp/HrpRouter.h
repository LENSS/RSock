//
// Created by Chen on 9/20/17.
//

#ifndef HRP_HRPSTORAGE_H
#define HRP_HRPSTORAGE_H

#include "hrpMessages.h"
#include "hrpNode.h"
#include "TxrxEngine.h"
#include "HrpCore.h"
#include <asio.hpp>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <bloom/bloom_filter.hpp>
#include <vector>
#include <map>

namespace hrp {

    class HrpRouter {

        enum hrp_buf_status{
            buffered = 0,
            scheduled = 1,
            outbound = 2,
            acked = 3
        };

        typedef struct hrp_buf_entry {
            std::shared_ptr<HrpDataHeader> pkt;
            hrp_buf_status status;
            std::chrono::time_point<std::chrono::system_clock> outbound_t;
        } HrpBufEntry;


    public:
        explicit HrpRouter(asio::io_service &io_service, HrpCore *core, TxrxEngine *engine)
                : _io_service(io_service), _core(core), _engine(engine), _update_timer(nullptr), _send_seq(0) {
            _logger = std::make_shared<spdlog::logger>("HrpRouter", file_sink);
                if (debug_mode)
                        _logger->set_level(spdlog::level::debug);
        }

        ~HrpRouter() { delete _update_timer; }

        void init();

        /// Add message to buffer
        std::shared_ptr<HrpDataHeader> newMessage(void *buf, size_t size, const hrpNode &dst, const std::string &app, unsigned long ttl);

        /// Transmission related call back functions
        /**
         * Callback function for scheduling transmission
         * @param pkt The packet that was scheduled
         * @param success If it was a success
         */
        void TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success);

        /**
         * Notify the HrpRouter that a trans. has started.
         * @param header The header of the message being transmitted
         */
        void txStarted(HrpDataHeader &header);

        /**
         * Notify the HRPCore that a trans. has finished successfully.
         * This means that the intended next carrier has received the packet.
         * HrpRouter should now modify the header with proper information.
         * @param header The header of the message having been transmitted
         */
        void txCompleted(HrpDataHeader &header);

        /**
         * Notify the HRPCore that a trans. was aborted, possibily due to
         * the break of a link. (What should the HRPCore do here?)
         * @param header The header of the message being aborted
         */
        void txAborted(HrpDataHeader &header);

        /**
         * Callback at the HrpRouter indicating that a message was successfully received.
         * If the message is a data message, HrpRouter should put it into
         * buffer, modify the header appropriately, and send back an ack.
         * If it is a ack, then we should call txCompleted().
         * @param header The header of the message received
         */
        void rxReceived(std::shared_ptr<HrpDataHeader> header);

        /// Node event callback

        /**
         * Some node is connected to the current mesh, a routing
         * decision process should start
         * @param n
         */
        void nodeConnected(const hrpNode &n);
        void nodeDisconnected(const hrpNode &n);

        /// API register callback
        void registerCallback(std::function<void(std::shared_ptr<HrpDataHeader>)> cb) {
            _recv_cb = cb;
        }

        // Self update methods
        void update(const std::error_code &error);
        void scheduleUpdateTimer();

        const bloom_filter &getFilter() const;

    private:
        /**
         * Traverse all the messages and check if there's any message can
         * be forwarded
         */
        void tryAllMessages();

        /**
         * Check the given message to see if it can be forwarded
         * @param pkt
         * @return Whether or not this pkt is scheduled for transfer
         */
        bool tryMessage(std::shared_ptr<HrpDataHeader> pkt);

        void processData(std::shared_ptr<HrpDataHeader> pkt);

        void processDataAck(std::shared_ptr<HrpDataHeader> pkt);

        /**
         * Add a new message to buffer. The packet should be well constructed.
         * Then this method will 1) create entry, 2) insert to buffer,
         * 3) insert it to bloomFilter, and 4) trigger a new routing decision making
         * @param pkt
         */
        void addMessageToBuffer(std::shared_ptr<HrpDataHeader> pkt);

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        asio::io_service    &_io_service;
        HrpCore *_core;
        TxrxEngine *_engine;
        asio::steady_timer  *_update_timer;

        bloom_filter filter;

        std::mutex  _mutex_buffer;
        std::map<std::string, HrpBufEntry> _buffer;

        std::function<void(std::shared_ptr<HrpDataHeader>)> _recv_cb;

        unsigned long _send_seq;
    };
}


#endif //HRP_HRPSTORAGE_H
