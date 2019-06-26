//
// Created by Chen Yang on 9/8/17.
//

#include <gtest/gtest.h>
#include <hrpNode.h>
#include "hrp/HrpGraph.h"

class HrpGraphTest : public ::testing::Test {
public:
    HrpGraphTest() {}

protected:
    virtual void SetUp() {
        _nodes[0] = "n0";
        _nodes[1] = "n1";
        _nodes[2] = "n2";
        _nodes[3] = "n3";
        _nodes[4] = "n4";

        for (auto itr : _nodes) _graph1.addNode(itr);

    }

    virtual void TearDown() {
    }

    hrp::HrpGraph _graph1;
    hrp::hrpNode _nodes[5];
};

TEST_F(HrpGraphTest, addNodeTest) {
    hrp::hrpNode n("test");
    _graph1.addNode(n);
    ASSERT_EQ(6, lemon::countNodes(_graph1.get_graph()));
    ASSERT_EQ(1, _graph1.get_vertices().count(n));
}

TEST_F(HrpGraphTest, delNodeTest) {
    _graph1.delNode(_nodes[0]);
    ASSERT_EQ(4, lemon::countNodes(_graph1.get_graph()));
    ASSERT_EQ(0, _graph1.get_vertices().count(_nodes[0]));
}

TEST_F(HrpGraphTest, addArcTest) {
    _graph1.addArc(_nodes[0], _nodes[1], 1.0);
    auto m = _graph1.get_vertices();
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[0]], m[_nodes[1]]) != lemon::INVALID);
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[1]], m[_nodes[0]]) == lemon::INVALID);
}

TEST_F(HrpGraphTest, delNodeTest2) {
    auto m = _graph1.get_vertices();
    _graph1.addArc(_nodes[0], _nodes[1], 1.0);
    _graph1.delNode(_nodes[0]);
    ASSERT_EQ(4, lemon::countNodes(_graph1.get_graph()));
    ASSERT_EQ(0, _graph1.get_vertices().count(_nodes[0]));
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[0]], m[_nodes[1]]) == lemon::INVALID);
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[1]], m[_nodes[0]]) == lemon::INVALID);
}


TEST_F(HrpGraphTest, delArcTest) {
    auto m = _graph1.get_vertices();
    _graph1.addArc(_nodes[0], _nodes[1], 1.0);
    _graph1.delEdge(_nodes[0], _nodes[1]);
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[0]], m[_nodes[1]]) == lemon::INVALID);
    ASSERT_TRUE(lemon::findArc(_graph1.get_graph(), m[_nodes[1]], m[_nodes[0]]) == lemon::INVALID);
}

