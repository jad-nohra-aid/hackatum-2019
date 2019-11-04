#include "lane_section.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(ParseLaneSectionTest, testParseTwoSidedLaneSection)
{
    XodrReader xml = XodrReader::fromText(
        "<laneSection s = '1'>"
        "  <left>"
        "    <lane id = '2' type = 'sidewalk' level = 'false'>"
        "      <width sOffset = '0' a = '1' b = '2' c = '3' d = '4'/>"
        "    </lane>"
        "    <lane id = '1' type = 'driving' level = 'false'>"
        "      <link>"
        "        <predecessor id = '2'/>"
        "        <successor id = '-2'/>"
        "      </link>"
        "      <width sOffset = '0' a = '5' b = '6' c = '7' d = '8'/>"
        "    </lane>"
        "  </left>"
        "  <center>"
        "    <lane id = '0' type = 'driving' level = '0'/>"
        "  </center>"
        "  <right>"
        "    <lane id = '-1' type = 'driving' level = 'false'>"
        "      <link>"
        "        <predecessor id = '3'/>"
        "      </link>"
        "      <width sOffset = '0' a = '9' b = '10' c = '11' d = '12'/>"
        "    </lane>"
        "    <lane id = '-2' type = 'sidewalk' level = 'false'>"
        "      <link>"
        "        <successor id = '4'/>"
        "      </link>"
        "      <width sOffset = '0' a = '13' b = '14' c = '15' d = '16'/>"
        "      <width sOffset = '.5' a = '17' b = '18' c = '19' d = '20'/>"
        "    </lane>"
        "  </right>"
        "</laneSection>");

    xml.readStartElement("laneSection");
    LaneSection laneSection = LaneSection::parseXml(xml).value();

    EXPECT_EQ(laneSection.singleSided(), false);
    EXPECT_EQ(laneSection.numLeftLanes(), 2);

    const auto& lanes = laneSection.lanes();
    ASSERT_EQ(lanes.size(), 4);
    {
        const auto& lane = lanes[0];
        EXPECT_EQ(lane.id(), LaneID(2));
        EXPECT_EQ(lane.type(), LaneType::SIDEWALK);
        EXPECT_EQ(lane.level(), false);
        EXPECT_FALSE(lane.hasPredecessor());
        EXPECT_FALSE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(1, 2, 3, 4));
    }

    {
        const auto& lane = lanes[1];
        EXPECT_EQ(lane.id(), LaneID(1));
        EXPECT_EQ(lane.type(), LaneType::DRIVING);
        EXPECT_EQ(lane.level(), false);
        EXPECT_EQ(lane.predecessor(), LaneID(2));
        EXPECT_EQ(lane.successor(), LaneID(-2));
        EXPECT_TRUE(lane.hasPredecessor());
        EXPECT_TRUE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(5, 6, 7, 8));
    }

    {
        const auto& lane = lanes[2];
        EXPECT_EQ(lane.id(), LaneID(-1));
        EXPECT_EQ(lane.predecessor(), LaneID(3));
        EXPECT_TRUE(lane.hasPredecessor());
        EXPECT_FALSE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(9, 10, 11, 12));
    }

    {
        const auto& lane = lanes[3];
        EXPECT_EQ(lane.id(), LaneID(-2));
        EXPECT_EQ(lane.successor(), LaneID(4));
        EXPECT_FALSE(lane.hasPredecessor());
        EXPECT_TRUE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 2);
        const auto& width0 = lane.widthPoly3s()[0];
        EXPECT_EQ(width0.sOffset(), 0);
        EXPECT_EQ(width0.poly3(), Poly3(13, 14, 15, 16));

        const auto& width1 = lane.widthPoly3s()[1];
        EXPECT_EQ(width1.sOffset(), .5);
        EXPECT_EQ(width1.poly3(), Poly3(17, 18, 19, 20));
    }
}

TEST(ParseLaneSectionTest, testParseLeftLaneSection)
{
    XodrReader xml = XodrReader::fromText(
        "<laneSection s = '1' singleSided = 'true'>"
        "  <left>"
        "    <lane id = '2' type = 'sidewalk' level = 'false'>"
        "      <width sOffset = '0' a = '1' b = '2' c = '3' d = '4'/>"
        "    </lane>"
        "    <lane id = '1' type = 'driving' level = 'false'>"
        "      <link>"
        "        <predecessor id = '2'/>"
        "        <successor id = '-2'/>"
        "      </link>"
        "      <width sOffset = '0' a = '5' b = '6' c = '7' d = '8'/>"
        "    </lane>"
        "  </left>"
        "  <center>"
        "    <lane id = '0' type = 'driving' level = '0'/>"
        "  </center>"
        "</laneSection>");

    xml.readStartElement("laneSection");
    LaneSection laneSection = LaneSection::parseXml(xml).value();

    EXPECT_EQ(laneSection.singleSided(), true);
    EXPECT_EQ(laneSection.numLeftLanes(), 2);

    const auto& lanes = laneSection.lanes();
    ASSERT_EQ(lanes.size(), 2);

    {
        const auto& lane = lanes[0];
        EXPECT_EQ(lane.id(), LaneID(2));
        EXPECT_EQ(lane.type(), LaneType::SIDEWALK);
        EXPECT_EQ(lane.level(), false);
        EXPECT_FALSE(lane.hasPredecessor());
        EXPECT_FALSE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(1, 2, 3, 4));
    }

    {
        const auto& lane = lanes[1];
        EXPECT_EQ(lane.id(), LaneID(1));
        EXPECT_EQ(lane.type(), LaneType::DRIVING);
        EXPECT_EQ(lane.level(), false);
        EXPECT_EQ(lane.predecessor(), LaneID(2));
        EXPECT_EQ(lane.successor(), LaneID(-2));

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(5, 6, 7, 8));
    }
}

TEST(ParseLaneSectionTest, testParseRightLaneSection)
{
    XodrReader xml = XodrReader::fromText(
        "<laneSection s = '1'>"
        "  <center>"
        "    <lane id = '0' type = 'driving' level = '0'/>"
        "  </center>"
        "  <right>"
        "    <lane id = '-1' type = 'driving' level = 'false'>"
        "      <link>"
        "        <predecessor id = '3'/>"
        "      </link>"
        "      <width sOffset = '0' a = '9' b = '10' c = '11' d = '12'/>"
        "    </lane>"
        "    <lane id = '-2' type = 'sidewalk' level = 'false'>"
        "      <link>"
        "        <successor id = '4'/>"
        "      </link>"
        "      <width sOffset = '0' a = '13' b = '14' c = '15' d = '16'/>"
        "      <width sOffset = '.5' a = '17' b = '18' c = '19' d = '20'/>"
        "    </lane>"
        "  </right>"
        "</laneSection>");

    xml.readStartElement("laneSection");
    LaneSection laneSection = LaneSection::parseXml(xml).value();

    EXPECT_EQ(laneSection.singleSided(), false);
    EXPECT_EQ(laneSection.numLeftLanes(), 0);

    const auto& lanes = laneSection.lanes();
    ASSERT_EQ(lanes.size(), 2);

    {
        const auto& lane = lanes[0];
        EXPECT_EQ(lane.id(), LaneID(-1));
        EXPECT_EQ(lane.predecessor(), LaneID(3));
        EXPECT_FALSE(lane.hasSuccessor());

        ASSERT_EQ(lane.widthPoly3s().size(), 1);
        const auto& width = lane.widthPoly3s()[0];
        EXPECT_EQ(width.sOffset(), 0);
        EXPECT_EQ(width.poly3(), Poly3(9, 10, 11, 12));
    }

    {
        const auto& lane = lanes[1];
        EXPECT_EQ(lane.id(), LaneID(-2));
        EXPECT_FALSE(lane.hasPredecessor());
        EXPECT_EQ(lane.successor(), LaneID(4));

        ASSERT_EQ(lane.widthPoly3s().size(), 2);
        const auto& width0 = lane.widthPoly3s()[0];
        EXPECT_EQ(width0.sOffset(), 0);
        EXPECT_EQ(width0.poly3(), Poly3(13, 14, 15, 16));

        const auto& width1 = lane.widthPoly3s()[1];
        EXPECT_EQ(width1.sOffset(), .5);
        EXPECT_EQ(width1.poly3(), Poly3(17, 18, 19, 20));
    }
}

}}  // namespace aid::xodr
