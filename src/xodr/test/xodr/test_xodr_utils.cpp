#include "xodr_utils.h"

#include <gtest/gtest.h>
#include <set>

#include "../test_config.h"

namespace aid { namespace xodr {

static bool roadLinkSetInsert(std::set<std::pair<RoadContactPointKey, RoadContactPointKey>>& set,
                              const RoadContactPointKey& aKey, const RoadContactPointKey& bKey)
{
    if (aKey < bKey)
    {
        return set.insert(std::make_pair(aKey, bKey)).second;
    }
    else
    {
        return set.insert(std::make_pair(bKey, aKey)).second;
    }
}

static void testForEachRoadLink(const XodrMap& xodrMap,
                                const std::set<std::pair<RoadContactPointKey, RoadContactPointKey>>& expectedPairs)
{
    std::set<std::pair<RoadContactPointKey, RoadContactPointKey>> encounteredPairs;

    forEachRoadLink(xodrMap, [&](RoadContactPointKey aKey, RoadContactPointKey bKey) {
        bool inserted = roadLinkSetInsert(encounteredPairs, aKey, bKey);
        EXPECT_TRUE(inserted) << "RoadLink encountered more than once.";
    });

    for (std::pair<RoadContactPointKey, RoadContactPointKey> encounteredPair : encounteredPairs)
    {
        EXPECT_TRUE(expectedPairs.find(encounteredPair) != expectedPairs.end())
            << "Unexpected road link encountered: " << encounteredPair.first.toString(xodrMap) << " - "
            << encounteredPair.second.toString(xodrMap);
    }

    for (std::pair<RoadContactPointKey, RoadContactPointKey> expectedPair : expectedPairs)
    {
        EXPECT_TRUE(encounteredPairs.find(expectedPair) != encounteredPairs.end())
            << "Expected road link not encountered: " << expectedPair.first.toString(xodrMap) << " - "
            << expectedPair.second.toString(xodrMap);
    }
}

TEST(XodrUtilsTests, forEachRoadLink_RoadRoad)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_for_each_roadlink/road_road_links.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    int road1Idx = xodrMap.roadIndexById("1");
    int road2Idx = xodrMap.roadIndexById("2");
    int road3Idx = xodrMap.roadIndexById("3");

    std::set<std::pair<RoadContactPointKey, RoadContactPointKey>> expectedPairs;
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(road1Idx, ContactPoint::END),
                      RoadContactPointKey(road2Idx, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(road2Idx, ContactPoint::END),
                      RoadContactPointKey(road3Idx, ContactPoint::END));

    testForEachRoadLink(xodrMap, expectedPairs);
}

TEST(XodrUtilsTests, forEachRoadLink_junctionLinks)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_for_each_roadlink/junction_links.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    int westIdx = xodrMap.roadIndexById("west");
    int eastIdx = xodrMap.roadIndexById("east");
    int northIdx = xodrMap.roadIndexById("north");

    int junctionWestEastIdx = xodrMap.roadIndexById("junction_westEast");
    int junctionEastWestIdx = xodrMap.roadIndexById("junction_eastWest");
    int junctionWestNorthIdx = xodrMap.roadIndexById("junction_westNorth");
    int junctionNorthWestIdx = xodrMap.roadIndexById("junction_northWest");
    int junctionEastNorth = xodrMap.roadIndexById("junction_eastNorth");
    int junctionNorthEast = xodrMap.roadIndexById("junction_northEast");

    std::set<std::pair<RoadContactPointKey, RoadContactPointKey>> expectedPairs;
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(westIdx, ContactPoint::END),
                      RoadContactPointKey(junctionWestEastIdx, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(westIdx, ContactPoint::END),
                      RoadContactPointKey(junctionEastWestIdx, ContactPoint::END));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(eastIdx, ContactPoint::START),
                      RoadContactPointKey(junctionWestEastIdx, ContactPoint::END));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(eastIdx, ContactPoint::START),
                      RoadContactPointKey(junctionEastWestIdx, ContactPoint::START));

    roadLinkSetInsert(expectedPairs, RoadContactPointKey(westIdx, ContactPoint::END),
                      RoadContactPointKey(junctionWestNorthIdx, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(westIdx, ContactPoint::END),
                      RoadContactPointKey(junctionNorthWestIdx, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(northIdx, ContactPoint::START),
                      RoadContactPointKey(junctionWestNorthIdx, ContactPoint::END));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(northIdx, ContactPoint::START),
                      RoadContactPointKey(junctionNorthWestIdx, ContactPoint::END));

    roadLinkSetInsert(expectedPairs, RoadContactPointKey(eastIdx, ContactPoint::START),
                      RoadContactPointKey(junctionEastNorth, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(eastIdx, ContactPoint::START),
                      RoadContactPointKey(junctionNorthEast, ContactPoint::START));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(northIdx, ContactPoint::START),
                      RoadContactPointKey(junctionEastNorth, ContactPoint::END));
    roadLinkSetInsert(expectedPairs, RoadContactPointKey(northIdx, ContactPoint::START),
                      RoadContactPointKey(junctionNorthEast, ContactPoint::END));

    testForEachRoadLink(xodrMap, expectedPairs);
}

}}  // namespace aid::xodr
