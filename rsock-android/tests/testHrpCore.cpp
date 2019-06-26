//
// Created by Chen on 10/16/17.
//

#include <gtest/gtest.h>
#include "hrp/HrpCore.h"

using namespace hrp;



TEST(TestHrpCore, test1) {
    file_sink = std::make_shared<spdlog::sinks::simple_file_sink_mt>("testing log");

    std::string ipAddr("192.168.0.78");
    asio::io_service io_service;
    TopologyManager manager(hrpNode(ipAddr), io_service);
    TxrxEngine engine(hrp::hrpNode(ipAddr), io_service, 18888, &manager);
    HrpCore core("test", std::map<std::string, std::string>());
    core.set_engine(&engine);

    manager.init();
    engine.init();
    core.init();

    hrpNode node1("node1");
    MVECore mveCore(node1);
    bloom_filter filter;



    core.nodeConnected(node1, mveCore, filter);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    core.nodeDisconnected(node1);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    core.nodeConnected(node1, mveCore, filter);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    core.nodeDisconnected(node1);

    auto db = core.get_neighbors_db();
    ASSERT_NE(db.find(node1), db.end());

    auto &entry = db.find(node1)->second;
    ASSERT_NEAR(entry._ct_duration.count(), 2, 0.01);

    std::chrono::duration<double> duration = (std::chrono::steady_clock::now() - entry._start_time);
    ASSERT_NEAR(duration.count(), 3, 0.02);

    io_service.stop();
    auto &thd = engine.get_thread_io_service();
    thd.join();
}