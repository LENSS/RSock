//
// Created by Chen Yang on 8/30/17.
//

#include "MetaDataExchanger.h"

namespace hrp {

    void MetaDataExchanger::init() {

        // check if TxrxEngine is initialized
        if (_engine == NULL) {
            _logger->error("ERROR in init(), TxrxEngine is not set");
            exit(1);
        }

        if (_hrp_core == NULL) {
            _logger->error("ERROR in init(), HrpCore is not set");
            exit(1);
        }

        // register handlers
        _engine->registerHandler(meta_request, std::bind(&MetaDataExchanger::rxReceived, this, std::placeholders::_1));
        _engine->registerHandler(meta_response, std::bind(&MetaDataExchanger::rxReceived, this, std::placeholders::_1));

        // schedule timers
        _request_timer = new asio::steady_timer(_engine->get_io_service());
        if (_request_timer == NULL) {
            _logger->error("ERROR in init(), cannot initialize _request_timer");
            exit(1);
        }

        scheduleMveRequestTimer();
    }

//    void MetaDataExchanger::nodeAvailable(sockaddr_in &sa) {
//        hrpNode n = sock2node(sa);
//        nodeAvailable(n);
//    }

    void MetaDataExchanger::nodeAvailable(const hrpNode &n) {
        std::lock_guard<std::mutex> lg(_mutex_nodes);
        auto itr = _connected_nodes.find(n);
        if (itr == _connected_nodes.end()) {
            itr = _discovered_nodes.find(n);
            if (itr == _discovered_nodes.end()) {
                _discovered_nodes.insert(n);
                //scheduleMveRequest(n);
            }
        }
    }

//    void MetaDataExchanger::nodeUnavailable(sockaddr_in &sa) {
//        hrpNode n = sock2node(sa);
//        nodeUnavailable(n);
//    }

    void MetaDataExchanger::nodeUnavailable(const hrpNode &n) {
        std::lock_guard<std::mutex> lg(_mutex_nodes);
        auto itr = _discovered_nodes.find(n);
        if (itr == _discovered_nodes.end()) {
            itr = _connected_nodes.find(n);
            if (itr != _connected_nodes.end()) {
                _hrp_core->nodeDisconnected(n);
                _router->nodeDisconnected(n);
                _connected_nodes.erase(itr);
            }
        } else {
            _discovered_nodes.erase(itr);
        }
    }

    void MetaDataExchanger::scheduleMveRequest(const hrpNode &peer) {
        _logger->debug("scheduleMveRequest, schedule mve request to {}", peer);

        HrpDataHeader header(_hrp_core->get_thisNode(), peer, 1, meta_request);
        std::shared_ptr<HrpDataHeader> header_ptr = std::make_shared<HrpDataHeader>(header);
        _engine->scheduleTransfer(header_ptr, std::bind(&MetaDataExchanger::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));

    }

    void MetaDataExchanger::scheduleMveRequestTimer() {
        _request_timer->expires_from_now(std::chrono::seconds((long long)2));
        _request_timer->async_wait(std::bind(&MetaDataExchanger::scheduleMveRequestForDiscovered, this));
    }

    void MetaDataExchanger::scheduleMveRequestForDiscovered() {
        std::lock_guard<std::mutex> lg(_mutex_nodes);
        for (const hrpNode &node : _discovered_nodes)
            scheduleMveRequest(node);
        scheduleMveRequestTimer();
    }

    void MetaDataExchanger::rxReceived(std::shared_ptr<HrpDataHeader> header) {
        switch (header->get_type()) {
            case meta_request:
                _logger->debug("rxReceived, receive mve request from {}", header->get_src());
                processMveRequest(header);
                break;
            case meta_response:
                _logger->debug("rxReceived, receive mve response from ", header->get_src());
                processMveResponse(header);
                break;
            default:
                break;
        }
    }

    void MetaDataExchanger::processMveRequest(std::shared_ptr<HrpDataHeader> request) {
        hrpNode peer = request->get_src();
        std::string buf;

        HrpMveCoreMessage mveCoreMessage(_hrp_core->get_mveCore());
        HrpBloomFilterMessage filterMessage(_router->getFilter());
        hrp_message::HrpMetaData metaData;
        metaData.set_allocated_bloomfilter(&filterMessage.get_proto_msg());
        metaData.set_allocated_mvecore(&mveCoreMessage.get_proto_msg());

        metaData.SerializeToString(&buf);
        metaData.release_bloomfilter();
        metaData.release_mvecore();

        HrpDataHeader response(_hrp_core->get_thisNode(), peer, 1, meta_response);
        response.set_payload(buf.c_str(), buf.size() * sizeof(char));

        std::shared_ptr<HrpDataHeader> response_ptr = std::make_shared<HrpDataHeader>(response);

        _logger->debug("processMveRequest, reply with mve response to {}", peer);
        _engine->scheduleTransfer(response_ptr, std::bind(&MetaDataExchanger::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));
    }

    void MetaDataExchanger::processMveResponse(std::shared_ptr<HrpDataHeader> response) {
        hrpNode peer = response->get_src();
        std::lock_guard<std::mutex> lg(_mutex_nodes);
        auto itr = _connected_nodes.find(peer);
        if (itr != _connected_nodes.end()) {
            // we already have this hrpNode in the connected set, what should we do here?
            _logger->debug("processMveResponse, receive MveResponse from already connected node {}, ignored ", peer);

        } else {
            itr = _discovered_nodes.find(peer);
            if (itr != _discovered_nodes.end()) {
                // this hrpNode was in _discovered_nodes set, we should promote it
                hrp_message::HrpMetaData metaData;
                metaData.ParseFromArray(response->get_payload(), response->get_payload_size());

                HrpMveCoreMessage mveCoreMessage(peer, metaData.mvecore());
                HrpBloomFilterMessage filterMessage(metaData.bloomfilter());

                MVECore mveCore("tmp");
                mveCoreMessage.inflate_mvecore(mveCore);

                // promote the peer to _connected
                _discovered_nodes.erase(peer);
                _connected_nodes.insert(peer);

                // notify HrpCore
                _hrp_core->nodeConnected(peer, mveCore, filterMessage);
                _router->nodeConnected(peer);

            } else {
                // this hrpNode was not even in the _discovered_nodes set, something is wrong
                // just ignore it for now
                _logger->debug("processMveResponse, receive MveResponse from non-discovered node {}, ignored ", peer);
            }
        }
    }

    void MetaDataExchanger::TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success) {

    }


}