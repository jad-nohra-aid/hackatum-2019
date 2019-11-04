#include "road_link.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(ParseRoadLinkTest, testParseRoadLink)
{
    XodrReader xml =
        XodrReader::fromText("<predecessor elementType = 'road' elementId = '509' contactPoint = 'start' />");

    xml.readStartElement("predecessor");
    RoadLink roadLink = RoadLink::parseXml(xml).value();

    EXPECT_EQ(roadLink.elementType(), RoadLink::ElementType::ROAD);
    EXPECT_EQ(roadLink.elementRef(), "509");
    EXPECT_EQ(roadLink.contactPoint(), ContactPoint::START);
}

TEST(ParseRoadLinkTest, testParseJunctionLink)
{
    XodrReader xml = XodrReader::fromText("<successor elementType = 'junction' elementId = '509'/>");

    xml.readStartElement("successor");
    RoadLink roadLink = RoadLink::parseXml(xml).value();

    EXPECT_EQ(roadLink.elementType(), RoadLink::ElementType::JUNCTION);
    EXPECT_EQ(roadLink.elementRef(), "509");
}

TEST(ParseRoadLinkTest, testParseLeftNeighbor)
{
    XodrReader xml = XodrReader::fromText("<neighbor side = 'left' elementId = '29' direction = 'same'/>");

    xml.readStartElement("neighbor");
    NeighborLink neighborLink = NeighborLink::parseXml(xml).value();

    EXPECT_EQ(neighborLink.side(), NeighborLink::Side::LEFT);
    EXPECT_EQ(neighborLink.elementRef(), "29");
    EXPECT_EQ(neighborLink.direction(), NeighborLink::Direction::SAME);
}

TEST(ParseRoadLinkTest, testParseRightNeighbor)
{
    XodrReader xml =
        XodrReader::fromText("<neighbor side = 'right' elementId = 'road to nowhere' direction = 'opposite'/>");

    xml.readStartElement("neighbor");
    NeighborLink neighborLink = NeighborLink::parseXml(xml).value();

    EXPECT_EQ(neighborLink.side(), NeighborLink::Side::RIGHT);
    EXPECT_EQ(neighborLink.elementRef(), "road to nowhere");
    EXPECT_EQ(neighborLink.direction(), NeighborLink::Direction::OPPOSITE);
}

TEST(ParseRoadLinkTest, testParsePair)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <predecessor elementType = 'road' elementId = '509' contactPoint = 'start' />"
        "  <successor elementType = 'junction' elementId = '510' />"
        "</link>");

    xml.readStartElement("link");
    RoadLinks roadLinks = RoadLinks::parseXml(xml).value();

    EXPECT_EQ(roadLinks.predecessor().elementType(), RoadLink::ElementType::ROAD);
    EXPECT_EQ(roadLinks.predecessor().elementRef(), "509");
    EXPECT_EQ(roadLinks.predecessor().contactPoint(), ContactPoint::START);

    EXPECT_EQ(roadLinks.successor().elementType(), RoadLink::ElementType::JUNCTION);
    EXPECT_EQ(roadLinks.successor().elementRef(), "510");
}

TEST(ParseRoadLinkTest, testParsePairPredecessorOnly)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <predecessor elementType = 'road' elementId = '509' contactPoint = 'start' />"
        "</link>");

    xml.readStartElement("link");
    RoadLinks roadLinks = RoadLinks::parseXml(xml).value();

    EXPECT_EQ(roadLinks.predecessor().elementType(), RoadLink::ElementType::ROAD);
    EXPECT_EQ(roadLinks.predecessor().elementRef(), "509");
    EXPECT_EQ(roadLinks.predecessor().contactPoint(), ContactPoint::START);

    EXPECT_EQ(roadLinks.successor().elementType(), RoadLink::ElementType::NOT_SPECIFIED);
}

TEST(ParseRoadLinkTest, testParsePairSuccessorOnly)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <successor elementType = 'junction' elementId = '510' />"
        "</link>");

    xml.readStartElement("link");
    RoadLinks roadLinks = RoadLinks::parseXml(xml).value();

    EXPECT_EQ(roadLinks.predecessor().elementType(), RoadLink::ElementType::NOT_SPECIFIED);

    EXPECT_EQ(roadLinks.successor().elementType(), RoadLink::ElementType::JUNCTION);
    EXPECT_EQ(roadLinks.successor().elementRef(), "510");
}

TEST(ParseRoadLinkTest, testParseAllLinks)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <predecessor elementType = 'road' elementId = '509' contactPoint = 'start' />"
        "  <successor elementType = 'junction' elementId = '510' />"
        "  <neighbor side = 'left' elementId = '511' direction = 'same'/>"
        "  <neighbor side = 'right' elementId = '512' direction = 'opposite'/>"
        "</link>");

    xml.readStartElement("link");
    RoadLinks roadLinks = RoadLinks::parseXml(xml).value();

    EXPECT_EQ(roadLinks.predecessor().elementType(), RoadLink::ElementType::ROAD);
    EXPECT_EQ(roadLinks.predecessor().elementRef(), "509");
    EXPECT_EQ(roadLinks.predecessor().contactPoint(), ContactPoint::START);

    EXPECT_EQ(roadLinks.successor().elementType(), RoadLink::ElementType::JUNCTION);
    EXPECT_EQ(roadLinks.successor().elementRef(), "510");

    EXPECT_EQ(roadLinks.leftNeighbor().side(), NeighborLink::Side::LEFT);
    EXPECT_EQ(roadLinks.leftNeighbor().elementRef(), "511");
    EXPECT_EQ(roadLinks.leftNeighbor().direction(), NeighborLink::Direction::SAME);

    EXPECT_EQ(roadLinks.rightNeighbor().side(), NeighborLink::Side::RIGHT);
    EXPECT_EQ(roadLinks.rightNeighbor().elementRef(), "512");
    EXPECT_EQ(roadLinks.rightNeighbor().direction(), NeighborLink::Direction::OPPOSITE);
}

TEST(ParseRoadLinkTest, testParseMultipleLeftNeighbors)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <neighbor side = 'left' elementId = '511' direction = 'same'/>"
        "  <neighbor side = 'left' elementId = '512' direction = 'opposite'/>"
        "</link>");

    xml.readStartElement("link");
    EXPECT_FALSE(RoadLinks::parseXml(xml).hasValidConnectivity());
}

TEST(ParseRoadLinkTest, testParseMultipleRightNeighbors)
{
    XodrReader xml = XodrReader::fromText(
        "<link>"
        "  <neighbor side = 'right' elementId = '511' direction = 'same'/>"
        "  <neighbor side = 'right' elementId = '512' direction = 'opposite'/>"
        "</link>");

    xml.readStartElement("link");
    EXPECT_FALSE(RoadLinks::parseXml(xml).hasValidConnectivity());
}

}}  // namespace aid::xodr
