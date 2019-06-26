//
// Created by Chen on 9/21/17.
//

#include "hrpMessages.h"

namespace hrp {

    HrpBloomFilterMessage::HrpBloomFilterMessage(const std::string &buf) {
        _proto_msg.ParseFromString(buf);
        init_from_proto_msg();
    }

    HrpBloomFilterMessage::HrpBloomFilterMessage(const hrp_message::BloomFilter &msg) : _proto_msg(msg) {
        init_from_proto_msg();
    }

    HrpBloomFilterMessage::HrpBloomFilterMessage(const bloom_filter &filter) : bloom_filter(filter) {
        _proto_msg.set_salt_count_(salt_count_);
        _proto_msg.set_table_size_(table_size_);
        _proto_msg.set_projected_element_count_(projected_element_count_);
        _proto_msg.set_inserted_element_count_(inserted_element_count_);
        _proto_msg.set_random_seed_(random_seed_);
        _proto_msg.set_desired_false_positive_probability_(desired_false_positive_probability_);
        for (auto n : salt_) _proto_msg.add_salt_(n);
        _proto_msg.set_bit_table_(std::string(bit_table_.begin(), bit_table_.end()));
    }

    HrpBloomFilterMessage::HrpBloomFilterMessage(const HrpBloomFilterMessage &msg) : bloom_filter(msg), _proto_msg(msg.get_proto_msg()) {

    }

    void HrpBloomFilterMessage::init_from_proto_msg() {
        salt_count_ = _proto_msg.salt_count_();
        table_size_ = _proto_msg.table_size_();
        projected_element_count_ = _proto_msg.projected_element_count_();
        inserted_element_count_ = _proto_msg.inserted_element_count_();
        random_seed_ = _proto_msg.random_seed_();
        desired_false_positive_probability_ = _proto_msg.desired_false_positive_probability_();
        for (int i=0; i<_proto_msg.salt__size(); i++) salt_.push_back(_proto_msg.salt_(i));

        const std::string& bit = _proto_msg.bit_table_();
        for (int i=0; i<_proto_msg.bit_table_().length(); i++) bit_table_.push_back((unsigned char)bit[i]);
    }

    const hrp_message::BloomFilter &HrpBloomFilterMessage::get_proto_msg() const {
        return _proto_msg;
    }
}