//
// Created by Chen Yang on 8/30/17.
//

#include <gtest/gtest.h>
#include "hrpMessages.h"

class HrpMessagesTest : public ::testing::Test {
public:
    HrpMessagesTest() : core("testNode") {}

protected:
    virtual void SetUp() {
        hrp::ect_set s1 = {"n1"};
        hrp::ect_set s2 = {"n1", "n2"};
        hrp::ect_set s3 = {"n1", "n2", "n3"};
        hrp::ect_set s4 = {"n1", "n2", "n3", "n4"};

        core.updateParam(s1, 1.0);
        core.updateParam(s2, 2.0);
        core.updateParam(s3, 3.0);
        core.updateParam(s4, 4.0);
    }

    virtual void TearDown() {
    }

    hrp::MVECore core;
};


TEST_F(HrpMessagesTest, InflateTest) {
    hrp::HrpMveCoreMessage message(core);
    std::string str;
    // Serialize the message to a buffer
    message.inflate_payload(&str);

    // Deserialize the message to a message
    hrp::HrpMveCoreMessage message1(core.get_thisNode(), str.c_str(), str.size() * sizeof(char));
    hrp::MVECore coreTest("testnode");
    // Then inflate the MVECore using the deserialized message
    message1.inflate_mvecore(coreTest);

    // The ect_set and the contact rates should be the same
    for (auto itr : core.get_params()) {
        auto itr1 = coreTest.get_params().find(itr.first);
        ASSERT_TRUE(itr1 != coreTest.get_params().end());
        ASSERT_EQ(itr.second, itr1->second);
    }

    // The _thisNode should be the same
    ASSERT_EQ(core.get_thisNode(), coreTest.get_thisNode());
}

TEST_F(HrpMessagesTest, InflateMveCoreTest) {
    hrp::HrpMveCoreMessage message(core);
    hrp::MVECore coreTest("testnode");
    message.inflate_mvecore(coreTest);

    for (auto itr : core.get_params()) {
        auto itr1 = coreTest.get_params().find(itr.first);
        ASSERT_TRUE(itr1 != coreTest.get_params().end());
        ASSERT_EQ(itr.second, itr1->second);
    }
    ASSERT_EQ(core.get_thisNode(), coreTest.get_thisNode());
}