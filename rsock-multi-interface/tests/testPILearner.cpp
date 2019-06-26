//
// Created by Chen on 7/20/17.
//

#include "rm/PILearner.h"
#include <cmath>
#include "common.h"
#include <gtest/gtest.h>
#include <rm/PILearner.h>
#include <thread>

class PILearnerTest : public ::testing::Test {

public:
    PILearnerTest() : rmax(8), core(rmax, 0.5), piLearner(core, 0.5) {}

protected:
    unsigned int rmax;
    hrp::RMCore core;
    hrp::PILearner piLearner;
};

class PILearnerSeqTest : public PILearnerTest {
public:
    PILearnerSeqTest(unsigned int s = 5) : PILearnerTest(), sent(s) {}
protected:
    virtual void SetUp() {
        for (int i=0; i<sent; i++) {
            hrp::pi_probe_meta meta = piLearner.prepareProbe();
            piLearner.sentProbe(meta);
            std::this_thread::sleep_for(std::chrono::duration<double>(0.1));
        }
    }

    virtual void TearDown() {
    }
protected:
    unsigned int sent;
};

class PILearnerUpdateTest : public PILearnerSeqTest {
public:
    PILearnerUpdateTest() : PILearnerSeqTest(20) {}
};



TEST_F(PILearnerTest, prepareProbeTest) {
    int i=0;
    for (i=0; i<rmax; i++) {
        hrp::pi_probe_meta meta = piLearner.prepareProbe();
        ASSERT_EQ(meta.seq, i);
        ASSERT_EQ(piLearner.get_seq(), i+1);
        ASSERT_EQ(meta.repl, i+1);
    }
    for (; i<piLearner.get_pi_epoch(); i++) {
        hrp::pi_probe_meta meta = piLearner.prepareProbe();
        ASSERT_EQ(meta.seq, i);
        ASSERT_EQ(piLearner.get_seq(), i+1);
    }

    for (; i<rmax+piLearner.get_pi_epoch(); i++) {
        hrp::pi_probe_meta meta = piLearner.prepareProbe();
        ASSERT_EQ(meta.seq, i);
        ASSERT_EQ(piLearner.get_seq(), i+1);
        ASSERT_EQ(meta.repl, i-piLearner.get_pi_epoch()+1);
    }

    std::vector<int> chosen(rmax);
    for (i=0; i<10000; i++) {
        hrp::pi_probe_meta meta = piLearner.prepareProbe();
        chosen[meta.repl-1]++;
    }

    for (i=0; i<rmax; i++) {
        ASSERT_LE(std::abs((double)chosen[i]/10000.0) - 1.0/(double)rmax, 0.005);
    }

}

TEST_F(PILearnerTest, sentProbeTest) {
    std::vector<std::chrono::steady_clock::time_point> timepoints;
    std::vector<hrp::pi_probe_meta> tmp;
    for (int i=0; i<10; i++) {
        timepoints.push_back(std::chrono::steady_clock::now());
        hrp::pi_probe_meta meta = piLearner.prepareProbe();
        piLearner.sentProbe(meta);
        tmp.push_back(meta);
        std::this_thread::sleep_for(std::chrono::duration<double>(0.1));
    }
    ASSERT_EQ(piLearner.get_track_table().size(), 10);
    for (unsigned int i=0; i<10; i++) {
        std::chrono::steady_clock::time_point t;
        auto itr = piLearner.get_track_table().find(i);
        t = itr->second.second;
        std::chrono::duration<double> d = t - timepoints[i];
        ASSERT_LE(d.count(), 0.0001);
        ASSERT_EQ(itr->second.first, tmp[i].repl);
    }
}

/**
 * In-order receive responses, then
 *  1) tracking table should be updated,
 *  2) _loss_aggr should be updated,
 *  3) _response_seq should be updated
 */
TEST_F(PILearnerSeqTest, recvResponseInOrderTest) {
    for (unsigned int i=0; i<sent; i++) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl =  i<core.get_rmax()? (i+1) : core.drawAction();
        piLearner.recvResponse(meta, i*0.1);
    }
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent);
    ASSERT_EQ(piLearner.get_response_seq(), sent);

    const std::vector<hrp::pi_response_local>& buf = piLearner.get_loss_aggr();
    for (unsigned int i=0; i<sent; i++) {
        ASSERT_EQ(buf[i].meta.seq, i);
        ASSERT_DOUBLE_EQ(buf[i].latency, i*0.1);
    }
}

/**
 * Out-order receive responses, test 1,
 *  1) tracking table should be updated, no matter what, as long as a response is received
 *  2) _loss_aggr should only be updated for in order responses
 *  3) _out_order_buf should contains out-of-order responses
 */
TEST_F(PILearnerSeqTest, recvResponseOutOrderTest_1) {
    for (int i = sent-1; i>=0; i--) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl =  i<core.get_rmax()? (i+1) : core.drawAction();
        piLearner.recvResponse(meta, i*0.1);
    }

    // only the last one is in order, but then it should get updated to the most recent seq
    // so it should seem like everything that is received is now in the loss_aggr
    ASSERT_EQ(piLearner.get_response_seq(), sent);
    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent);
    // all responses received within timeout, tracking table should be empty now
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    // there are sent-1 out-of-order responses, but everything should be fixed by the first
    // in-order response
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
}

TEST_F(PILearnerSeqTest, recvResponseOutOrderTest_2) {
    // receive two in-order response first
    for (unsigned int i = 0; i<2; i++) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl = i+1;
        piLearner.recvResponse(meta, i*0.1);
    }
    // then receive out-of-order responses
    for (unsigned int i = sent-1; i>=2; i--) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl =  i<core.get_rmax()? (i+1) : core.drawAction();
        piLearner.recvResponse(meta, i*0.1);
    }

    // only the last one is in order, but then it should get updated to the most recent seq
    // so it should seem like everything that is received is now in the loss_aggr
    ASSERT_EQ(piLearner.get_response_seq(), sent);
    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent);
    // all responses received within timeout, tracking table should be empty now
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    // there are sent-1 out-of-order responses, but everything should be fixed by the first
    // in-order response
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
}

/**
 * If it receives duplicate response, it should be ignored
 */
TEST_F(PILearnerSeqTest, recvResponseDupTest) {
    hrp::pi_response_tx meta;
    meta.seq = 1;
    meta.repl = 2;

    piLearner.recvResponse(meta, 0.1);
    piLearner.recvResponse(meta, 0.1);

    // in total 0 responses is in order, _response_seq should be 0
    ASSERT_EQ(piLearner.get_response_seq(), 0);
    ASSERT_EQ(piLearner.get_loss_aggr().size(), 0);
    // 1 response received within timeout, tracking table should have size sent-1
    ASSERT_EQ(piLearner.get_track_table().size(), sent-1);
    // there are 1 out-of-order responses, _out_order_buf should have 1 element
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 1);
}

/**
 * As long as the learner receives enough responses without timeout, no matter
 * in what order, the update procedure should be invoked
 */
TEST_F(PILearnerUpdateTest, recvResponseUpdateTest_1) {
    // Receive everything in order
    for (unsigned int i=0; i<piLearner.get_pi_epoch(); i++) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl = i<core.get_rmax()? (i+1) : core.drawAction();
        piLearner.recvResponse(meta, i*0.1);
    }
    ASSERT_EQ(piLearner.get_loss_aggr().size(), 0);
    ASSERT_EQ(piLearner.get_track_table().size(), sent-piLearner.get_pi_epoch());
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
}

TEST_F(PILearnerUpdateTest, recvResponseUpdateTest_2) {
    // Receive everything in reverse order
    for (int i=piLearner.get_pi_epoch()-1; i>=0; i--) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl = i<core.get_rmax()? (i+1) : core.drawAction();
        piLearner.recvResponse(meta, i*0.1);
    }
    ASSERT_EQ(piLearner.get_loss_aggr().size(), 0);
    ASSERT_EQ(piLearner.get_track_table().size(), sent-piLearner.get_pi_epoch());
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
}

TEST_F(PILearnerUpdateTest, recvResponseUpdateTest_3) {
    // Receive everything in crazy order
    std::vector<unsigned int> seqs;
    for (unsigned int i=0; i<piLearner.get_pi_epoch(); i++) {
        seqs.push_back(i);
    }
    std::random_shuffle(seqs.begin(), seqs.end());
    for (unsigned int i=0; i<piLearner.get_pi_epoch(); i++) {
        hrp::pi_response_tx meta;
        meta.seq = seqs[i];
        meta.repl = seqs[i]<core.get_rmax()? (seqs[i]+1) : core.drawAction();
        piLearner.recvResponse(meta, seqs[i]*0.1);
    }

    ASSERT_EQ(piLearner.get_loss_aggr().size(), 0);
    ASSERT_EQ(piLearner.get_track_table().size(), sent-piLearner.get_pi_epoch());
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
}

/**
 * If the probes in the tracking table is expired, it should be deleted
 * once the purgeOutdatedProbes is called
 */
TEST_F(PILearnerSeqTest, purgeOutdatedProbesTest_1) {
    piLearner.set_pi_timeout(0.5);
    // Sleep enough time so that everything can be purged
    std::this_thread::sleep_for(std::chrono::duration<double>(1));

    piLearner.update();
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
    // since there's not enough responses for an epoch, they should still be in loss_aggr
    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent);
    // now these out-dated are purged, _response_seq should get updated too
    ASSERT_EQ(piLearner.get_response_seq(), sent);
}

TEST_F(PILearnerUpdateTest, purgeOutdatedProbesTest_2) {
    piLearner.set_pi_timeout(0.5);
    // Sleep enough time so that everything can be purged
    std::this_thread::sleep_for(std::chrono::duration<double>(1));

    piLearner.update();
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
    // since there're enough responses for an epoch, those should be gone; the rest should be
    // sent minus epoch size
    int supposed_val = 0;
    while (supposed_val + piLearner.get_pi_epoch() <= sent) supposed_val += piLearner.get_pi_epoch();

    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent - supposed_val);
    ASSERT_EQ(piLearner.get_response_seq(), sent);
}

/**
 * If a few responses were received in-order and on time
 */
TEST_F(PILearnerUpdateTest, purgeOutdatedProbesTest_3) {
    for (unsigned int i=0; i<2; i++) {
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl = i+1;
        piLearner.recvResponse(meta, i*0.1);
    }

    piLearner.set_pi_timeout(0.5);
    // Sleep enough time so that everything can be purged
    std::this_thread::sleep_for(std::chrono::duration<double>(1));

    piLearner.update();
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
    // since there're enough responses for an epoch, those should be gone; the rest should be
    // sent minus epoch size
    int supposed_val = 0;
    while (supposed_val + piLearner.get_pi_epoch() <= sent) supposed_val += piLearner.get_pi_epoch();

    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent - supposed_val);
    ASSERT_EQ(piLearner.get_response_seq(), sent);
}


/**
 * If a few responses were received out-of-order and on time
 */
TEST_F(PILearnerUpdateTest, purgeOutdatedProbesTest_4) {
    for (unsigned int i=0; i<6; i++) {
        if (i%2 == 0) continue;
        hrp::pi_response_tx meta;
        meta.seq = i;
        meta.repl = i+1;
        piLearner.recvResponse(meta, i*0.1);
    }

    piLearner.set_pi_timeout(0.5);
    // Sleep enough time so that everything can be purged
    std::this_thread::sleep_for(std::chrono::duration<double>(1));

    piLearner.update();
    ASSERT_EQ(piLearner.get_track_table().size(), 0);
    ASSERT_EQ(piLearner.get_out_order_buf().size(), 0);
    // since there're enough responses for an epoch, those should be gone; the rest should be
    // sent minus epoch size
    int supposed_val = 0;
    while (supposed_val + piLearner.get_pi_epoch() <= sent) supposed_val += piLearner.get_pi_epoch();

    ASSERT_EQ(piLearner.get_loss_aggr().size(), sent - supposed_val);
    ASSERT_EQ(piLearner.get_response_seq(), sent);
}