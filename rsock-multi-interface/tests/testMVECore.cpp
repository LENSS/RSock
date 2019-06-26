//
// Created by Chen on 7/18/17.
//

#include "mve/MVECore.h"
#include <gtest/gtest.h>

class MVECoreTest : public ::testing::Test {
public:
    MVECoreTest() : core("testNode") {}

protected:
    virtual void SetUp() {
        hrp::ect_set s1, s2, s3;
        s1.insert("1");
        s2.insert("2");
        s3.insert("1");
        s3.insert("2");

        core.updateParam(s1,1.0);
        core.updateParam(s2,1.0);
        core.updateParam(s3,1.0);
    }

    virtual void TearDown() {
    }


    hrp::MVECore core;
};


TEST_F(MVECoreTest, updateParamTest) {
    hrp::ect_set s;
    s.insert("4");
    core.updateParam(s, 2.0);

    // Should be able to add
    ASSERT_EQ(core.get_params().size(), 4);
    ASSERT_TRUE(core.get_params().find(s) != core.get_params().end());

    auto itr = core.get_params().find(s);
    ASSERT_DOUBLE_EQ(itr->second, 2.0);

    // Should be able to update
    core.updateParam(s, 3.0);
    itr = core.get_params().find(s);
    ASSERT_DOUBLE_EQ(itr->second, 3.0);

    // Should be able to update on a different set but with same elements
    hrp::ect_set s1;
    s1.insert("4");
    core.updateParam(s1, 4.0);
    itr = core.get_params().find(s);
    ASSERT_DOUBLE_EQ(itr->second, 4.0);
}

TEST_F(MVECoreTest, getParamTest) {

    // Should be able to get param that exist
    hrp::ect_set s;
    s.insert("1");
    ASSERT_DOUBLE_EQ(core.getParam(s), 1.0);

    // Should be able to return 0 if not exist
    hrp::ect_set s1;
    s1.insert("4");
    ASSERT_DOUBLE_EQ(core.getParam(s1), 0.0);
}

TEST_F(MVECoreTest, getMarginalTest) {

    // Should be able to get marginal
    hrp::ect_set s;
    s.insert("1");
    ASSERT_DOUBLE_EQ(core.getMarginal(s), 2.0);

    s.insert("2");
    ASSERT_DOUBLE_EQ(core.getMarginal(s), 1.0);


    // Should be able to return 0 if not marginal
    hrp::ect_set s1;
    s1.insert("1");
    s1.insert("3");
    ASSERT_DOUBLE_EQ(core.getMarginal(s1), 0.0);
}
