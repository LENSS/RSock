//
// Created by Chen on 7/18/17.
//

#include "mve/DelayEstimator.h"
#include "mve/MVEEstimator.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

void run_io_service(asio::io_service &io) {
    //std::cout << "called run_io_service"<< std::endl;
    io.run();
    //std::cout << "exited run_io_service"<< std::endl;
}

class MVEEstimatorTest : public ::testing::Test {
public:
    MVEEstimatorTest() : core("testNode"), estimator(core, _mutex) {}

protected:
    std::mutex _mutex;
    hrp::MVECore core;
    hrp::MVEEstimator estimator;
};

TEST_F(MVEEstimatorTest, delayEstimateTest1) {
    using namespace hrp;
    MVECore srcCore("n1");
    MVECore dstCore("n2");

    int n = 10;
    std::vector<ect_set> nodes(n+1);
    for (int i=1; i<=n; i++) nodes[i] = ect_set({"n"+std::to_string(i)});
    for (int i=3; i<=n; i++) srcCore.updateParam(nodes[i], 1);
    for (int i=3; i<=n; i++) dstCore.updateParam(nodes[i], 1);

    srcCore.updateParam(nodes[2], 1);
    dstCore.updateParam(nodes[1], 1);

    auto d = calcDelays(4, srcCore, dstCore);

    ASSERT_NEAR(d[0], 1, 0.0001);
    ASSERT_NEAR(d[1], 0.625, 0.0001);
    ASSERT_NEAR(d[2], 0.5978, 0.0001);
    ASSERT_NEAR(d[3], 0.6497, 0.0001);
}

TEST_F(MVEEstimatorTest, delayEstimateTest2) {
    using namespace hrp;
    MVECore srcCore("n1");
    MVECore dstCore("n2");

    int n = 10;
    std::vector<ect_set> nodes(n+1);
    for (int i=1; i<=n; i++) nodes[i] = ect_set({"n"+std::to_string(i)});
    for (int i=3; i<=n; i++) srcCore.updateParam(nodes[i], 1);
    for (int i=3; i<=n; i++) dstCore.updateParam(nodes[i], 1);

    ect_set share1({"n3", "n4"});
    srcCore.updateParam(share1, 1);
    dstCore.updateParam(share1, 1);

    srcCore.updateParam(nodes[2], 1);
    dstCore.updateParam(nodes[1], 1);

    auto d = calcDelays(4, srcCore, dstCore);

    ASSERT_NEAR(d[0], 1, 0.0001);
    ASSERT_NEAR(d[1], 0.5448, 0.0001);
    ASSERT_NEAR(d[2], 0.5173, 0.0001);
    ASSERT_NEAR(d[3], 0.5662, 0.0001);
}

TEST_F(MVEEstimatorTest, delayEstimateTest3) {
    using namespace hrp;
    MVECore srcCore("n1");
    MVECore dstCore("n2");

    int n = 10;
    std::vector<ect_set> nodes(n+1);
    for (int i=1; i<=n; i++) nodes[i] = ect_set({"n"+std::to_string(i)});
    for (int i=3; i<=n; i++) srcCore.updateParam(nodes[i], 1);
    for (int i=3; i<=n; i++) dstCore.updateParam(nodes[i], 1);

    auto d = calcDelays(6, srcCore, dstCore);

    ASSERT_NEAR(d[0], std::numeric_limits<double>::max(), 0.0001);
    ASSERT_NEAR(d[1], 1.125, 0.0001);
    ASSERT_NEAR(d[2], 0.7833, 0.0001);
    ASSERT_NEAR(d[3], 0.7536, 0.0001);
    ASSERT_NEAR(d[4], 0.8020, 0.0001);
}

TEST_F(MVEEstimatorTest, delayEstimateTest4) {
    using namespace hrp;
    MVECore srcCore("n1");
    MVECore dstCore("n2");

    srcCore.updateParam(ect_set({"n2"}), 1);
    dstCore.updateParam(ect_set({"n1"}), 1);

    auto d = calcDelays(6, srcCore, dstCore);

    ASSERT_NEAR(d[0], 1, 0.0001);
    ASSERT_NEAR(d[1], 1, 0.0001);
    ASSERT_NEAR(d[2], 1, 0.0001);
    ASSERT_NEAR(d[3], 1, 0.0001);
    ASSERT_NEAR(d[4], 1, 0.0001);
}

TEST_F(MVEEstimatorTest, delayEstimateTest5) {
    using namespace hrp;
    MVECore srcCore("n1");
    MVECore dstCore("n2");

    srcCore.updateParam(ect_set({"n2"}), 0);
    dstCore.updateParam(ect_set({"n1"}), 0);

    auto d = calcDelays(6, srcCore, dstCore);

    ASSERT_NEAR(d[0], std::numeric_limits<double>::max(), 0.0001);
    ASSERT_NEAR(d[1], std::numeric_limits<double>::max(), 0.0001);
    ASSERT_NEAR(d[2], std::numeric_limits<double>::max(), 0.0001);
    ASSERT_NEAR(d[3], std::numeric_limits<double>::max(), 0.0001);
    ASSERT_NEAR(d[4], std::numeric_limits<double>::max(), 0.0001);
}

TEST_F(MVEEstimatorTest, counterTest) {

    asio::io_service io_service;
    estimator.set_io_service(&io_service);
    estimator.init();

    ASSERT_FALSE(io_service.stopped());

    estimator.set_epochDuration(1.0);
    std::chrono::duration<double> duration = std::chrono::duration<double>(estimator.get_epochDuration());

    estimator.addNode("1");
    std::thread thd1 = std::thread(run_io_service, std::ref(io_service));

    // Sleep duration seconds, then check value
    std::this_thread::sleep_for(2*duration);
    // total counter should be 1
    ASSERT_EQ(estimator.get_totalEncounter(), 1);
    // encountered set should be cleared
    ASSERT_TRUE(estimator.get_encountered().empty());
    // counters size should be 1
    ASSERT_EQ(estimator.get_counters().size(), 1);

    std::set<hrp::hrpNode> nodes;
    nodes.insert("1");
    auto itr = estimator.get_counters().find(nodes);
    // Encountered hrpNode set should be in counters
    ASSERT_TRUE(itr != estimator.get_counters().end());
    // Encountered hrpNode should have counter as 1
    ASSERT_DOUBLE_EQ(itr->second, 1);


    estimator.set_epochDuration(2.0);
    duration = std::chrono::duration<double>(estimator.get_epochDuration());

    estimator.addNode("1");
    estimator.addNode("2");

    io_service.reset();
    std::thread thd2 = std::thread(run_io_service, std::ref(io_service));

    // Sleep duration seconds, then check value
    std::this_thread::sleep_for(duration+duration);
    // total counter should be 2
    ASSERT_EQ(estimator.get_totalEncounter(), 2);
    // encountered set should be cleared
    ASSERT_TRUE(estimator.get_encountered().empty());
    // counters size should be 2
    ASSERT_EQ(estimator.get_counters().size(), 2);

    nodes.clear();
    nodes.insert("1");
    nodes.insert("2");
    itr = estimator.get_counters().find(nodes);
    // Encountered hrpNode set should be in counters
    ASSERT_TRUE(itr != estimator.get_counters().end());
    // Encountered hrpNode should have counter as 1
    ASSERT_DOUBLE_EQ(itr->second, 1);

    io_service.stop();
    thd1.join();
    thd2.join();
}


TEST_F(MVEEstimatorTest, paramCalcTest) {
    ASSERT_DOUBLE_EQ(hrp::MVEEstimator::rateCalc(1,1,10),0);
    ASSERT_DOUBLE_EQ(hrp::MVEEstimator::rateCalc(10,5,10),0.45);
    ASSERT_DOUBLE_EQ(hrp::MVEEstimator::rateCalc(20,4,10),0.38);
}

