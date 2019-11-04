#include "junction.h"

#include <gtest/gtest.h>

#include "xodr_reader.h"

namespace aid { namespace xodr {

TEST(ParseJunctionTest, testParseJunction)
{
    XodrReader xml = XodrReader::fromText(
        "<junction name = 'TheJunction' id = '2'>"
        "  <connection id = '0' incomingRoad = '502' connectingRoad = '500' contactPoint = 'start'>"
        "    <laneLink from = '1' to = '-1'/>"
        "    <laneLink from = '2' to = '-2'/>"
        "    <laneLink from = '3' to = '-3'/>"
        "  </connection>"
        "  <connection id = '1' incomingRoad = '502' connectingRoad = '510' contactPoint = 'end'>"
        "    <laneLink from = '1' to = '-1'/>"
        "  </connection>"
        "</junction>");

    xml.readStartElement("junction");
    Junction junction = Junction::parseXml(xml).value();

    EXPECT_EQ(junction.name(), "TheJunction");
    EXPECT_EQ(junction.id(), "2");

    const auto& connections = junction.connections();
    ASSERT_EQ(connections.size(), 2);

    {
        const auto& conn = connections[0];
        EXPECT_EQ(conn.id(), "0");
        EXPECT_EQ(conn.incomingRoad(), "502");
        EXPECT_EQ(conn.connectingRoad(), "500");
        EXPECT_EQ(conn.contactPoint(), ContactPoint::START);

        const auto& laneLinks = conn.laneLinks();
        ASSERT_EQ(laneLinks.size(), 3);
        EXPECT_EQ(laneLinks[0].from(), LaneID(1));
        EXPECT_EQ(laneLinks[0].to(), LaneID(-1));
        EXPECT_EQ(laneLinks[1].from(), LaneID(2));
        EXPECT_EQ(laneLinks[1].to(), LaneID(-2));
        EXPECT_EQ(laneLinks[2].from(), LaneID(3));
        EXPECT_EQ(laneLinks[2].to(), LaneID(-3));
    }

    {
        const auto& conn = connections[1];
        EXPECT_EQ(conn.id(), "1");
        EXPECT_EQ(conn.incomingRoad(), "502");
        EXPECT_EQ(conn.connectingRoad(), "510");
        EXPECT_EQ(conn.contactPoint(), ContactPoint::END);

        const auto& laneLinks = conn.laneLinks();
        ASSERT_EQ(laneLinks.size(), 1);
        EXPECT_EQ(laneLinks[0].from(), LaneID(1));
        EXPECT_EQ(laneLinks[0].to(), LaneID(-1));
    }
}

}}  // namespace aid::xodr
