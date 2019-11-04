#include "junction.h"

#include <gtest/gtest.h>

#include "xodr_map.h"
#include "../test_config.h"

namespace aid { namespace xodr {

TEST(JunctionTest, testConnectionFindLinkTarget)
{
    XodrReader xml = XodrReader::fromText(
        "<connection id='0' incomingRoad='1' connectingRoad='2' contactPoint='start'>"
        "  <laneLink from='2' to='3'/>"
        "  <laneLink from='-1' to='-5'/>"
        "  <laneLink from='-2' to='-6'/>"
        "</connection>");

    xml.readStartElement("connection");
    Junction::Connection connection = Junction::Connection::parseXml(xml).value();

    EXPECT_EQ(connection.findLaneLinkTarget(LaneID(2)), LaneID(3));
    EXPECT_EQ(connection.findLaneLinkTarget(LaneID(1)), LaneIDOpt::null());
    EXPECT_EQ(connection.findLaneLinkTarget(LaneID(-1)), LaneID(-5));
    EXPECT_EQ(connection.findLaneLinkTarget(LaneID(-2)), LaneID(-6));
    EXPECT_EQ(connection.findLaneLinkTarget(LaneID(-1000)), LaneIDOpt::null());
}

}}  // namespace aid::xodr
