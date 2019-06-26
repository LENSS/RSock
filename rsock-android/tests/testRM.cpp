//
// Created by Chen Yang on 7/17/17.
//

#include "rm/RMCore.h"
#include <gtest/gtest.h>
#include <cmath>


class RMCoreTest : public ::testing::Test {
public:
    RMCoreTest() : core(4,0.5) {}

protected:
//    virtual void SetUp() {
//
//    }

    // virtual void TearDown() {}

    hrp::RMCore core;
};



// Tests factorial of 0.
TEST_F(RMCoreTest, RMTest) {
    ASSERT_EQ(core.get_rmax(), 4);
    ASSERT_DOUBLE_EQ(core.get_ita(), 0.5);
    ASSERT_EQ(core.get_probs().size(), 4);
    ASSERT_EQ(core.get_weights().size(), 4);
    for (int i=0; i<4; i++) {
        ASSERT_DOUBLE_EQ(core.get_weights()[i], 1.0);
        ASSERT_DOUBLE_EQ(core.get_probs()[i], 0.25);
    }
}

TEST_F(RMCoreTest, DrawActionTest) {
    int r[4];
    int a;
    double p[4];
    memset(r, 0, 4*sizeof(int));
    memset(p, 0, 4*sizeof(double));

    for (int i=0; i<10000; i++) {
        a = core.drawAction();
        ASSERT_GT(a, 0);
        ASSERT_LE(a, 4);
        r[a-1]++;
    }

    for (int i=0; i<4; i++) p[i] = ((double)r[i]) / ((double)10000);
    for (int i=0; i<4; i++) ASSERT_LE(std::abs(0.25-p[i]), 0.01);
}

TEST_F(RMCoreTest, ReceiveLossVecTest) {
    std::vector<double> loss = {0,0,0,0.5};
    core.receiveLossVec(loss);

    ASSERT_DOUBLE_EQ(core.get_weights()[0], 1);
    ASSERT_DOUBLE_EQ(core.get_weights()[1], 1);
    ASSERT_DOUBLE_EQ(core.get_weights()[2], 1);
    ASSERT_DOUBLE_EQ(core.get_weights()[3], 0.75);

    ASSERT_LE(std::abs(core.get_probs()[0] - 0.267), 0.01);
    ASSERT_LE(std::abs(core.get_probs()[1] - 0.267), 0.01);
    ASSERT_LE(std::abs(core.get_probs()[2] - 0.267), 0.01);
    ASSERT_LE(std::abs(core.get_probs()[3] - 0.2), 0.01);
}
