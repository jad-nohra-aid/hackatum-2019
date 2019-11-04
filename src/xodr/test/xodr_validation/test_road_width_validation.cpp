#include <gtest/gtest.h>

#include "../test_config.h"

#include "xodr_validation/road_width_validation.h"
#include "xodr/xodr_map.h"

namespace aid { namespace xodr {

static const std::string VALIDATE_ROAD_GEOMETRY_BASE_PATH =
    std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_road_width_validation/";
static const std::string VALIDATE_ARCS_XODR_PATH = VALIDATE_ROAD_GEOMETRY_BASE_PATH + "validate_arcs.xodr";
static const std::string VALIDATE_SPIRALS_XODR_PATH = VALIDATE_ROAD_GEOMETRY_BASE_PATH + "validate_spirals.xodr";
static const std::string VALIDATE_POLY3S_XODR_PATH = VALIDATE_ROAD_GEOMETRY_BASE_PATH + "validate_poly3s.xodr";
static const std::string VALIDATE_PARAMPOLY3S_XODR_PATH =
    VALIDATE_ROAD_GEOMETRY_BASE_PATH + "validate_parampoly3s.xodr";

constexpr double RESOLUTION = 1.0;

void roadTooWideCaseSuccess(const XodrMap& xodrMap, const std::string& roadId)
{
    const Road* road = xodrMap.roadById(roadId);
    ASSERT_NE(road, nullptr);
    RoadWidthValidator validator(*road, RESOLUTION);
    std::vector<RoadTooWideViolation> errors;
    bool res = validator.validateRoadWidth(errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

void roadTooWideCaseFailure(const XodrMap& xodrMap, const std::string& roadId,
                            std::initializer_list<RoadTooWideViolation> expectedErrors)
{
    const Road* road = xodrMap.roadById(roadId);
    ASSERT_NE(road, nullptr);
    RoadWidthValidator validator(*road, RESOLUTION);
    std::vector<RoadTooWideViolation> errors;
    bool res = validator.validateRoadWidth(errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), expectedErrors.size());
    for (const auto& expectedError : expectedErrors)
    {
        const auto targetViolation = [&expectedError](const RoadTooWideViolation& error) -> bool {
            return expectedError.direction_ == error.direction_ && expectedError.startS_ <= error.startS_ &&
                   expectedError.endS_ >= error.endS_;
        };
        auto match = std::find_if(errors.begin(), errors.end(), targetViolation);
        EXPECT_NE(match, errors.end()) << "Expected to receive a RoadTooWideViolation like \""
                                       << expectedError.description() << "\", but did not find it.";
    }
}

TEST(RoadWidthValidationTest, testValidateArcs)
{
    XodrMap xodrMap = XodrMap::fromFile(VALIDATE_ARCS_XODR_PATH).extract_value();
    roadTooWideCaseFailure(xodrMap, "1",
                           {RoadTooWideViolation(nullptr, 0, 32, BoundaryDirection::LEFT),
                            RoadTooWideViolation(nullptr, 31, 63, BoundaryDirection::RIGHT)});
}

TEST(RoadWidthValidationTest, testValidateSpirals)
{
    XodrMap xodrMap = XodrMap::fromFile(VALIDATE_ARCS_XODR_PATH).extract_value();
    roadTooWideCaseFailure(xodrMap, "1",
                           {RoadTooWideViolation(nullptr, 0, 32, BoundaryDirection::LEFT),
                            RoadTooWideViolation(nullptr, 31, 63, BoundaryDirection::RIGHT)});
}

TEST(RoadWidthValidationTest, testValidatePoly3s)
{
    XodrMap xodrMap = XodrMap::fromFile(VALIDATE_POLY3S_XODR_PATH).extract_value();
    {
        const Road* road = xodrMap.roadById("1");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("2");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("3");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_FALSE(res);
        EXPECT_EQ(errors.size(), 1);
        EXPECT_EQ(errors[0].direction_, BoundaryDirection::LEFT);
    }
    {
        const Road* road = xodrMap.roadById("4");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("5");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_FALSE(res);
        EXPECT_EQ(errors.size(), 1);
        EXPECT_EQ(errors[0].direction_, BoundaryDirection::RIGHT);
    }
}

TEST(RoadWidthValidationTest, testValidateParamPoly3s)
{
    XodrMap xodrMap = XodrMap::fromFile(VALIDATE_PARAMPOLY3S_XODR_PATH).extract_value();
    {
        const Road* road = xodrMap.roadById("1");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("2");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("3");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_FALSE(res);
        EXPECT_EQ(errors.size(), 1);
    }
    {
        const Road* road = xodrMap.roadById("4");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_TRUE(res);
        EXPECT_EQ(errors.size(), 0);
    }
    {
        const Road* road = xodrMap.roadById("5");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_FALSE(res);
        EXPECT_EQ(errors.size(), 1);
    }
    {
        const Road* road = xodrMap.roadById("6");
        ASSERT_NE(road, nullptr);
        RoadWidthValidator validator(*road, RESOLUTION);
        std::vector<RoadTooWideViolation> errors;
        bool res = validator.validateRoadWidth(errors);
        EXPECT_FALSE(res);
        EXPECT_EQ(errors.size(), 1);
    }
}

TEST(RoadWidthValidationTest, testValidateNSUv4)
{
    XodrMap xodrMap = XodrMap::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/NSU_v4.xodr").extract_value();
    std::vector<RoadTooWideViolation> errors;
    bool res = validateRoadWidths(xodrMap, RESOLUTION, errors);
    EXPECT_FALSE(res);
    // There's a lot of roads that are technically incorrect but not reporting them would not matter too much as the
    // actually produced loops are very tiny. The specifically highlighted expected failures below are actual failures
    // that are visibile from within xodr_dev.
    EXPECT_LE(errors.size(), 115);
    std::vector<std::string> expectedFailures{"9222", "9223", "9277", "9297", "9364", "9419", "9454", "9467", "9483"};
    auto itFailure = errors.begin();
    for (auto failureCase : expectedFailures)
    {
        // starting at itFailure works if the list of expected failures is sorted.
        itFailure = std::find_if(itFailure, errors.end(), [&failureCase](const RoadTooWideViolation& error) {
            return error.road_->id() == failureCase;
        });
        EXPECT_NE(itFailure, errors.end()) << "Road " << failureCase << " should not validate" << std::endl;
        if (itFailure == errors.end())
        {
            // Try again from the beginning for the next failure
            itFailure = errors.begin();
        }
    }
}

}}  // namespace aid::xodr