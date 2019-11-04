#include <gtest/gtest.h>

#include "xodr_validation/road_link_validation.h"
#include "xodr/xodr_map.h"
#include "../test_config.h"

namespace aid { namespace xodr {

static const std::string VALIDATE_LINKS_BASE_PATH = std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_link_validation/";
static const std::string VALIDATE_LINKS_XODR_PATH = VALIDATE_LINKS_BASE_PATH + "validate_links.xodr";
static const std::string VALIDATE_LINKS_PRED_PRED_XODR_PATH =
    VALIDATE_LINKS_BASE_PATH + "validate_links_pred_pred_connection.xodr";
static const std::string VALIDATE_LINKS_SUCC_SUCC_XODR_PATH =
    VALIDATE_LINKS_BASE_PATH + "validate_links_succ_succ_connection.xodr";
static const std::string VALIDATE_LINKS_JUNCTION_XODR_PATH = VALIDATE_LINKS_BASE_PATH + "validate_links_junction.xodr";
static const std::string VALIDATE_LINKS_JUNCTION2_XODR_PATH =
    VALIDATE_LINKS_BASE_PATH + "validate_links_junction2.xodr";

TEST(RoadLinkValidationTest, testValidateLinks)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(RoadLinkValidationTest, testValidateLinksPredPred)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_PRED_PRED_XODR_PATH).extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(RoadLinkValidationTest, testValidateLinksSuccSucc)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_SUCC_SUCC_XODR_PATH).extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(RoadLinkValidationTest, validateLinksJunction)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(RoadLinkValidationTest, validateLinksJunction2)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION2_XODR_PATH).extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

// Failing cases

TEST(RoadLinkValidationTest, succNoPred)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    // Set the predecessor of road 2 to 'not specified'.
    map.test_roadById("2")->test_setPredecessor(RoadLink());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[0].get());
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
    EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
    EXPECT_EQ(error->aToBJunctionIdx_, -1);
}

TEST(RoadLinkValidationTest, predNoSucc)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    // Set the successor of road 1 to 'not specified'.
    map.test_roadById("1")->test_setSuccessor(RoadLink());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[0].get());
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
    EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
    EXPECT_EQ(error->aToBJunctionIdx_, -1);
}

TEST(RoadLinkValidationTest, predNoPred)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_PRED_PRED_XODR_PATH).extract_value();

    // Set the predecessor of road 2 to 'not specified'.
    map.test_roadById("2")->test_setPredecessor(RoadLink());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[0].get());
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::START));
    EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
    EXPECT_EQ(error->aToBJunctionIdx_, -1);
}

TEST(RoadLinkValidationTest, succNoSucc)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_SUCC_SUCC_XODR_PATH).extract_value();

    // Set the successor of road 1 to 'not specified'.
    map.test_roadById("1")->test_setSuccessor(RoadLink());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[0].get());
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::END));
    EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
    EXPECT_EQ(error->aToBJunctionIdx_, -1);
}

TEST(RoadLinkValidationTest, roadLinkMismatch)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    map.test_roadById("2")->test_setPredecessor(
        RoadLink::roadLink(XodrObjectReference("1", map.roadIndexById("1")), ContactPoint::START));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 2);

    {
        RoadLinkMisMatchError* error = dynamic_cast<RoadLinkMisMatchError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->cContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::START));
        EXPECT_EQ(error->aToBJunctionIdx_, -1);
    }

    {
        RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::START));
        EXPECT_EQ(error->aToBJunctionIdx_, -1);
    }
}

TEST(RoadLinkValidationTest, noBackLinkFromJunction)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION2_XODR_PATH).extract_value();

    Road& road = *map.test_roadById("2");
    road.test_setPredecessor(RoadLink());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->aToBJunctionIdx_, map.junctionIndexById("100"));
    }
}

TEST(RoadLinkValidationTest, invalidRoadJunctionLink)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION2_XODR_PATH).extract_value();

    Road& road = *map.test_roadById("2");
    road.test_setPredecessor(RoadLink::roadLink(XodrObjectReference("3", map.roadIndexById("3")), ContactPoint::END));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 2);

    {
        const RoadLinkMisMatchError* error = dynamic_cast<const RoadLinkMisMatchError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->cContactPointKey_, RoadContactPointKey(map.roadIndexById("3"), ContactPoint::END));
        EXPECT_EQ(error->aToBJunctionIdx_, map.junctionIndexById("100"));
    }

    {
        const RoadBackLinkNotSpecifiedError* error = dynamic_cast<RoadBackLinkNotSpecifiedError*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("3"), ContactPoint::END));
        EXPECT_EQ(error->aToBJunctionIdx_, -1);
    }
}

TEST(RoadLinkValidationTest, invalidJunctionJunctionLink)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION2_XODR_PATH).extract_value();

    Road& road = *map.test_roadById("2");
    road.test_setPredecessor(RoadLink::junctionLink(XodrObjectReference("100", map.junctionIndexById("100"))));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const RoadBackLinkNotSpecifiedInJunctionError* error =
            dynamic_cast<const RoadBackLinkNotSpecifiedInJunctionError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
        EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
        EXPECT_EQ(error->aToBJunctionIdx_, map.junctionIndexById("100"));
        EXPECT_EQ(error->backLinkJunctionIdx_, map.junctionIndexById("100"));
    }
}

TEST(RoadLinkValidationTest, directLinkToJunctionRoad)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_BASE_PATH + "validate_links_direct_link_to_connecting_road.xodr")
                      .extract_value();

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    const DirectLinkToJunctionRoadError* error = dynamic_cast<const DirectLinkToJunctionRoadError*>(errors[0].get());
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(error->aContactPointKey_, RoadContactPointKey(map.roadIndexById("1"), ContactPoint::END));
    EXPECT_EQ(error->bContactPointKey_, RoadContactPointKey(map.roadIndexById("2"), ContactPoint::START));
}

}}  // namespace aid::xodr