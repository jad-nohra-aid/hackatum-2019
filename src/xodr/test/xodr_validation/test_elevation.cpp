#include "xodr/elevation.h"
#include "xodr_validation/elevation_validation.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(ElevationProfileTest, testParse)
{
    XodrReader xml = XodrReader::fromText(
        "<elevationProfile>"
        "  <elevation s='0' a='100' b='0' c='3' d='-2'/>"
        "  <elevation s='1' a='101' b='0' c='-3' d='2'/>"
        "</elevationProfile>");

    xml.readStartElement("elevationProfile");
    ElevationProfile elevationProfile = ElevationProfile::parseXml(xml).value();

    const std::vector<ElevationProfile::Elevation>& elevations = elevationProfile.elevations();
    EXPECT_EQ(elevations.size(), 2);
    EXPECT_EQ(elevations[0].sCoord(), 0);
    EXPECT_EQ(elevations[0].poly3().a_, 100);
    EXPECT_EQ(elevations[0].poly3().b_, 0);
    EXPECT_EQ(elevations[0].poly3().c_, 3);
    EXPECT_EQ(elevations[0].poly3().d_, -2);
    EXPECT_EQ(elevations[1].sCoord(), 1);
    EXPECT_EQ(elevations[1].poly3().a_, 101);
    EXPECT_EQ(elevations[1].poly3().b_, 0);
    EXPECT_EQ(elevations[1].poly3().c_, -3);
    EXPECT_EQ(elevations[1].poly3().d_, 2);
}

TEST(ElevationTest, testParseEmpty)
{
    XodrReader xml = XodrReader::fromText("<elevationProfile/>");

    xml.readStartElement("elevationProfile");
    ElevationProfile::parseXml(xml);
    EXPECT_FALSE(ElevationProfile::parseXml(xml).errors().empty());
}

TEST(ElevationTest, testValidateMaxSlope_InRange)
{
    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(0, 0, 3, -2));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(ElevationTest, testValidateMaxSlope_InclineMidTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(500, 0, 3, -2));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 1);

    EXPECT_EQ(errors[0].roadIndex_, 20);
    EXPECT_EQ(errors[0].segmentIndex_, 10);

    double errorStartS = errors[0].startS_;
    double errorEndS = errors[0].endS_;
    double errorMaxSlope = errors[0].maxSlope_;
    EXPECT_LT(segment.poly3().evalDerivative(errorStartS - .001), maxSlope);
    EXPECT_GT(segment.poly3().evalDerivative(errorStartS + .001), maxSlope);
    EXPECT_GT(segment.poly3().evalDerivative(errorEndS - .001), maxSlope);
    EXPECT_LT(segment.poly3().evalDerivative(errorEndS + .001), maxSlope);
    EXPECT_NEAR(segment.poly3().evalDerivative(.5), errorMaxSlope, 00001);
}

TEST(ElevationTest, testValidateMaxSlope_DeclineMidTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(501, 0, -3, 2));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 1);

    EXPECT_EQ(errors[0].roadIndex_, 20);
    EXPECT_EQ(errors[0].segmentIndex_, 10);

    double errorStartS = errors[0].startS_;
    double errorEndS = errors[0].endS_;
    double errorMaxSlope = errors[0].maxSlope_;
    EXPECT_GT(segment.poly3().evalDerivative(errorStartS - .001), -maxSlope);
    EXPECT_LT(segment.poly3().evalDerivative(errorStartS + .001), -maxSlope);
    EXPECT_LT(segment.poly3().evalDerivative(errorEndS - .001), -maxSlope);
    EXPECT_GT(segment.poly3().evalDerivative(errorEndS + .001), -maxSlope);
    EXPECT_NEAR(segment.poly3().evalDerivative(.5), errorMaxSlope, 00001);
}

TEST(ElevationTest, testValidateMaxSlope_InclineSidesTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(500, 3, -6, 4));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 2);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);

        double errorStartS = errors[0].startS_;
        double errorEndS = errors[0].endS_;
        double errorMaxSlope = errors[0].maxSlope_;
        EXPECT_EQ(errorStartS, 0);
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS), maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS - .001), maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS + .001), maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(0), errorMaxSlope, 00001);
    }

    {
        EXPECT_EQ(errors[1].roadIndex_, 20);
        EXPECT_EQ(errors[1].segmentIndex_, 10);

        double errorStartS = errors[1].startS_;
        double errorEndS = errors[1].endS_;
        double errorMaxSlope = errors[1].maxSlope_;
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS - .001), maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS + .001), maxSlope);
        EXPECT_EQ(errorEndS, 1);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS), maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(1), errorMaxSlope, 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_DeclineSidesTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(500, -3, 6, -4));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 2);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);

        double errorStartS = errors[0].startS_;
        double errorEndS = errors[0].endS_;
        double errorMaxSlope = errors[0].maxSlope_;
        EXPECT_EQ(errorStartS, 0);
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS), -maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS - .001), -maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS + .001), -maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(0), errorMaxSlope, 00001);
    }

    {
        EXPECT_EQ(errors[1].roadIndex_, 20);
        EXPECT_EQ(errors[1].segmentIndex_, 10);

        double errorStartS = errors[1].startS_;
        double errorEndS = errors[1].endS_;
        double errorMaxSlope = errors[1].maxSlope_;
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS - .001), -maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS + .001), -maxSlope);
        EXPECT_EQ(errorEndS, 1);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS), -maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(1), errorMaxSlope, 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_InclineEverywhereTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(320, 6, -3, 2));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 1);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);
        EXPECT_EQ(errors[0].startS_, 0);
        EXPECT_EQ(errors[0].endS_, 1);
        EXPECT_NEAR(errors[0].maxSlope_, segment.poly3().evalDerivative(0), 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_DeclineEverywhereTooSteep)
{
    constexpr double maxSlope = 1;

    ElevationProfile::Elevation segment(0, Poly3(320, -6, 3, -2));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 1);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);
        EXPECT_EQ(errors[0].startS_, 0);
        EXPECT_EQ(errors[0].endS_, 1);
        EXPECT_NEAR(errors[0].maxSlope_, segment.poly3().evalDerivative(0), 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_LinearSlope_InRange)
{
    // Test the case where the elevation profile uses a quadratic polynomial
    // (ie, the highest coefficient of the cubic is set to 0), which means the
    // slope is linear.

    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, -.5, .5, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(ElevationTest, testValidateMaxSlope_LinearSlope_TooLowToTooHigh)
{
    // Test the case where the elevation profile uses a quadratic polynomial
    // (ie, the highest coefficient of the cubic is set to 0), which means the
    // slope is linear.

    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, -3, 3, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 2);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);

        double errorStartS = errors[0].startS_;
        double errorEndS = errors[0].endS_;
        double errorMaxSlope = errors[0].maxSlope_;
        EXPECT_EQ(errorStartS, 0);
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS), -maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS - .001), -maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS + .001), -maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(0), errorMaxSlope, 00001);
    }

    {
        EXPECT_EQ(errors[1].roadIndex_, 20);
        EXPECT_EQ(errors[1].segmentIndex_, 10);

        double errorStartS = errors[1].startS_;
        double errorEndS = errors[1].endS_;
        double errorMaxSlope = errors[1].maxSlope_;
        EXPECT_EQ(errorEndS, 1);
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS - .001), maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS + .001), maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS), maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(1), errorMaxSlope, 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_LinearSlope_TooHighToTooLow)
{
    // Test the case where the elevation profile uses a quadratic polynomial
    // (ie, the highest coefficient of the cubic is set to 0), which means the
    // slope is linear.

    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, 3, -3, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_FALSE(res);
    EXPECT_EQ(errors.size(), 2);

    {
        EXPECT_EQ(errors[0].roadIndex_, 20);
        EXPECT_EQ(errors[0].segmentIndex_, 10);

        double errorStartS = errors[0].startS_;
        double errorEndS = errors[0].endS_;
        double errorMaxSlope = errors[0].maxSlope_;
        EXPECT_EQ(errorStartS, 0);
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS), maxSlope);
        EXPECT_GT(segment.poly3().evalDerivative(errorEndS - .001), maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS + .001), maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(0), errorMaxSlope, 00001);
    }

    {
        EXPECT_EQ(errors[1].roadIndex_, 20);
        EXPECT_EQ(errors[1].segmentIndex_, 10);

        double errorStartS = errors[1].startS_;
        double errorEndS = errors[1].endS_;
        double errorMaxSlope = errors[1].maxSlope_;
        EXPECT_EQ(errorEndS, 1);
        EXPECT_GT(segment.poly3().evalDerivative(errorStartS - .001), -maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorStartS + .001), -maxSlope);
        EXPECT_LT(segment.poly3().evalDerivative(errorEndS), -maxSlope);
        EXPECT_NEAR(segment.poly3().evalDerivative(1), errorMaxSlope, 00001);
    }
}

TEST(ElevationTest, testValidateMaxSlope_ConstantSlope_InRange)
{
    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, 0, 0, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(ElevationTest, testValidateMaxSlope_ConstantSlope_TooLow)
{
    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, -1.5, 0, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

TEST(ElevationTest, testValidateMaxSlope_ConstantSlope_TooHigh)
{
    constexpr double maxSlope = 2;

    ElevationProfile::Elevation segment(0, Poly3(100, 1.5, 0, 0));

    std::vector<ElevationProfileMaxSlopeExceeded> errors;
    bool res = validateElevationProfileSegmentSlope(segment, 20, 10, 1, maxSlope, errors);
    EXPECT_TRUE(res);
    EXPECT_EQ(errors.size(), 0);
}

}}  // namespace aid::xodr