//
// Created by Chen on 10/2/17.
//

#include <assert.h>
#include "HrpAPIPacket.h"


namespace hrp {
    char *HrpAPIPacketHelper::registerPacket(const std::string &name, uint32_t &out_size) {
        assert(!name.empty());
        uint64_t sz_of_data = sizeof(char) + sizeof(int32_t) + sizeof(int64_t) + sizeof(uint32_t) + name.length();
        auto *buf = helper(sz_of_data, "10.10.10.10", 0, name.length(), name.c_str(), out_size);
        buf[sizeof(uint64_t)] = 'r';
        return buf;
    }

    char *HrpAPIPacketHelper::dataPacket(const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char *payload, uint32_t &out_size) {
        uint64_t sz_of_data = sizeof(char) + sizeof(int32_t) + sizeof(int64_t) + sizeof(uint32_t) + sz_of_payload;
        auto *buf = helper(sz_of_data, node, time, sz_of_payload, payload, out_size);
        buf[sizeof(uint64_t)] = 's';
        return buf;
    }

    char *HrpAPIPacketHelper::helper(uint64_t sz_of_data, const hrpNode &node, int64_t time, uint32_t sz_of_payload,
                                     const char *payload, uint32_t &out_size) {
        out_size = sizeof(sz_of_data) + sz_of_data;
        auto *buf = new char[out_size];
        auto *mv_buf = buf;
        int32_t raw_ip = node_to_ip(node);

        // copy sz_of_data
        memcpy(mv_buf, &sz_of_data, sizeof(sz_of_data));
        mv_buf += sizeof(sz_of_data);
        // reserve for command
        mv_buf += 1;
        // copy raw ip
        memcpy(mv_buf, &raw_ip, sizeof(int32_t));
        mv_buf += sizeof(int32_t);
        // copy time
        memcpy(mv_buf, &time, sizeof(time));
        mv_buf += sizeof(time);
        // copy sz_of_payload
        memcpy(mv_buf, &sz_of_payload, sizeof(sz_of_payload));
        mv_buf += sizeof(sz_of_payload);
        // copy payload
        memcpy(mv_buf, payload, sz_of_payload);

        return buf;
    }

    HrpAPIPacket HrpAPIPacketHelper::parseFromBuffer(const char *buf, uint64_t size) {
        const char* mv_buf = buf;
        char command;
        int32_t ip_raw;
        int64_t time;
        uint32_t sz_of_payload;
        char* payload;

        command = mv_buf[0];
        mv_buf += 1;

        memcpy(&ip_raw, mv_buf, sizeof(int32_t));
        mv_buf += sizeof(int32_t);

        memcpy(&time, mv_buf, sizeof(int64_t));
        mv_buf += sizeof(int64_t);

        memcpy(&sz_of_payload, mv_buf, sizeof(uint32_t));
        mv_buf += sizeof(uint32_t);

        payload = new char[sz_of_payload];
        memcpy(payload, mv_buf, sz_of_payload);

        hrp_api_pkt_type type = command=='r'?api_register:api_data;
        HrpAPIPacket pkt(type, ip_to_node(ip_raw), time, sz_of_payload, payload);

        return pkt;
    }
}
