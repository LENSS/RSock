//
// Created by Chen on 7/24/17.
//

#ifndef HRP_HRPMESSAGES_H
#define HRP_HRPMESSAGES_H

#include <mve/MVECore.h>
#include "common.h"
#include "hrpNode.h"
#include "hrp/HrpGraph.h"
#include "proto/HrpMsg.pb.h"
#include "bloom/bloom_filter.hpp"


namespace hrp {

    enum hrp_packet_type{
        pi_probe_request = 0,
        pi_probe_response = 1,
        meta_request = 3,
        meta_response = 4,
        data = 5,
        data_ack = 6
    };

    /**
     * Probe message header metadata, for evaluating a particular repl. factor
     */
    typedef struct {
        unsigned int seq;   /// sequence number
        unsigned short repl;  /// replication factor
    } pi_probe_meta;

    /**
     * Probe message response received from the peer.
     */
    typedef struct {
        unsigned int seq;   /// sequence number
        unsigned short repl;  /// replication factor
    } pi_response_tx;

    /**
     * Probe message response received from the peer.
     */
    typedef struct {
        pi_response_tx meta;
        double latency;
    } pi_response_local;

    /**
     * Main class for HRP message header
     */
    class HrpDataHeader {
    public:

        /// Constructor
        HrpDataHeader(const hrpNode &src, const hrpNode &dst, unsigned int repl, hrp_packet_type type) {
            _proto_msg.set_src(node_to_ip(src));
            _proto_msg.set_dst(node_to_ip(dst));
            _proto_msg.set_remain_repl(repl);
            _proto_msg.set_type(type);
        }

        HrpDataHeader(const hrpNode &src, const hrpNode &dst, unsigned int repl, hrp_packet_type type, unsigned int seq) {
            int src_ip = node_to_ip(src);

            _proto_msg.set_src(src_ip);
            _proto_msg.set_dst(node_to_ip(dst));
            _proto_msg.set_remain_repl(repl);
            _proto_msg.set_type(type);
            _proto_msg.set_current_carrier(src_ip);
            _proto_msg.set_next_carrier(node_to_ip(dst));

            _proto_msg.set_ttl(HRP_DEFAULT_PKT_TTL);
            _proto_msg.set_seqnum(seq);
            _proto_msg.set_gen_time(ms_from_epoch());

            int64_t hash = src_ip;
            hash = (hash << 32) | seq;
            _proto_msg.set_hash(hash);
        }

        /// Constructor
        explicit HrpDataHeader(const std::string &data) {
            _proto_msg.ParseFromString(data);
        }

        /// Copy constructor
        HrpDataHeader(const HrpDataHeader &header) : _carriers(header._carriers) {
            _proto_msg.CopyFrom(header._proto_msg);
        }

        bool SerializeToString(std::string* output) const {
            return _proto_msg.SerializeToString(output);
        }

        bool ParseFromString(const std::string& data) {
            return _proto_msg.ParseFromString(data);
        }

        /// Utility methods
        void add_carriers(const hrpNode &new_carrier) {
            if (_carriers.insert(new_carrier).second)
                _proto_msg.add_carriers(node_to_ip(new_carrier));
        }

        /// Setters
        void set_current_carrier(const hrpNode &_current_carrier) {
            _proto_msg.set_current_carrier(node_to_ip(_current_carrier));
        }

        void set_next_carrier(const hrpNode &_next_carrier) {
            _proto_msg.set_next_carrier(node_to_ip(_next_carrier));
        }

        void set_remain_repl(unsigned int _remain_repl) {
            _proto_msg.set_remain_repl(_remain_repl);
        }

        void set_payload(const void *_pkt, size_t size) {
            _proto_msg.set_payload(_pkt, size);
        }

        void set_routes(const HrpGraph::succMap &routes) {
            auto m = _proto_msg.mutable_routes();
            for (auto itr : routes) {
                hrp_message::HrpPacket::NextHop hops;
                for (const hrpNode &hop : itr.second) hops.add_hops(node_to_ip(hop));
                (*m)[node_to_ip(itr.first)] = hops;
            }
        }

        void set_ttl(int ttl) { _proto_msg.set_ttl(ttl); }

        void set_seqnum(unsigned long seqnum) { _proto_msg.set_seqnum(seqnum); }

        // void set_hash(const std::string& h) { _proto_msg.set_hash(h); }

        void set_app(const std::string& app) { _proto_msg.set_app(app); }

        /// Hash
        std::string get_hash() const {
            int64_t hash = _proto_msg.hash();
            int src_ip = ((hash >> 32) & 0xFFFFFFFF);
            unsigned int seq = hash & 0xFFFFFFFF;
            return std::to_string(src_ip) + "-" + std::to_string(seq);
        }

        /// Getters
        hrpNode get_src() const { return ip_to_node(_proto_msg.src()); }
        hrpNode get_dst() const { return ip_to_node(_proto_msg.dst()); }
        hrpNode get_current_carrier() const { return ip_to_node(_proto_msg.current_carrier()); }
        hrpNode get_next_carrier() const { return ip_to_node(_proto_msg.next_carrier()); }
        const ect_set &get_carriers() const { return _carriers; }
        unsigned long get_ttl() const { return _proto_msg.ttl(); }
        long get_gen_time() const { return _proto_msg.gen_time(); }
        unsigned int get_remain_repl() const { return _proto_msg.remain_repl(); }
        const void* get_payload() const { return _proto_msg.payload().c_str(); }
        const hrp_packet_type get_type() const { return hrp_packet_type(_proto_msg.type()); }
        const size_t get_payload_size() { return _proto_msg.payload().size(); }
        std::set<hrpNode> get_next_hop_for(const hrpNode &node) {
            std::set<hrpNode> ret;
            int node_raw_ip = node_to_ip(node);
            auto m = _proto_msg.routes();
            if (m.count(node_raw_ip) != 0) {
                hrp_message::HrpPacket::NextHop n = m[node_raw_ip];
                for (int i=0; i<n.hops_size(); i++) ret.insert(ip_to_node(n.hops(i)));
            }
            return ret;
        }
        unsigned long get_seqnum() const { return _proto_msg.seqnum(); }
        std::string get_app() const { return _proto_msg.app(); }

    private:
        ect_set _carriers;          /// All carriers that carry this message
        hrp_message::HrpPacket _proto_msg;
    };

    class HrpMveCoreMessage {
    public:
        explicit HrpMveCoreMessage(const MVECore &core) : _thisNode(core.get_thisNode()) {
            for (auto itr : core.get_params()) {
                hrp_message::MveCoreMessage_EctSet *ectSet = _proto_msg.add_keys();
                for (auto n : itr.first) ectSet->add_node(n);
                double val = itr.second;
                _proto_msg.add_vals(val);
            }
        }

        HrpMveCoreMessage(const hrpNode &node, const void *buf, int size) {
            _proto_msg.ParseFromArray(buf, size);
            _thisNode = node;
        }

        HrpMveCoreMessage(const hrpNode &node, const hrp_message::MveCoreMessage &msg)
                : _thisNode(node), _proto_msg(msg) {}

        /**
         * Fill the string buffer
         * @param str The buffer to fill
         */
        void inflate_payload(std::string *str) {
            _proto_msg.SerializeToString(str);
        }

        /**
         * Inflate the MVECore from the _proto_msg this object is holding
         * @param core Output parameter, the MVECore object to inflate
         */
        void inflate_mvecore(MVECore &core) {
            core.set_thisNode(_thisNode);
            for (int i=0; i<_proto_msg.keys_size(); i++) {
                const hrp_message::MveCoreMessage_EctSet &ectSetMsg = _proto_msg.keys(i);
                std::set<hrpNode> ectSet;
                for (int j=0; j<ectSetMsg.node_size(); j++) {
                    ectSet.insert(ectSetMsg.node(j));
                }
                double val = _proto_msg.vals(i);
                core.updateParam(ectSet, val);
            }
        }

        const hrp_message::MveCoreMessage &get_proto_msg() const {
            return _proto_msg;
        }

        hrp_message::MveCoreMessage &get_proto_msg() { return _proto_msg; }

    private:
        hrp_message::MveCoreMessage _proto_msg;
        hrpNode _thisNode;
    };

    class HrpBloomFilterMessage : public bloom_filter {
    public:
        explicit HrpBloomFilterMessage(const std::string& buf);
        explicit HrpBloomFilterMessage(const hrp_message::BloomFilter &msg);
        HrpBloomFilterMessage(const bloom_filter& filter);
        HrpBloomFilterMessage(const HrpBloomFilterMessage& msg);
        void inflate_payload(std::string *str) {
            _proto_msg.SerializeToString(str);
        }

        const hrp_message::BloomFilter &get_proto_msg() const;
        hrp_message::BloomFilter &get_proto_msg() { return _proto_msg; }

    protected:
        void init_from_proto_msg();

    private:
        hrp_message::BloomFilter _proto_msg;
    };
}

#endif //HRP_HRPMESSAGES_H
