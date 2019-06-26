//
// Created by Chen on 7/27/17.
//

#include <gtest/gtest.h>
#include <hrp/TxrxEngine.h>
#include <hrp/HrpCore.h>
#include "hrpMessages.h"
#include <iostream>
#include <hrpMessages.h>

//TEST(testRecv, test1) {
//    hrp::TxrxEngine engine(8080);
//    engine.init();
//
//    std::this_thread::sleep_for(std::chrono::duration<double>(100));
//    engine.shutdown();
//}

TEST(testProtoMessage1, test1) {
    hrp::HrpDataHeader header("src", "dst", 1, hrp::hrp_packet_type::pi_probe_request);
    hrp::pi_probe_meta payload;
    payload.seq = 0;
    payload.repl = 3;

    header.set_payload(&payload, sizeof(payload));

    auto *get_payload = static_cast<const hrp::pi_probe_meta *>(header.get_payload());
    ASSERT_EQ(payload.seq, get_payload->seq);
    ASSERT_EQ(payload.repl, get_payload->repl);
}

TEST(testProtoMessage2, test2) {
    std::chrono::duration<double> duration = std::chrono::seconds(13);
    asio::io_service io_service;
    hrp::TopologyManager manager("node1", io_service);
    hrp::TxrxEngine engine("node1", io_service, 8080, &manager);
    engine.init();

    hrp::HrpCore core("node1", std::map<std::string, std::string>());
    core.set_engine(&engine);
    core.init();
    std::this_thread::sleep_for(duration);
}
