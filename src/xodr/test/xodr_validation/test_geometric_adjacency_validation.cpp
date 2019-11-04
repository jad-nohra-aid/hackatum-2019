#include "xodr_validation/geometric_adjacency_validation.h"

#include <gtest/gtest.h>

#include "xodr/xodr_map.h"
#include "../test_config.h"

namespace aid { namespace xodr {

TEST(TestGeometricAdjacencyValidation, testSimpleSuccess)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/simple_success.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(TestGeometricAdjacencyValidation, testSimpleFailure)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/simple_failure.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_FALSE(res);

    ASSERT_EQ(errors.size(), 1);

    const GeometricAdjacencyError& error = errors[0];
    EXPECT_EQ(error.aLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("1"), 0, ContactPoint::END));
    EXPECT_EQ(error.bLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("2"), 0, ContactPoint::START));
    EXPECT_EQ(error.aLaneIdx_, 0);
    EXPECT_EQ(error.bLaneIdx_, 0);
    EXPECT_EQ(error.onLeftBoundary_, false);
}

TEST(TestGeometricAdjacencyValidation, testManyLanesSuccess)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/many_lanes_success.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(TestGeometricAdjacencyValidation, testManyLanesFailure)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/many_lanes_failure.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_FALSE(res);

    ASSERT_EQ(errors.size(), 2);

    {
        const GeometricAdjacencyError& error = errors[0];
        EXPECT_EQ(error.aLaneSectionContactPointKey_,
                  LaneSectionContactPointKey(xodrMap.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error.bLaneSectionContactPointKey_,
                  LaneSectionContactPointKey(xodrMap.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error.aLaneIdx_, 0);
        EXPECT_EQ(error.bLaneIdx_, 0);
        EXPECT_EQ(error.onLeftBoundary_, false);
    }

    {
        const GeometricAdjacencyError& error = errors[1];
        EXPECT_EQ(error.aLaneSectionContactPointKey_,
                  LaneSectionContactPointKey(xodrMap.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error.bLaneSectionContactPointKey_,
                  LaneSectionContactPointKey(xodrMap.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error.aLaneIdx_, 0);
        EXPECT_EQ(error.bLaneIdx_, 0);
        EXPECT_EQ(error.onLeftBoundary_, true);
    }
}

TEST(TestGeometricAdjacencyValidation, testOnArcSpiralSuccess)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/on_arc_and_spiral_success.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(TestGeometricAdjacencyValidation, testOnArcSpiralFailure)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/on_arc_and_spiral_failure.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_FALSE(res);

    ASSERT_EQ(errors.size(), 1);

    const GeometricAdjacencyError& error = errors[0];
    EXPECT_EQ(error.aLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("1"), 0, ContactPoint::END));
    EXPECT_EQ(error.bLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("2"), 0, ContactPoint::START));
    EXPECT_EQ(error.aLaneIdx_, 0);
    EXPECT_EQ(error.bLaneIdx_, 0);
    EXPECT_EQ(error.onLeftBoundary_, false);
}

TEST(TestGeometricAdjacencyValidation, testOpposingDirectionsSuccess)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/opposing_directions_success.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(TestGeometricAdjacencyValidation, testOpposingDirectionsFailure)
{
    XodrReader xml = XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) +
                                          "xodr/test_geometric_adjacency_validation/opposing_directions_failure.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    std::vector<GeometricAdjacencyError> errors;
    bool res = validateGeometricAdjacency(xodrMap, .05, errors);
    EXPECT_FALSE(res);

    ASSERT_EQ(errors.size(), 1);

    const GeometricAdjacencyError& error = errors[0];
    EXPECT_EQ(error.aLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("1"), 0, ContactPoint::END));
    EXPECT_EQ(error.bLaneSectionContactPointKey_,
              LaneSectionContactPointKey(xodrMap.roadIndexById("2"), 0, ContactPoint::END));
    EXPECT_EQ(error.aLaneIdx_, 0);
    EXPECT_EQ(error.bLaneIdx_, 1);
    EXPECT_EQ(error.onLeftBoundary_, false);
}

}}  // namespace aid::xodr
