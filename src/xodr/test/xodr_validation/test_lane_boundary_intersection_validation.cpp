#include <gtest/gtest.h>
#include "xodr_validation/lane_boundary_intersection_validation.h"
#include "xodr/xodr_map.h"

#include "../test_config.h"

namespace aid { namespace xodr {

bool intersectingGeometryViolationEquals(const IntersectingGeometryViolation& a, const IntersectingGeometryViolation& b)
{
    if (a.laneKeyA_ == b.laneKeyA_)
    {
        return a.laneKeyB_ == b.laneKeyB_ && std::abs(a.sCoordA_ - b.sCoordA_) < 0.001 &&
               std::abs(a.sCoordB_ - b.sCoordB_) < 0.001;
    }
    if (a.laneKeyA_ == b.laneKeyB_)
    {
        return a.laneKeyB_ == b.laneKeyA_ && std::abs(a.sCoordA_ - b.sCoordB_) < 0.001 &&
               std::abs(a.sCoordB_ - b.sCoordA_) < 0.001;
    }
    return false;
}

TEST(LaneBoundaryIntersectionValidationTest, testValidateRoundabout)
{
    XodrMap xodrMap =
        XodrMap::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/roundabout_1lane_houses_v1.xodr").extract_value();

    std::vector<IntersectingGeometryViolation> expectedErrors;
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(14, 0, -3), 10.8159, LaneKey(1, 0, -1), 0.29699, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(15, 0, -3), 10.8159, LaneKey(18, 0, -1), 0.29699, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(16, 0, -3), 10.8159, LaneKey(19, 0, -1), 0.29699, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(17, 0, -3), 10.8159, LaneKey(0, 0, -1), 0.29699, Eigen::Vector2d()));
    {
        std::vector<IntersectingGeometryViolation> errors;
        bool res = validateBoundaryIntersections(xodrMap, 0.1, errors);
        EXPECT_FALSE(res);
        ASSERT_EQ(expectedErrors.size(), 4);
        EXPECT_EQ(errors.size(), 4);
        if (errors.size() == 4)
        {
            for (const auto& error : errors)
            {
                EXPECT_NE(std::find_if(expectedErrors.begin(), expectedErrors.end(),
                                       [&error](const IntersectingGeometryViolation& expected) {
                                           return intersectingGeometryViolationEquals(error, expected);
                                       }),
                          expectedErrors.end())
                    << error.description(xodrMap) << " was not expected";
            }
        }
    }
    {
        // More tolerance = no errors
        std::vector<IntersectingGeometryViolation> errors;
        bool res = validateBoundaryIntersections(xodrMap, 0.3, errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(15, 0, -3), 13.1655, LaneKey(18, 0, 1), 0.0736295, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(15, 0, -3), 8.46634, LaneKey(18, 0, -2), 0.0737, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(16, 0, -3), 13.1655, LaneKey(19, 0, 1), 0.0736295, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(16, 0, -3), 8.46634, LaneKey(19, 0, -2), 0.0737, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(17, 0, -3), 13.1655, LaneKey(0, 0, 1), 0.073629, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(17, 0, -3), 8.46634, LaneKey(0, 0, -2), 0.0737, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(14, 0, -3), 13.1655, LaneKey(1, 0, 1), 0.0737, Eigen::Vector2d()));
    expectedErrors.push_back(
        IntersectingGeometryViolation(LaneKey(14, 0, -3), 8.46634, LaneKey(1, 0, -2), 0.0737, Eigen::Vector2d()));
    {
        std::vector<IntersectingGeometryViolation> errors;
        bool res = validateBoundaryIntersections(xodrMap, 0.01, errors);
        EXPECT_FALSE(res);
        ASSERT_EQ(expectedErrors.size(), 12);
        EXPECT_EQ(errors.size(), 12);
        if (errors.size() == 12)
        {
            for (const auto& error : errors)
            {
                EXPECT_NE(std::find_if(expectedErrors.begin(), expectedErrors.end(),
                                       [&error](const IntersectingGeometryViolation& expected) {
                                           return intersectingGeometryViolationEquals(error, expected);
                                       }),
                          expectedErrors.end())
                    << error.description(xodrMap) << " was not expected";
            }
        }
    }
}

}}  // namespace aid::xodr
