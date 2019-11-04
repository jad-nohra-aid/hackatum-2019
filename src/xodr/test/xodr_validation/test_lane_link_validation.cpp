#include <gtest/gtest.h>

#include "validation/road_link_validation.h"
#include "validation/lane_link_validation.h"
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

static void setExternalLanePredecessor(XodrMap& map, const std::string& roadId, LaneID laneId, LaneIDOpt toLaneId)
{
    Road& road = *map.test_roadById(roadId);
    LaneSection& laneSection = road.test_laneSection(0);
    laneSection.test_laneById(laneId).test_setPredecessor(toLaneId);
}

static void setExternalLaneSuccessor(XodrMap& map, const std::string& roadId, LaneID laneId, LaneIDOpt toLaneId)
{
    Road& road = *map.test_roadById(roadId);
    LaneSection& laneSection = road.test_laneSection(road.laneSections().size() - 1);
    laneSection.test_laneById(laneId).test_setSuccessor(toLaneId);
}

static void setJunctionConnectionTo(XodrMap& map, const std::string& junctionId, const std::string& connectionId,
                                    LaneID fromLaneId, LaneIDOpt toLaneId)
{
    Junction& junction = *map.test_junctionById(junctionId);
    Junction::Connection& conn = *junction.test_connectionById(connectionId);
    conn.test_setLaneLinkTarget(fromLaneId, toLaneId);
}

TEST(LaneLinkValidationTest, succNoPred)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    setExternalLanePredecessor(map, "2", LaneID(1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneBackLinkNotSpecified* error = dynamic_cast<const LaneBackLinkNotSpecified*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, predNoSucc)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    setExternalLaneSuccessor(map, "1", LaneID(1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneBackLinkNotSpecified* error = dynamic_cast<const LaneBackLinkNotSpecified*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, predNoPred)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_PRED_PRED_XODR_PATH).extract_value();

    setExternalLanePredecessor(map, "2", LaneID(1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneBackLinkNotSpecified* error = dynamic_cast<const LaneBackLinkNotSpecified*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, succNoSucc)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_SUCC_SUCC_XODR_PATH).extract_value();

    setExternalLaneSuccessor(map, "2", LaneID(1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneBackLinkNotSpecified* error = dynamic_cast<const LaneBackLinkNotSpecified*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, succPredMismatch)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    setExternalLaneSuccessor(map, "1", LaneID(1), LaneID(2));
    setExternalLanePredecessor(map, "2", LaneID(1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneLinkMisMatch* error = dynamic_cast<const LaneLinkMisMatch*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(2));
        EXPECT_EQ(error->cLaneId_, LaneID(2));
    }
}

TEST(LaneLinkValidationTest, junctionIncomingMismatch)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    setJunctionConnectionTo(map, "100", "2", LaneID(-1), LaneID(-2));
    setExternalLanePredecessor(map, "3", LaneID(-1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneLinkMisMatch* error = dynamic_cast<const LaneLinkMisMatch*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(-2));
        EXPECT_EQ(error->cLaneId_, LaneID(-2));
    }
}

TEST(LaneLinkValidationTest, junctionConnectingMismatch)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    setJunctionConnectionTo(map, "100", "2", LaneID(-1), LaneIDOpt::null());
    setExternalLanePredecessor(map, "3", LaneID(-1), LaneID(-2));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneLinkMisMatch* error = dynamic_cast<const LaneLinkMisMatch*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(-2));
        EXPECT_EQ(error->cLaneId_, LaneID(-2));
    }
}

TEST(LaneLinkValidationTest, testOpposingDrivingDirInternal)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    Road& road = *map.test_roadById("1");
    LaneSection& aLaneSection = road.test_laneSection(0);
    LaneSection& bLaneSection = road.test_laneSection(1);

    aLaneSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(-1));
    aLaneSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(1));

    bLaneSection.test_laneById(LaneID(1)).test_setPredecessor(LaneID(-1));
    bLaneSection.test_laneById(LaneID(-1)).test_setPredecessor(LaneID(1));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 4);

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 1, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 1, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[2].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 1, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[3].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 1, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 0, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, testOpposingDrivingDirBetweenRoads)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_XODR_PATH).extract_value();

    Road& road1 = *map.test_roadById("1");
    Road& road2 = *map.test_roadById("2");

    LaneSection& aLaneSection = road1.laneSectionForExternalLinkType(RoadLinkType::SUCCESSOR);
    LaneSection& bLaneSection = road2.laneSectionForExternalLinkType(RoadLinkType::PREDECESSOR);

    aLaneSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(-1));
    aLaneSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(1));

    bLaneSection.test_laneById(LaneID(1)).test_setPredecessor(LaneID(-1));
    bLaneSection.test_laneById(LaneID(-1)).test_setPredecessor(LaneID(1));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 4);

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[2].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[3].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, testOpposingDrivingDirBetweenRoadsOpposingDir)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_SUCC_SUCC_XODR_PATH).extract_value();

    Road& road1 = *map.test_roadById("1");
    Road& road2 = *map.test_roadById("2");

    LaneSection& aLaneSection = road1.laneSectionForExternalLinkType(RoadLinkType::SUCCESSOR);
    LaneSection& bLaneSection = road2.laneSectionForExternalLinkType(RoadLinkType::SUCCESSOR);

    aLaneSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(1));
    aLaneSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-1));

    bLaneSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(1));
    bLaneSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-1));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 4);

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[2].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[3].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("2"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }
}

TEST(LaneLinkValidationTest, testOpposingDrivingDirJunctionIncoming)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    setJunctionConnectionTo(map, "100", "2", LaneID(-1), LaneIDOpt::null());
    setJunctionConnectionTo(map, "100", "2", LaneID(1), LaneID(-1));

    setExternalLanePredecessor(map, "2", LaneID(1), LaneIDOpt::null());
    setExternalLanePredecessor(map, "3", LaneID(-1), LaneID(1));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 2);

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(1));
        EXPECT_EQ(error->bLaneId_, LaneID(-1));
    }

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[1].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 0, ContactPoint::START));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, testOpposingDrivingDirJunctionOutgoing)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    setExternalLaneSuccessor(map, "3", LaneID(-1), LaneID(1));

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneLinkOpposingDirections* error = dynamic_cast<const LaneLinkOpposingDirections*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("6"), 0, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(-1));
        EXPECT_EQ(error->bLaneId_, LaneID(1));
    }
}

TEST(LaneLinkValidationTest, testLinkToCenterLineJunctionIncoming)
{
    XodrMap map = XodrMap::fromFile(VALIDATE_LINKS_JUNCTION_XODR_PATH).extract_value();

    setJunctionConnectionTo(map, "100", "2", LaneID(-1), LaneID(0));
    setExternalLanePredecessor(map, "3", LaneID(-1), LaneIDOpt::null());

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    bool res = validateLinks(map, errors);
    EXPECT_FALSE(res);
    ASSERT_EQ(errors.size(), 1);

    {
        const LaneLinkToCenterLaneError* error = dynamic_cast<const LaneLinkToCenterLaneError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("1"), 2, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_,
                  LaneSectionContactPointKey(map.roadIndexById("3"), 0, ContactPoint::START));
        EXPECT_EQ(error->fromLaneId_, LaneID(-1));
    }
}

}}  // namespace aid::xodr
