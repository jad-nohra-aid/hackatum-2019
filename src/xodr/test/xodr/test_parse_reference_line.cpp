#include "reference_line.h"

#include <gtest/gtest.h>

#include "xml/xml_reader.h"

namespace aid { namespace xodr {

TEST(ParseReferenceLineTest, testParseEmptyPlanView)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "</planView>");

    xml.readStartElement("planView");
    EXPECT_FALSE(ReferenceLine::parseXml(xml).hasValidGeometry());
}

TEST(ParseReferenceLineTest, testParseLine)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <line/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::Line& line = static_cast<const ReferenceLine::Line&>(referenceLine.geometry(0));
    EXPECT_EQ(line.startVertex().sCoord_, 1);
    EXPECT_EQ(line.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(line.startVertex().heading_, 5.4);
    EXPECT_EQ(line.length(), 100);
}

TEST(ParseReferenceLineTest, testParseZeroLengthLine)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '0'>"
        "    <line/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    EXPECT_FALSE(ReferenceLine::parseXml(xml).hasValidGeometry());
}

TEST(ParseReferenceLineTest, testParseSpiral)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <spiral curvStart = '.1' curvEnd = '.2'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::Spiral& spiral = static_cast<const ReferenceLine::Spiral&>(referenceLine.geometry(0));
    EXPECT_EQ(spiral.startVertex().sCoord_, 1);
    EXPECT_EQ(spiral.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(spiral.startVertex().heading_, 5.4);
    EXPECT_EQ(spiral.length(), 100);
    EXPECT_EQ(spiral.startCurvature(), .1);
    EXPECT_EQ(spiral.endCurvature(), .2);
}

TEST(ParseReferenceLineTest, testParseSpiralZeroRateOfChange)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <spiral curvStart = '.1' curvEnd = '.1'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    EXPECT_FALSE(ReferenceLine::parseXml(xml).hasValidGeometry());
}

TEST(ParseReferenceLineTest, testParseArc)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <arc curvature= '.1'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::Arc& arc = static_cast<const ReferenceLine::Arc&>(referenceLine.geometry(0));
    EXPECT_EQ(arc.startVertex().sCoord_, 1);
    EXPECT_EQ(arc.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(arc.startVertex().heading_, 5.4);
    EXPECT_EQ(arc.length(), 100);
    EXPECT_EQ(arc.curvature(), .1);
}

TEST(ParseReferenceLineTest, testParseArcZeroCurvature)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <arc curvature= '0'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    EXPECT_FALSE(ReferenceLine::parseXml(xml).hasValidGeometry());
}

TEST(ParseReferenceLineTest, testParsePoly3)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <poly3 a = '5' b = '6' c = '7' d = '8'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::Poly3Geom& poly3 = static_cast<const ReferenceLine::Poly3Geom&>(referenceLine.geometry(0));
    EXPECT_EQ(poly3.startVertex().sCoord_, 1);
    EXPECT_EQ(poly3.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(poly3.startVertex().heading_, 5.4);
    EXPECT_EQ(poly3.length(), 100);
    EXPECT_EQ(poly3.poly(), Poly3(5, 6, 7, 8));
}

TEST(ParseReferenceLineTest, testParseParamPoly3ArcLength)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <paramPoly3 "
        "      aU = '5' bU = '6' cU = '7' dU = '8'"
        "      aV = '15' bV = '16' cV = '17' dV = '18'"
        "      pRange = 'arcLength'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::ParamPoly3& paramPoly3 =
        static_cast<const ReferenceLine::ParamPoly3&>(referenceLine.geometry(0));
    EXPECT_EQ(paramPoly3.startVertex().sCoord_, 1);
    EXPECT_EQ(paramPoly3.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(paramPoly3.startVertex().heading_, 5.4);
    EXPECT_EQ(paramPoly3.length(), 100);
    EXPECT_EQ(paramPoly3.uPoly(), Poly3(5, 6, 7, 8));
    EXPECT_EQ(paramPoly3.vPoly(), Poly3(15, 16, 17, 18));
    EXPECT_EQ(paramPoly3.pRange(), ReferenceLine::PRange::ARC_LENGTH);
}

TEST(ParseReferenceLineTest, testParseParamPoly3Normalized)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <paramPoly3 "
        "      aU = '5' bU = '6' cU = '7' dU = '8'"
        "      aV = '15' bV = '16' cV = '17' dV = '18'"
        "      pRange = 'normalized'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::ParamPoly3& paramPoly3 =
        static_cast<const ReferenceLine::ParamPoly3&>(referenceLine.geometry(0));
    EXPECT_EQ(paramPoly3.startVertex().sCoord_, 1);
    EXPECT_EQ(paramPoly3.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(paramPoly3.startVertex().heading_, 5.4);
    EXPECT_EQ(paramPoly3.length(), 100);
    EXPECT_EQ(paramPoly3.uPoly(), Poly3(5, 6, 7, 8));
    EXPECT_EQ(paramPoly3.vPoly(), Poly3(15, 16, 17, 18));
    EXPECT_EQ(paramPoly3.pRange(), ReferenceLine::PRange::NORMALIZED);
}

TEST(ParseReferenceLineTest, testParseParamPoly3DefaultToNormalized)
{
    XodrReader xml = XodrReader::fromText(
        "<planView>"
        "  <geometry s = '1' x = '-7' y = '6' hdg = '5.4' length = '100'>"
        "    <paramPoly3 "
        "      aU = '5' bU = '6' cU = '7' dU = '8'"
        "      aV = '15' bV = '16' cV = '17' dV = '18'/>"
        "  </geometry>"
        "</planView>");

    xml.readStartElement("planView");
    ReferenceLine referenceLine = ReferenceLine::parseXml(xml).value();

    EXPECT_EQ(referenceLine.numGeometries(), 1);

    const ReferenceLine::ParamPoly3& paramPoly3 =
        static_cast<const ReferenceLine::ParamPoly3&>(referenceLine.geometry(0));
    EXPECT_EQ(paramPoly3.startVertex().sCoord_, 1);
    EXPECT_EQ(paramPoly3.startVertex().position_, Eigen::Vector2d(-7, 6));
    EXPECT_EQ(paramPoly3.startVertex().heading_, 5.4);
    EXPECT_EQ(paramPoly3.length(), 100);
    EXPECT_EQ(paramPoly3.uPoly(), Poly3(5, 6, 7, 8));
    EXPECT_EQ(paramPoly3.vPoly(), Poly3(15, 16, 17, 18));
    EXPECT_EQ(paramPoly3.pRange(), ReferenceLine::PRange::NORMALIZED);
}

}}  // namespace aid::xodr
