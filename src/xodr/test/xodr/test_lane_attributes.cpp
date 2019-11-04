#include "lane_attributes.h"
#include "lane_section.h"

#include <gtest/gtest.h>

#include "xodr_map.h"
#include "../test_config.h"

namespace aid { namespace xodr {

TEST(LaneMaterialTest, testParseLaneMaterial)
{
    XodrReader xml = XodrReader::fromText("<material sOffset='2' surface='asphalt' friction='5' roughness='20'/>");

    xml.readStartElement("material");
    LaneMaterial material = LaneMaterial::parseXml(xml).value();

    EXPECT_EQ(material.sOffset(), 2);
    EXPECT_EQ(material.surface(), "asphalt");
    EXPECT_EQ(material.friction(), 5);
    EXPECT_EQ(material.roughness(), 20);
}

TEST(LaneVisibilityTest, testParseLaneVisibility)
{
    XodrReader xml = XodrReader::fromText("<visibility sOffset='2' forward='10' back='20' left='3' right='4'/>");

    xml.readStartElement("visibility");
    LaneVisibility visibility = LaneVisibility::parseXml(xml).value();

    EXPECT_EQ(visibility.sOffset(), 2);
    EXPECT_EQ(visibility.forward(), 10);
    EXPECT_EQ(visibility.back(), 20);
    EXPECT_EQ(visibility.left(), 3);
    EXPECT_EQ(visibility.right(), 4);
}

TEST(LaneSpeedLimitTest, testParseLaneSpeedLimit)
{
    XodrReader xml = XodrReader::fromText("<speed sOffset='1.5' max='120' unit='km/h'/>");

    xml.readStartElement("speed");
    LaneSpeedLimit speedLimit = LaneSpeedLimit::parseXml(xml).value();

    EXPECT_EQ(speedLimit.sOffset(), 1.5);
    EXPECT_EQ(speedLimit.maxSpeed(), 120);
    EXPECT_EQ(speedLimit.unit(), SpeedUnit::KILOMETERS_PER_HOUR);
}

TEST(LaneSpeedLimitTest, testParseLaneSpeedLimitNoUnit)
{
    XodrReader xml = XodrReader::fromText("<speed sOffset='1.5' max='120'/>");

    xml.readStartElement("speed");
    LaneSpeedLimit speedLimit = LaneSpeedLimit::parseXml(xml).value();

    EXPECT_EQ(speedLimit.sOffset(), 1.5);
    EXPECT_EQ(speedLimit.maxSpeed(), 120);
    EXPECT_EQ(speedLimit.unit(), SpeedUnit::NOT_SPECIFIED);
}

TEST(LaneAccessTest, testParseLaneAccess)
{
    XodrReader xml = XodrReader::fromText("<access sOffset='1.5' restriction='pedestrian'/>");

    xml.readStartElement("access");
    LaneAccess access = LaneAccess::parseXml(xml).value();

    EXPECT_EQ(access.sOffset(), 1.5);
    EXPECT_EQ(access.restriction(), "pedestrian");
}

TEST(LaneHeightTest, testParseLaneHeight)
{
    XodrReader xml = XodrReader::fromText("<height sOffset='1.5' inner='-1.25' outer='1.75'/>");

    xml.readStartElement("height");
    LaneHeight height = LaneHeight::parseXml(xml).value();

    EXPECT_EQ(height.sOffset(), 1.5);
    EXPECT_EQ(height.inner(), -1.25);
    EXPECT_EQ(height.outer(), 1.75);
}

TEST(LaneRuleTest, testParseLaneRule)
{
    XodrReader xml = XodrReader::fromText("<rule sOffset='1.5' value='no stopping at any time'/>");

    xml.readStartElement("rule");
    LaneRule rule = LaneRule::parseXml(xml).value();

    EXPECT_EQ(rule.sOffset(), 1.5);
    EXPECT_EQ(rule.value(), "no stopping at any time");
}

}}  // namespace aid::xodr
