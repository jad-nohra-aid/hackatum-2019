#include "road.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(RoadTest, testParseRoad)
{
    XodrReader xml = XodrReader::fromText(
        "<road name = 'RoadToNowhere' id = '50' junction = '2' length = '20'>"
        "  <planView>"
        "    <geometry s = '0' x = '10' y = '20' hdg = '2' length = '20'>"
        "      <line/>"
        "    </geometry>"
        "  </planView>"
        "  <lanes>"
        "    <laneSection s = '0'>"
        "      <left>"
        "        <lane id = '1' type = 'sidewalk' level = 'false'>"
        "          <width sOffset = '0' a = '1' b = '2' c = '3' d = '4'/>"
        "        </lane>"
        "      </left>"
        "      <center>"
        "        <lane id = '0' type = 'driving' level = '0'/>"
        "      </center>"
        "      <right>"
        "        <lane id = '-1' type = 'driving' level = 'false'>"
        "          <width sOffset = '0' a = '9' b = '10' c = '11' d = '12'/>"
        "        </lane>"
        "      </right>"
        "    </laneSection>"
        "  </lanes>"
        "</road>");

    xml.readStartElement("road");
    Road road = std::move(Road::parseXml(xml).value());

    EXPECT_EQ(road.name(), "RoadToNowhere");
    EXPECT_EQ(road.id(), "50");
    EXPECT_EQ(road.junctionRef(), "2");

    EXPECT_EQ(road.length(), 20);

    const ReferenceLine& refLine = road.referenceLine();
    ASSERT_EQ(refLine.numGeometries(), 1);

    const ReferenceLine::Line& line = static_cast<const ReferenceLine::Line&>(refLine.geometry(0));
    EXPECT_EQ(line.geometryType(), ReferenceLine::GeometryType::LINE);
    EXPECT_EQ(line.startVertex().sCoord_, 0);
    EXPECT_EQ(line.startVertex().position_, Eigen::Vector2d(10, 20));
    EXPECT_EQ(line.startVertex().heading_, 2);

    ASSERT_EQ(road.laneSections().size(), 1);

    const LaneSection& laneSection = road.laneSections()[0];
    EXPECT_EQ(laneSection.startS(), 0);
    EXPECT_EQ(laneSection.endS(), 20);
    EXPECT_EQ(laneSection.singleSided(), false);
    EXPECT_EQ(laneSection.numLeftLanes(), 1);

    ASSERT_EQ(laneSection.lanes().size(), 2);

    const LaneSection::Lane& leftLane = laneSection.lanes()[0];
    EXPECT_EQ(leftLane.id(), LaneID(1));
    EXPECT_EQ(leftLane.type(), LaneType::SIDEWALK);
    EXPECT_EQ(leftLane.level(), false);

    ASSERT_EQ(leftLane.widthPoly3s().size(), 1);
    const LaneSection::WidthPoly3& leftWidthPoly = leftLane.widthPoly3s()[0];
    EXPECT_EQ(leftWidthPoly.sOffset(), 0);
    EXPECT_EQ(leftWidthPoly.poly3(), Poly3(1, 2, 3, 4));

    const LaneSection::Lane& rightLane = laneSection.lanes()[1];
    EXPECT_EQ(rightLane.id(), LaneID(-1));
    EXPECT_EQ(rightLane.type(), LaneType::DRIVING);
    EXPECT_EQ(rightLane.level(), false);

    ASSERT_EQ(rightLane.widthPoly3s().size(), 1);
    const LaneSection::WidthPoly3& rightWidthPoly = rightLane.widthPoly3s()[0];
    EXPECT_EQ(rightWidthPoly.sOffset(), 0);
    EXPECT_EQ(rightWidthPoly.poly3(), Poly3(9, 10, 11, 12));
}

}}  // namespace aid::xodr
