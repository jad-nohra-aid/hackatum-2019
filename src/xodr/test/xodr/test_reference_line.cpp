#include "reference_line.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

class TestFactory
{
  public:
    static ReferenceLine polyLineReferenceLine(std::vector<Eigen::Vector2d> points)
    {
        ReferenceLine refLine;

        double sCoord = 0;
        for (int i = 0; i < static_cast<int>(points.size()) - 1; i++)
        {
            Eigen::Vector2d startPt = points[i];
            Eigen::Vector2d endPt = points[i + 1];

            Eigen::Vector2d dir = endPt - startPt;

            ReferenceLine::Line* line = new ReferenceLine::Line();
            line->startVertex_.position_ = startPt;
            line->startVertex_.heading_ = std::atan2(dir.y(), dir.x());
            line->startVertex_.sCoord_ = sCoord;
            line->length_ = (endPt - startPt).norm();
            refLine.geometries_.emplace_back(line);

            sCoord += line->length_;
        }

        refLine.endVertex_ = refLine.geometries_.back()->endVertex();

        return refLine;
    }

    static ReferenceLine::Poly3Geom poly3(const ReferenceLine::Vertex startVertex, double length, const Poly3& poly)
    {
        ReferenceLine::Poly3Geom ret;
        ret.startVertex_ = startVertex;
        ret.length_ = length;
        ret.poly_ = poly;
        return ret;
    }

    static ReferenceLine::ParamPoly3 paramPoly3(const ReferenceLine::Vertex startVertex, double length,
                                                const Poly3& uPoly, const Poly3& vPoly, ReferenceLine::PRange pRange)
    {
        ReferenceLine::ParamPoly3 ret;
        ret.startVertex_ = startVertex;
        ret.length_ = length;
        ret.uPoly_ = uPoly;
        ret.vPoly_ = vPoly;
        ret.pRange_ = pRange;
        return ret;
    }
};

TEST(ReferenceLineTest, testLineEndVertex1)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 50;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(100, 200);
    geomAttribs.startVertex_.heading_ = 0;
    geomAttribs.length_ = 20;

    ReferenceLine::Line line;
    line.setGeometryAttribs(geomAttribs);

    ReferenceLine::Vertex endVert = line.endVertex();
    EXPECT_EQ(endVert.sCoord_, 70);
    EXPECT_EQ(endVert.position_, Eigen::Vector2d(120, 200));
    EXPECT_EQ(endVert.heading_, 0);
}

TEST(ReferenceLineTest, testLineEndVertex2)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 50;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(100, 200);
    geomAttribs.startVertex_.heading_ = M_PI / 2;
    geomAttribs.length_ = 30;

    ReferenceLine::Line line;
    line.setGeometryAttribs(geomAttribs);

    ReferenceLine::Vertex endVert = line.endVertex();
    EXPECT_EQ(endVert.sCoord_, 80);
    EXPECT_EQ(endVert.position_, Eigen::Vector2d(100, 230));
    EXPECT_EQ(endVert.heading_, M_PI / 2);
}

TEST(ReferenceLineTest, testParamPolyEndVertex)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 0;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(-12.587860206, 17.7214457346);
    geomAttribs.startVertex_.heading_ = 0.714740646259;
    geomAttribs.length_ = 52.6469056383;

    ReferenceLine::ParamPoly3 paramPoly(geomAttribs, Poly3(0, 50.9258993577, -27.4319660133, 13.5897489916),
                                        Poly3(0, -3.5527136788e-15, -69.5406376123, 34.6371411881),
                                        ReferenceLine::PRange::NORMALIZED);

    ReferenceLine::Vertex endVert = paramPoly.endVertex();
    EXPECT_DOUBLE_EQ(endVert.sCoord_, 52.646905638299998);
    EXPECT_DOUBLE_EQ(endVert.position_.x(), 38.296586354799459);
    EXPECT_DOUBLE_EQ(endVert.position_.y(), 15.665562364210516);
    EXPECT_NEAR(endVert.heading_, -0.047587471269590731, .00000001);
}

TEST(ReferenceLineTest, testTessellateIncludesEndPoints)
{
    ReferenceLine refLine = TestFactory::polyLineReferenceLine(
        {Eigen::Vector2d(0, 0), Eigen::Vector2d(100, 0), Eigen::Vector2d(100, 100), Eigen::Vector2d(0, 100)});

    {
        auto tessellation = refLine.tessellate(0, 100);
        EXPECT_EQ(tessellation.front().position_, Eigen::Vector2d(0, 0));
        EXPECT_EQ(tessellation.back().position_, Eigen::Vector2d(100, 0));

        EXPECT_NE(tessellation[tessellation.size() - 2].position_, Eigen::Vector2d(100, 0));
    }

    {
        auto tessellation = refLine.tessellate(0, 101);
        EXPECT_EQ(tessellation.front().position_, Eigen::Vector2d(0, 0));
        EXPECT_EQ(tessellation.back().position_, Eigen::Vector2d(100, 1));
    }
}

TEST(ReferenceLineTest, testPolyHeading)
{
    ReferenceLine::Vertex startVertex;
    startVertex.sCoord_ = 0;
    startVertex.position_ = Eigen::Vector2d(0, 0);
    startVertex.heading_ = 1;

    Poly3 poly(0, 0, 3.0 / (20 * 20), -2.0 / (20 * 20 * 20));

    ReferenceLine::Poly3Geom poly3 = TestFactory::poly3(startVertex, 20, poly);

    ReferenceLine::Tessellation tessellation;
    poly3.tessellate(tessellation, 0, 20, true);

    EXPECT_DOUBLE_EQ(tessellation[0].heading_, 1);

    for (int i = 1; i < static_cast<int>(tessellation.size()); i++)
    {
        const ReferenceLine::Vertex& prevVert = tessellation[i - 1];
        const ReferenceLine::Vertex& vert = tessellation[i];

        Eigen::Vector2d headingVec(std::cos(vert.heading_), std::sin(vert.heading_));

        // Compute an approximate heading vector from the positions of the
        // previous and current vertices, and use that as a reference for the
        // heading vector computed from vert.heading_.
        Eigen::Vector2d refApproxHeadingVec = vert.position_ - prevVert.position_;
        refApproxHeadingVec.normalize();

        EXPECT_NEAR(headingVec.x(), refApproxHeadingVec.x(), .05);
        EXPECT_NEAR(headingVec.y(), refApproxHeadingVec.y(), .05);
    }
}

TEST(ReferenceLineTest, testParamPolyHeading)
{
    ReferenceLine::Vertex startVertex;
    startVertex.sCoord_ = 0;
    startVertex.position_ = Eigen::Vector2d(0, 0);
    startVertex.heading_ = 1;

    Poly3 uPoly(0, 1, 0, -1.0 / 6);
    Poly3 vPoly(0, 0, -1.0 / 2, 0);

    ReferenceLine::ParamPoly3 paramPoly3 =
        TestFactory::paramPoly3(startVertex, 20, uPoly, vPoly, ReferenceLine::PRange::NORMALIZED);

    ReferenceLine::Tessellation tessellation;
    paramPoly3.tessellate(tessellation, 0, 20, true);

    EXPECT_DOUBLE_EQ(tessellation[0].heading_, 1);

    for (int i = 1; i < static_cast<int>(tessellation.size()); i++)
    {
        const ReferenceLine::Vertex& prevVert = tessellation[i - 1];
        const ReferenceLine::Vertex& vert = tessellation[i];

        Eigen::Vector2d headingVec(std::cos(vert.heading_), std::sin(vert.heading_));

        // Compute an approximate heading vector from the positions of the
        // previous and current vertices, and use that as a reference for the
        // heading vector computed from vert.heading_.
        Eigen::Vector2d refApproxHeadingVec = vert.position_ - prevVert.position_;
        refApproxHeadingVec.normalize();

        EXPECT_NEAR(headingVec.x(), refApproxHeadingVec.x(), .05);
        EXPECT_NEAR(headingVec.y(), refApproxHeadingVec.y(), .05);
    }
}

// Test the eval functions

TEST(ReferenceLineTest, testEvalLine)
{
    Eigen::Vector2d start(1, 2);
    Eigen::Vector2d end(10, -5);
    ReferenceLine::Line line(1, start, end);

    ReferenceLine::PointAndTangentDir res = line.eval(2);

    Eigen::Vector2d dir = (end - start).normalized();
    ReferenceLine::PointAndTangentDir expectedRes(start + dir, dir);

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalSpiral)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 2;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(10, 20);
    geomAttribs.startVertex_.heading_ = 1;
    geomAttribs.length_ = 100;
    ReferenceLine::Spiral spiral(geomAttribs, 1.0 / 100, 1.0 / 10);

    ReferenceLine::Tessellation tessellation;
    spiral.tessellate(tessellation, 2, 50, true);

    ReferenceLine::PointAndTangentDir res = spiral.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;
    expectedRes.point_ = Eigen::Vector2d(17.506551, 36.271387);
    expectedRes.tangentDir_ = (spiral.eval(20.001).point_ - res.point_).normalized();

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalArcPosCurvature)
{
    const Eigen::Vector2d circleCenter(4, 6);
    const double radius = 100;
    const double startAngle = 1;
    const double segmentAngle = 2;

    ReferenceLine::Arc arc = ReferenceLine::Arc::fromCircleSegment(1, circleCenter, radius, startAngle, segmentAngle);

    ReferenceLine::PointAndTangentDir res = arc.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;

    ReferenceLine::PointAndTangentDir nearbyRes = arc.eval(20.01);
    expectedRes.tangentDir_ = (nearbyRes.point_ - res.point_).normalized();

    double pointAngle = startAngle + (19 / radius);
    expectedRes.point_ = circleCenter + radius * Eigen::Vector2d(std::cos(pointAngle), std::sin(pointAngle));

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalArcNegCurvature)
{
    const Eigen::Vector2d circleCenter(4, 6);
    const double radius = 100;
    const double startAngle = 1;
    const double segmentAngle = -2;

    ReferenceLine::Arc arc = ReferenceLine::Arc::fromCircleSegment(1, circleCenter, radius, startAngle, segmentAngle);

    ReferenceLine::PointAndTangentDir res = arc.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;

    ReferenceLine::PointAndTangentDir nearbyRes = arc.eval(20.01);
    expectedRes.tangentDir_ = (nearbyRes.point_ - res.point_).normalized();

    double pointAngle = startAngle - (19 / radius);
    expectedRes.point_ = circleCenter + radius * Eigen::Vector2d(std::cos(pointAngle), std::sin(pointAngle));

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalPoly3)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 2;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(10, 20);
    geomAttribs.startVertex_.heading_ = 1;
    geomAttribs.length_ = 100;
    ReferenceLine::Poly3Geom poly3(geomAttribs, Poly3(0, 4, -2, 1));

    ReferenceLine::PointAndTangentDir res = poly3.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;
    expectedRes.point_ = Eigen::Vector2d(-4403.046055, 2874.975397);
    expectedRes.tangentDir_ = (poly3.eval(20.001).point_ - res.point_).normalized();

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalParamPoly3ArcLength)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 2;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(10, 20);
    geomAttribs.startVertex_.heading_ = 1;
    geomAttribs.length_ = 100;
    ReferenceLine::ParamPoly3 paramPoly3(geomAttribs, Poly3(0, 1, -2, 1), Poly3(0, 0, -4, .2),
                                         ReferenceLine::PRange::ARC_LENGTH);

    ReferenceLine::PointAndTangentDir res = paramPoly3.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;
    expectedRes.point_ = Eigen::Vector2d(2929.707235, 4327.308884);
    expectedRes.tangentDir_ = (paramPoly3.eval(20.001).point_ - res.point_).normalized();

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalParamPoly3Normalized)
{
    ReferenceLine::GeometryAttribs geomAttribs;
    geomAttribs.startVertex_.sCoord_ = 2;
    geomAttribs.startVertex_.position_ = Eigen::Vector2d(10, 20);
    geomAttribs.startVertex_.heading_ = 1;
    geomAttribs.length_ = 100;
    ReferenceLine::ParamPoly3 paramPoly3(geomAttribs, Poly3(0, 1, -2, 1), Poly3(0, 0, -4, .2),
                                         ReferenceLine::PRange::NORMALIZED);

    ReferenceLine::PointAndTangentDir res = paramPoly3.eval(20);

    ReferenceLine::PointAndTangentDir expectedRes;
    expectedRes.point_ = Eigen::Vector2d(10.173467, 20.032452);
    expectedRes.tangentDir_ = (paramPoly3.eval(20.001).point_ - res.point_).normalized();

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

TEST(ReferenceLineTest, testEvalManyGeoms)
{
    ReferenceLine refLine =
        ReferenceLine::fromText(
            "<planView>"
            "  <geometry s='0.0000000000000000e+00' x='-2.4987580725285643e+01' y='-2.5832091405861903e+00' "
            "hdg='1.6037544397595069e-01' length='4.3128423551701054e+01'>"
            "    <line/>"
            "  </geometry>"
            "  <geometry s='4.3128423551701054e+01' x='1.7587392963884160e+01' y='4.3039189561912785e+00' "
            "hdg='1.6037544397595105e-01' length='1.8305514916840888e+01'>"
            "    <arc curvature='-1.3541558338907550e-01'/>"
            "  </geometry>"
            "  <geometry s='6.1433938468541939e+01' x='2.4181589720294031e+01' y='-8.0071232490541568e+00' "
            "hdg='-2.3184765377254810e+00' length='1.8768541515408529e+01'>"
            "    <line/>"
            "  </geometry>"
            "  <geometry s='8.0202479983950468e+01' x='1.1420115792643021e+01' y='-2.1769497092599359e+01' "
            "hdg='-2.3184765377205143e+00' length='1.9484729793772146e+01'>"
            "    <arc curvature='1.1755308345684025e-01'/>"
            "  </geometry>"
            "  <geometry s='9.9687209777722614e+01' x='1.7419833700343691e+01' y='-3.6057067597165286e+01' "
            "hdg='-2.7986470144202080e-02' length='5.0011573820195586e+01'>"
            "    <line/>"
            "  </geometry>"
            "</planView>")
            .extract_value();

    ReferenceLine::PointAndTangentDir res = refLine.eval(85);

    ReferenceLine::PointAndTangentDir expectedRes;
    expectedRes.point_ = Eigen::Vector2d(9.294240, -25.999538);
    expectedRes.tangentDir_ = (refLine.eval(85.001).point_ - res.point_).normalized();

    EXPECT_NEAR(res.point_.x(), expectedRes.point_.x(), .0001);
    EXPECT_NEAR(res.point_.y(), expectedRes.point_.y(), .0001);
    EXPECT_NEAR(res.tangentDir_.x(), expectedRes.tangentDir_.x(), .0001);
    EXPECT_NEAR(res.tangentDir_.y(), expectedRes.tangentDir_.y(), .0001);
}

// Test the evalCurvature functions

TEST(ReferenceLineTest, testEvalLineCurvature)
{
    Eigen::Vector2d start(1, 2);
    Eigen::Vector2d end(10, -5);
    ReferenceLine::Line line(0, start, end);
    EXPECT_EQ(line.evalCurvature(5), 0);
}

TEST(ReferenceLineTest, testEvalSpiralCurvature)
{
    ReferenceLine::GeometryAttribs geomAttribs{};
    geomAttribs.length_ = 100;
    ReferenceLine::Spiral spiral(geomAttribs, 1.0 / 100, 1.0 / 10);

    EXPECT_NEAR(spiral.evalCurvature(0), 1.0 / 100, 0.0001);
    EXPECT_NEAR(spiral.evalCurvature(50), 1.0 / 100 + 0.5 * (1.0 / 10 - 1.0 / 100), 0.0001);
    EXPECT_NEAR(spiral.evalCurvature(100), 1.0 / 10, 0.0001);
}

TEST(ReferenceLineTest, testEvalArcCurvature)
{
    {
        ReferenceLine::Arc arc(ReferenceLine::Vertex{}, 100, 0.001);
        EXPECT_EQ(arc.evalCurvature(0), 0.001);
        EXPECT_EQ(arc.evalCurvature(50), 0.001);
        EXPECT_EQ(arc.evalCurvature(100), 0.001);
    }
    {
        ReferenceLine::Arc arc(ReferenceLine::Vertex{}, 100, -0.001);
        EXPECT_EQ(arc.evalCurvature(0), -0.001);
        EXPECT_EQ(arc.evalCurvature(50), -0.001);
        EXPECT_EQ(arc.evalCurvature(100), -0.001);
    }
}

TEST(ReferenceLineTest, testEvalPoly3Curvature)
{
    ReferenceLine::GeometryAttribs geomAttribs{};
    geomAttribs.length_ = 100;
    EXPECT_NEAR(ReferenceLine::Poly3Geom(geomAttribs, Poly3(0, -4, 0, 1)).evalCurvature(0), 0, .0001);
    EXPECT_NEAR(ReferenceLine::Poly3Geom(geomAttribs, Poly3(0, 2, -1, .02)).evalCurvature(3), -0.0351, .0001);
}

TEST(ReferenceLineTest, testEvalParamPoly3ArcLengthCurvature)
{
    ReferenceLine::GeometryAttribs geomAttribs{};
    geomAttribs.length_ = 100;
    ReferenceLine::ParamPoly3 paramPoly3(geomAttribs, Poly3(0, 1, -2, 1), Poly3(0, 0, -4, .2),
                                         ReferenceLine::PRange::ARC_LENGTH);

    EXPECT_NEAR(paramPoly3.evalCurvature(0), -8, 0.0001);
    EXPECT_NEAR(paramPoly3.evalCurvature(0.25), -0.82875, 0.0001);
}

TEST(ReferenceLineTest, testEvalParamPoly3NormalizedCurvature)
{
    ReferenceLine::GeometryAttribs geomAttribs{};
    geomAttribs.length_ = 100;
    ReferenceLine::ParamPoly3 paramPoly3(geomAttribs, Poly3(0, 1, -2, 1), Poly3(0, 0, -4, .2),
                                         ReferenceLine::PRange::NORMALIZED);

    EXPECT_NEAR(paramPoly3.evalCurvature(0), -8, 0.0001);
    EXPECT_NEAR(paramPoly3.evalCurvature(25), -0.82875, 0.0001);
}

}}  // namespace aid::xodr