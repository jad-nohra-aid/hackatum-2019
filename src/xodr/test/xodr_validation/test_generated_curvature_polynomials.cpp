#include <gtest/gtest.h>

#include "xodr_validation/generated_curvature_polynomials.h"

#include "../test_config.h"

namespace aid { namespace xodr {

static int sign(double d)
{
    if (d > 0)
    {
        return 1;
    }
    if (d < 0)
    {
        return -1;
    }
    return 0;
}

TEST(GeneratedCurvaturePolynomialsTest, testPoly3GeomCurvatureRadiusVersusDistance)
{
    {
        Poly3 referenceLinePoly(0, 0.012, -0.02, 0.0167);
        ReferenceLine::Poly3Geom referenceLine(ReferenceLine::Vertex{}, 10, referenceLinePoly);
        Poly3 distancePoly(5.0, 0.12, -0.3, 0.22);
        Polynomial p = curvatureRadiusVersusDistance(referenceLine, distancePoly);
        Polynomial reference = curvatureRadiusVersusDistance(referenceLine, Poly3{});
        EXPECT_EQ(p.degree(), 12);
        for (double s : {0.1, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0})
        {
            double absCurvature = std::abs(referenceLine.evalCurvature(s));
            double radiusMinusDistance = 1.0 / absCurvature - distancePoly.eval(s);
            double pVal = p.eval(s);
            EXPECT_EQ(sign(radiusMinusDistance), sign(pVal))
                << radiusMinusDistance << " has different sign than " << pVal;
            EXPECT_GT(reference.eval(s), 0);
        }
    }
    {
        Poly3 referenceLinePoly(0, 0.012, -0.02, 0);
        ReferenceLine::Poly3Geom referenceLine(ReferenceLine::Vertex{}, 10, referenceLinePoly);
        Poly3 distancePoly(5.0, 0.12, -0.3, 0.22);
        Polynomial p = curvatureRadiusVersusDistance(referenceLine, distancePoly);
        EXPECT_EQ(p.degree(), 6);  //((ax+b)^2)^3 - (px^3+qx^2+rx+s)^2
        for (double s : {0.1, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0})
        {
            double absCurvature = std::abs(referenceLine.evalCurvature(s));
            double radiusMinusDistance = 1.0 / absCurvature - distancePoly.eval(s);
            double pVal = p.eval(s);
            EXPECT_EQ(sign(radiusMinusDistance), sign(pVal))
                << radiusMinusDistance << " has different sign than " << pVal;
        }
    }
}

TEST(GeneratedCurvaturePolynomialsTest, testParamPoly3CurvatureRadiusVersusDistance)
{
    ReferenceLine::ParamPoly3 referenceLine(
        ReferenceLine::Vertex{}, 10, Poly3(0, 0.9258993577, -0.4319660133, 0.5897489916),
        Poly3(0, -3.5527136788e-15, -0.5406376123, 0.6371411881), ReferenceLine::PRange::NORMALIZED);
    Poly3 distancePoly = Poly3(3.0, 0.12, -0.3, 0.12).scale(10);
    Polynomial p = curvatureRadiusVersusDistance(referenceLine, distancePoly);
    Polynomial reference = curvatureRadiusVersusDistance(referenceLine, Poly3{});
    EXPECT_EQ(p.degree(), 12);
    for (double s : {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9})
    {
        double absCurvature = std::abs(referenceLine.evalCurvature(s * 10));
        double radiusMinusDistance = 1.0 / absCurvature - distancePoly.eval(s);
        double pVal = p.eval(s);
        EXPECT_EQ(sign(radiusMinusDistance), sign(pVal)) << radiusMinusDistance << " has different sign than " << pVal;
        EXPECT_GT(reference.eval(s), 0);
    }
}

TEST(GeneratedCurvaturePolynomialsTest, testSpiralCurvatureRadiusVersusDistance)
{
    ReferenceLine::Spiral referenceLine(ReferenceLine::Vertex{}, 10, 0.1, -0.1);
    Poly3 distancePoly(4.0, -0.02, 0.035, 0.01);
    Polynomial reference = curvatureRadiusVersusDistance(referenceLine, Poly3{});
    Polynomial p = curvatureRadiusVersusDistance(referenceLine, distancePoly);
    EXPECT_EQ(p.degree(), 8);
    for (double s : {0.1, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0})
    {
        double absCurvature = std::abs(referenceLine.evalCurvature(s));
        double radiusMinusDistance = 1.0 / absCurvature - distancePoly.eval(s);
        double pVal = p.eval(s);
        EXPECT_EQ(sign(radiusMinusDistance), sign(pVal)) << radiusMinusDistance << " has different sign than " << pVal;
        EXPECT_GT(reference.eval(s), 0);
    }
}

TEST(GeneratedCurvaturePolynomialsTest, testZeroCurvatureChangeSpiral)
{
    ReferenceLine::Spiral referenceLine(ReferenceLine::Vertex{}, 10, 0.1, 0.1);
    Poly3 distancePoly(4.0, -0.02, 0.035, 0.01);
    Polynomial reference = curvatureRadiusVersusDistance(referenceLine, Poly3{});
    Polynomial p = curvatureRadiusVersusDistance(referenceLine, distancePoly);
    EXPECT_EQ(p.degree(), 6);
    for (double s : {0.1, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0})
    {
        double absCurvature = std::abs(referenceLine.evalCurvature(s));
        double radiusMinusDistance = 1.0 / absCurvature - distancePoly.eval(s);
        double pVal = p.eval(s);
        EXPECT_EQ(sign(radiusMinusDistance), sign(pVal)) << radiusMinusDistance << " has different sign than " << pVal;
        EXPECT_GT(reference.eval(s), 0);
    }
}

TEST(GeneratedCurvaturePolynomialsTest, testPoly3GeomInflectionPoints)
{
    Poly3 referenceLinePoly(0, 0.012, -0.02, 0.0167);
    ReferenceLine::Poly3Geom referenceLine(ReferenceLine::Vertex{}, 10, referenceLinePoly);
    EXPECT_NEAR(referenceLinePoly.eval2ndDerivative(inflectionPoints(referenceLine)[0]), 0, 0.0001);
    Poly3 noInflectionsPoly(1, 0.5, 0.02, 0.0);
    ReferenceLine::Poly3Geom noInflectionsReferenceLine(ReferenceLine::Vertex{}, 10, noInflectionsPoly);
    EXPECT_TRUE(std::isinf(inflectionPoints(noInflectionsReferenceLine)[0]));
}

TEST(GeneratedCurvaturePolynomialsTest, testParamPoly3InflectionPoints)
{
    ReferenceLine::ParamPoly3 referenceLine(
        ReferenceLine::Vertex{}, 10, Poly3(0, 0.9258993577, -0.4319660133, 0.5897489916),
        Poly3(0, -3.5527136788e-15, -0.5406376123, 0.6371411881), ReferenceLine::PRange::NORMALIZED);
    for (auto inflectionPoint : inflectionPoints(referenceLine))
    {
        double evaluatedDerivatives = referenceLine.uPoly().evalDerivative(inflectionPoint) *
                                          referenceLine.vPoly().eval2ndDerivative(inflectionPoint) -
                                      referenceLine.vPoly().evalDerivative(inflectionPoint) *
                                          referenceLine.uPoly().eval2ndDerivative(inflectionPoint);
        EXPECT_NEAR(evaluatedDerivatives, 0, 0.0001);
    }
}

}}  // namespace aid::xodr
