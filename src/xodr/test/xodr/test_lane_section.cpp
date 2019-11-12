#include "lane_section.h"

#include <gtest/gtest.h>

#include "reference_line.h"
#include "validation/lane_link_validation.h"

namespace aid { namespace xodr {

class LaneSectionTest : public testing::Test
{
  public:
    LaneSectionTest()
    {
        XodrReader refLineReader = XodrReader::fromText(
            "<planView>"
            "  <geometry s='0' x='0' y='0' hdg='0' length='10'>"
            "    <line/>"
            "  </geometry>"
            "</planView>");

        refLineReader.readStartElement("planView");
        refLine_ = ReferenceLine::parseXml(refLineReader).value();

        XodrReader laneSectionReader = XodrReader::fromText(
            "<laneSection s='0'>"
            "  <left>"
            "    <lane id='3' type='sidewalk' level='false'>"
            "      <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
            "    </lane>"
            "    <lane id='2' type='border' level='false'>"
            "      <width sOffset='0' a='0.35' b='0' c='0' d='0'/>"
            "    </lane>"
            "    <lane id='1' type='driving' level='false'>"
            "      <width sOffset='0' a='3.65' b='0' c='0' d='0'/>"
            "    </lane>"
            "  </left>"
            "  <center>"
            "    <lane id='0' type='driving' level='false'>"
            "    </lane>"
            "  </center>"
            "  <right>"
            "    <lane id='-1' type='driving' level='false'>"
            "      <width sOffset='0' a='3.65' b='0' c='0' d='0'/>"
            "    </lane>"
            "    <lane id='-2' type='border' level= 'false'>"
            "      <width sOffset='0' a='0.35' b='0' c='0' d='0'/>"
            "    </lane>"
            "    <lane id='-3' type='sidewalk' level = 'false'>"
            "      <width sOffset='0' a='1.5' b='-0' c='0' d='0'/>"
            "    </lane>"
            "  </right>"
            "</laneSection>");

        laneSectionReader.readStartElement("laneSection");
        laneSection_ = LaneSection::parseXml(laneSectionReader).value();
    }

    LaneSection laneSection_;
    ReferenceLine refLine_;
};

TEST_F(LaneSectionTest, testTessellate)
{
    xodr::ReferenceLine::Tessellation refLineTessellation = refLine_.tessellate(0, 9);

    auto boundaries = laneSection_.tessellateLaneBoundaries(refLineTessellation);
    ASSERT_EQ(boundaries.size(), 7);

    const double expectedLateral[] = {
        5.5, 4, 3.65, 0, -3.65, -4, -5.5,
    };

    for (int i = 0; i < static_cast<int>(boundaries.size()); i++)
    {
        EXPECT_EQ(boundaries[i].lateralPositions_.size(), refLineTessellation.size());

        for (double lateralPos : boundaries[i].lateralPositions_)
        {
            EXPECT_EQ(lateralPos, expectedLateral[i]);
        }
    }
}

TEST_F(LaneSectionTest, testLaneIdToIndex)
{
    int lane3Idx = laneSection_.laneIdToIndex(LaneID(3));
    EXPECT_EQ(lane3Idx, 0);

    int lane1Idx = laneSection_.laneIdToIndex(LaneID(1));
    EXPECT_EQ(lane1Idx, 2);

    int laneMinus1Idx = laneSection_.laneIdToIndex(LaneID(-1));
    EXPECT_EQ(laneMinus1Idx, 3);

    int laneMinus3Idx = laneSection_.laneIdToIndex(LaneID(-3));
    EXPECT_EQ(laneMinus3Idx, 5);
}

TEST_F(LaneSectionTest, testLaneIndexToId)
{
    LaneID lane3Id = laneSection_.laneIndexToId(0);
    EXPECT_EQ(lane3Id, LaneID(3));

    LaneID lane1Id = laneSection_.laneIndexToId(2);
    EXPECT_EQ(lane1Id, LaneID(1));

    LaneID laneMinus1Id = laneSection_.laneIndexToId(3);
    EXPECT_EQ(laneMinus1Id, LaneID(-1));

    LaneID laneMinus3Id = laneSection_.laneIndexToId(5);
    EXPECT_EQ(laneMinus3Id, LaneID(-3));
}

TEST_F(LaneSectionTest, testLaneById)
{
    const LaneSection::Lane& lane3 = laneSection_.laneById(LaneID(3));
    EXPECT_EQ(lane3.id(), LaneID(3));

    const LaneSection::Lane& lane1 = laneSection_.laneById(LaneID(1));
    EXPECT_EQ(lane1.id(), LaneID(1));

    const LaneSection::Lane& laneMinus1 = laneSection_.laneById(LaneID(-1));
    EXPECT_EQ(laneMinus1.id(), LaneID(-1));

    const LaneSection::Lane& laneMinus3 = laneSection_.laneById(LaneID(-3));
    EXPECT_EQ(laneMinus3.id(), LaneID(-3));
}

TEST_F(LaneSectionTest, testValidateLaneLinks)
{
    LaneSection fromSection = laneSection_;
    LaneSection toSection = laneSection_;

    fromSection.test_laneById(LaneID(3)).test_setSuccessor(LaneID(3));
    fromSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(2));
    fromSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-3));
    fromSection.test_laneById(LaneID(-3)).test_setSuccessor(LaneID(-2));

    toSection.test_laneById(LaneID(3)).test_setPredecessor(LaneID(3));
    toSection.test_laneById(LaneID(2)).test_setPredecessor(LaneID(1));
    toSection.test_laneById(LaneID(-3)).test_setPredecessor(LaneID(-1));
    toSection.test_laneById(LaneID(-2)).test_setPredecessor(LaneID(-3));

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(fromSection, toSection, LaneSectionContactPointKey(0, 0, ContactPoint::END),
                          LaneSectionContactPointKey(0, 1, ContactPoint::START), errors);
        EXPECT_EQ(errors.size(), 0);
    }

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(toSection, fromSection, LaneSectionContactPointKey(0, 1, ContactPoint::START),
                          LaneSectionContactPointKey(0, 0, ContactPoint::END), errors);
        EXPECT_EQ(errors.size(), 0);
    }
}

TEST_F(LaneSectionTest, testValidateLaneLinks_LinkToCenterLane)
{
    LaneSection fromSection = laneSection_;
    LaneSection toSection = laneSection_;

    fromSection.test_laneById(LaneID(3)).test_setSuccessor(LaneID(0));
    fromSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(1));
    fromSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-1));
    fromSection.test_laneById(LaneID(-3)).test_setSuccessor(LaneID(-3));

    toSection.test_laneById(LaneID(3)).test_setPredecessor(LaneIDOpt::null());
    toSection.test_laneById(LaneID(1)).test_setPredecessor(LaneID(1));
    toSection.test_laneById(LaneID(-1)).test_setPredecessor(LaneID(-1));
    toSection.test_laneById(LaneID(-3)).test_setPredecessor(LaneID(-3));

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(fromSection, toSection, LaneSectionContactPointKey(0, 0, ContactPoint::END),
                          LaneSectionContactPointKey(0, 1, ContactPoint::START), errors);
        ASSERT_EQ(errors.size(), 1);

        const LaneLinkToCenterLaneError* error = dynamic_cast<const LaneLinkToCenterLaneError*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_, LaneSectionContactPointKey(0, 0, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_, LaneSectionContactPointKey(0, 1, ContactPoint::START));
        EXPECT_EQ(error->fromLaneId_, LaneID(3));
    }
}

TEST_F(LaneSectionTest, testValidateLaneLinks_OutOfRange)
{
    LaneSection fromSection = laneSection_;
    LaneSection toSection = laneSection_;

    fromSection.test_laneById(LaneID(3)).test_setSuccessor(LaneID(5));
    fromSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(2));
    fromSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-3));
    fromSection.test_laneById(LaneID(-3)).test_setSuccessor(LaneID(-2));

    toSection.test_laneById(LaneID(3)).test_setPredecessor(LaneIDOpt::null());
    toSection.test_laneById(LaneID(2)).test_setPredecessor(LaneID(1));
    toSection.test_laneById(LaneID(-3)).test_setPredecessor(LaneID(-1));
    toSection.test_laneById(LaneID(-2)).test_setPredecessor(LaneID(-3));

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(fromSection, toSection, LaneSectionContactPointKey(0, 0, ContactPoint::END),
                          LaneSectionContactPointKey(0, 1, ContactPoint::START), errors);
        ASSERT_EQ(errors.size(), 1);

        const LaneLinkTargetOutOfRange* error = dynamic_cast<const LaneLinkTargetOutOfRange*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_, LaneSectionContactPointKey(0, 0, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_, LaneSectionContactPointKey(0, 1, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(3));
        EXPECT_EQ(error->bLaneId_, LaneID(5));
    }
}

TEST_F(LaneSectionTest, testValidateLaneLinks_BackLinkMissing)
{
    LaneSection fromSection = laneSection_;
    LaneSection toSection = laneSection_;

    fromSection.test_laneById(LaneID(3)).test_setSuccessor(LaneID(3));
    fromSection.test_laneById(LaneID(1)).test_setSuccessor(LaneID(2));
    fromSection.test_laneById(LaneID(-1)).test_setSuccessor(LaneID(-3));
    fromSection.test_laneById(LaneID(-3)).test_setSuccessor(LaneID(-2));

    toSection.test_laneById(LaneID(2)).test_setPredecessor(LaneID(1));
    toSection.test_laneById(LaneID(-3)).test_setPredecessor(LaneID(-1));
    toSection.test_laneById(LaneID(-2)).test_setPredecessor(LaneID(-3));

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(fromSection, toSection, LaneSectionContactPointKey(0, 0, ContactPoint::END),
                          LaneSectionContactPointKey(0, 1, ContactPoint::START), errors);
        ASSERT_EQ(errors.size(), 1);

        const LaneBackLinkNotSpecified* error = dynamic_cast<const LaneBackLinkNotSpecified*>(errors[0].get());
        ASSERT_NE(error, nullptr);
        EXPECT_EQ(error->aSectionContactPointKey_, LaneSectionContactPointKey(0, 0, ContactPoint::END));
        EXPECT_EQ(error->bSectionContactPointKey_, LaneSectionContactPointKey(0, 1, ContactPoint::START));
        EXPECT_EQ(error->aLaneId_, LaneID(3));
        EXPECT_EQ(error->bLaneId_, LaneID(3));
    }

    {
        std::vector<std::unique_ptr<LinkValidationError>> errors;
        validateLaneLinks(toSection, fromSection, LaneSectionContactPointKey(0, 1, ContactPoint::START),
                          LaneSectionContactPointKey(0, 0, ContactPoint::END), errors);
        EXPECT_EQ(errors.size(), 0);
    }
}

class ValidateLaneAttribSOffsetTest : public ::testing::Test, public ::testing::WithParamInterface<const char*>
{
  public:
};

TEST_P(ValidateLaneAttribSOffsetTest, testSuccess)
{
    char xodrText[256];
    int numWritten = snprintf(xodrText, sizeof(xodrText), GetParam(), 1.5, 2.75);
    ASSERT_LT(numWritten, sizeof(xodrText));

    XodrReader xml = XodrReader::fromText(xodrText);

    xml.readStartElement("lane");
    LaneSection::Lane lane = LaneSection::Lane::parseXml(xml).value();

    lane.validate(3);
}

TEST_P(ValidateLaneAttribSOffsetTest, testWrongOrder)
{
    char xodrText[256];
    int numWritten = snprintf(xodrText, sizeof(xodrText), GetParam(), 2.75, 1.5);
    ASSERT_LT(numWritten, sizeof(xodrText));

    XodrReader xml = XodrReader::fromText(xodrText);

    xml.readStartElement("lane");
    LaneSection::Lane lane = LaneSection::Lane::parseXml(xml).value();

    constexpr float endT = 3;
    EXPECT_ANY_THROW(lane.validate(endT));
}

TEST_P(ValidateLaneAttribSOffsetTest, testRepeatedSCoord)
{
    char xodrText[256];
    int numWritten = snprintf(xodrText, sizeof(xodrText), GetParam(), 2.75, 2.75);
    ASSERT_LT(numWritten, sizeof(xodrText));

    XodrReader xml = XodrReader::fromText(xodrText);

    xml.readStartElement("lane");
    LaneSection::Lane lane = LaneSection::Lane::parseXml(xml).value();

    constexpr float endT = 3;
    EXPECT_ANY_THROW(lane.validate(endT));
}

TEST_P(ValidateLaneAttribSOffsetTest, testSOffsetOutOfRange1)
{
    char xodrText[256];
    int numWritten = snprintf(xodrText, sizeof(xodrText), GetParam(), -1.5, 2.75);
    ASSERT_LT(numWritten, sizeof(xodrText));

    XodrReader xml = XodrReader::fromText(xodrText);

    xml.readStartElement("lane");
    LaneSection::Lane lane = LaneSection::Lane::parseXml(xml).value();

    constexpr float endT = 3;
    EXPECT_ANY_THROW(lane.validate(endT));
}

TEST_P(ValidateLaneAttribSOffsetTest, testSOffsetOutOfRange2)
{
    char xodrText[256];
    int numWritten = snprintf(xodrText, sizeof(xodrText), GetParam(), 1.5, 5.75);
    ASSERT_LT(numWritten, sizeof(xodrText));

    XodrReader xml = XodrReader::fromText(xodrText);

    xml.readStartElement("lane");
    LaneSection::Lane lane = LaneSection::Lane::parseXml(xml).value();

    constexpr float endT = 3;
    EXPECT_ANY_THROW(lane.validate(endT));
}

INSTANTIATE_TEST_CASE_P(
    Material, ValidateLaneAttribSOffsetTest,
    ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                   "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                   "  <material sOffset='%f' surface='dirt' friction='50' roughness='200'/>"
                                   "  <material sOffset='%f' surface='asphalt' friction='5' roughness='20'/>"
                                   "</lane>"));

INSTANTIATE_TEST_CASE_P(
    Visibility, ValidateLaneAttribSOffsetTest,
    ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                   "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                   "  <visibility sOffset='%f' forward='10' back='20' left='3' right='4'/>"
                                   "  <visibility sOffset='%f' forward='4' back='10' left='30' right='44'/>"
                                   "</lane>"));

INSTANTIATE_TEST_CASE_P(SpeedLimit, ValidateLaneAttribSOffsetTest,
                        ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                                       "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                                       "  <speed sOffset='%f' max='60' unit='km/h'/>"
                                                       "  <speed sOffset='%f' max='120'/>"
                                                       "</lane>"));

INSTANTIATE_TEST_CASE_P(Access, ValidateLaneAttribSOffsetTest,
                        ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                                       "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                                       "  <access sOffset='%f' restriction='pedestrian'/>"
                                                       "  <access sOffset='%f' restriction='simulation'/>"
                                                       "</lane>"));

INSTANTIATE_TEST_CASE_P(Height, ValidateLaneAttribSOffsetTest,
                        ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                                       "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                                       "  <height sOffset='%f' inner='-1.25' outer='1.75'/>"
                                                       "  <height sOffset='%f' inner='-1.15' outer='1.55'/>"
                                                       "</lane>"));

INSTANTIATE_TEST_CASE_P(Rule, ValidateLaneAttribSOffsetTest,
                        ::testing::Values<const char*>("<lane id='1' type='driving' level='false'>"
                                                       "  <width sOffset='0' a='1.5' b='0' c='0' d='0'/>"
                                                       "  <rule sOffset='%f' value='no stopping at any time'/>"
                                                       "  <rule sOffset='%f' value='I would turn back if I were you'/>"
                                                       "</lane>"));

}}  // namespace aid::xodr
