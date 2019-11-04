#include "xodr_map.h"

#include <gtest/gtest.h>

#include "../test_config.h"

namespace aid { namespace xodr {

TEST(XodrMapTest, testParseGeoReference)
{
    XodrReader xml = XodrReader::fromText(
        "<OpenDRIVE>"
        "  <header>"
        "    <geoReference><![CDATA[+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs]]></geoReference>"
        "  </header>"
        "  <road name='road' length='10' id='5' junction='-1'>"
        "    <planView>"
        "      <geometry s='0' x='0' y='0' hdg='0' length='40'>"
        "        <line/>"
        "      </geometry>"
        "    </planView>"
        "    <lanes>"
        "      <laneSection s='0'>"
        "        <left>"
        "          <lane id='1' type='driving' level='false'>"
        "            <width sOffset='0' a='4' b='0' c='0' d='0'/>"
        "          </lane>"
        "        </left>"
        "        <center/>"
        "      </laneSection>"
        "    </lanes>"
        "  </road>"
        "</OpenDRIVE>");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    EXPECT_TRUE(xodrMap.hasGeoReference());
    EXPECT_EQ(xodrMap.geoReference(), "+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
}

TEST(XodrMapTest, testParseGeoReferenceMissing)
{
    XodrReader xml = XodrReader::fromText(
        "<OpenDRIVE>"
        "  <header>"
        "  </header>"
        "  <road name='road' length='10' id='5' junction='-1'>"
        "    <planView>"
        "      <geometry s='0' x='0' y='0' hdg='0' length='40'>"
        "        <line/>"
        "      </geometry>"
        "    </planView>"
        "    <lanes>"
        "      <laneSection s='0'>"
        "        <left>"
        "          <lane id='1' type='driving' level='false'>"
        "            <width sOffset='0' a='4' b='0' c='0' d='0'/>"
        "          </lane>"
        "        </left>"
        "        <center/>"
        "      </laneSection>"
        "    </lanes>"
        "  </road>"
        "</OpenDRIVE>");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    EXPECT_FALSE(xodrMap.hasGeoReference());
}

TEST(XodrMapTest, testResolveRoadRefs)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/resolve_road_refs.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    {
        const Road* road = xodrMap.roadById("15");
        ASSERT_NE(road, nullptr);

        const RoadLink& pred = road->predecessor();
        EXPECT_EQ(pred.elementType(), RoadLink::ElementType::JUNCTION);

        const Junction& predJunction = xodrMap.junctions()[pred.elementRef().index()];
        EXPECT_EQ(predJunction.id(), "junction number two");

        const RoadLink& succ = road->successor();
        EXPECT_EQ(succ.elementType(), RoadLink::ElementType::ROAD);

        const Road& succRoad = xodrMap.roads()[succ.elementRef().index()];
        EXPECT_EQ(succRoad.id(), "road one hundred and sixteen");
    }

    {
        const Junction* junction = xodrMap.junctionById("junction number two");
        ASSERT_NE(junction, nullptr);

        const Junction::Connection& connection = junction->connections()[2];
        EXPECT_EQ(connection.id(), "2");

        const Road& incomingRoad = xodrMap.roads()[connection.incomingRoad().index()];
        EXPECT_EQ(incomingRoad.id(), "15");

        const Road& connectingRoad = xodrMap.roads()[connection.connectingRoad().index()];
        EXPECT_EQ(connectingRoad.id(), "18");
    }
}

TEST(XodrMapTest, testResolveInvalidRoadRef)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/resolve_invalid_road_ref.xodr");

    xml.readStartElement("OpenDRIVE");
    EXPECT_FALSE(XodrMap::parseXml(xml).hasValidConnectivity());
}

TEST(XodrMapTest, testResolveInvalidJunctionRef)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/resolve_invalid_junction_ref.xodr");

    xml.readStartElement("OpenDRIVE");
    EXPECT_FALSE(XodrMap::parseXml(xml).hasValidConnectivity());
}

TEST(XodrMapTest, testGlobalLaneIndex)
{
    XodrMap xodrMap =
        XodrMap::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/resolve_road_refs.xodr").extract_value();

    int expectedGlobalIndex = 0;
    for (const Road& road : xodrMap.roads())
    {
        EXPECT_EQ(road.globalLaneIndicesBegin(), expectedGlobalIndex);

        for (const LaneSection& laneSection : road.laneSections())
        {
            for (const LaneSection::Lane& lane : laneSection.lanes())
            {
                EXPECT_EQ(lane.globalIndex(), expectedGlobalIndex);
                expectedGlobalIndex++;
            }
        }

        EXPECT_EQ(road.globalLaneIndicesEnd(), expectedGlobalIndex);
    }

    EXPECT_EQ(xodrMap.totalNumLanes(), expectedGlobalIndex);
}

}}  // namespace aid::xodr
