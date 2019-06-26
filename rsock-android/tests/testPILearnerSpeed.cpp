//
// Created by Chen on 10/9/17.
//

#include "rm/PILearner.h"
#include <cmath>
#include "common.h"
#include <gtest/gtest.h>
#include <rm/PILearner.h>
#include <thread>
#include <hrpMessages.h>

class PILearnerSpeedTest : public ::testing::Test {

public:
    PILearnerSpeedTest() : rmax(3), core(rmax, HRP_DEFAULT_ITA), piLearner(core, HRP_DEFAULT_ALPHA) {}

protected:
    unsigned int rmax;
    hrp::RMCore core;
    hrp::PILearner piLearner;
};



TEST_F(PILearnerSpeedTest, test1) {
    for (int i=0; i<1000; i++) {
        auto meta = piLearner.prepareProbe();
        piLearner.sentProbe(meta);
        hrp::pi_response_tx response_tx;
        response_tx.seq = meta.seq;
        response_tx.repl = meta.repl;
        piLearner.recvResponse(response_tx, 0.01);
        std::cout << i << ": " << piLearner.format_probs();
    }
}