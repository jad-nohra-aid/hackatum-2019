#include "poly3.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(Poly3Test, testCtor)
{
    Poly3 f(1, 2, 3, 4);
    EXPECT_EQ(f.a_, 1);
    EXPECT_EQ(f.b_, 2);
    EXPECT_EQ(f.c_, 3);
    EXPECT_EQ(f.d_, 4);
}

TEST(Poly3Test, testCtor2)
{
    Poly3 f(2367278, 347834.56313, -.00687303427, -134820);
    EXPECT_EQ(f.a_, 2367278);
    EXPECT_EQ(f.b_, 347834.56313);
    EXPECT_EQ(f.c_, -.00687303427);
    EXPECT_EQ(f.d_, -134820);
}

TEST(Poly3Test, testEqOp)
{
    EXPECT_TRUE(Poly3(1, 2, 3, 4) == Poly3(1, 2, 3, 4));

    EXPECT_FALSE(Poly3(1, 2, 3, 4) == Poly3(0, 2, 3, 4));
    EXPECT_FALSE(Poly3(1, 2, 3, 4) == Poly3(1, 0, 3, 4));
    EXPECT_FALSE(Poly3(1, 2, 3, 4) == Poly3(1, 2, 0, 4));
    EXPECT_FALSE(Poly3(1, 2, 3, 4) == Poly3(1, 2, 3, 0));
}

TEST(Poly3Test, testEqOp2)
{
    EXPECT_TRUE(Poly3(2367278, 347834.56313, -.00687303427, -134820) ==
                Poly3(2367278, 347834.56313, -.00687303427, -134820));

    EXPECT_FALSE(Poly3(2367278, 347834.56313, -.00687303427, -134820) ==
                 Poly3(0, 347834.56313, -.00687303427, -134820));
    EXPECT_FALSE(Poly3(2367278, 347834.56313, -.00687303427, -134820) == Poly3(2367278, 0, -.00687303427, -134820));
    EXPECT_FALSE(Poly3(2367278, 347834.56313, -.00687303427, -134820) == Poly3(2367278, 347834.56313, 0, -134820));
    EXPECT_FALSE(Poly3(2367278, 347834.56313, -.00687303427, -134820) ==
                 Poly3(2367278, 347834.56313, -.00687303427, 0));
}

TEST(Poly3Test, testNEqOp)
{
    EXPECT_FALSE(Poly3(1, 2, 3, 4) != Poly3(1, 2, 3, 4));

    EXPECT_TRUE(Poly3(1, 2, 3, 4) != Poly3(0, 2, 3, 4));
    EXPECT_TRUE(Poly3(1, 2, 3, 4) != Poly3(1, 0, 3, 4));
    EXPECT_TRUE(Poly3(1, 2, 3, 4) != Poly3(1, 2, 0, 4));
    EXPECT_TRUE(Poly3(1, 2, 3, 4) != Poly3(1, 2, 3, 0));
}

TEST(Poly3Test, testNEqOp2)
{
    EXPECT_FALSE(Poly3(2367278, 347834.56313, -.00687303427, -134820) !=
                 Poly3(2367278, 347834.56313, -.00687303427, -134820));

    EXPECT_TRUE(Poly3(2367278, 347834.56313, -.00687303427, -134820) != Poly3(0, 347834.56313, -.00687303427, -134820));
    EXPECT_TRUE(Poly3(2367278, 347834.56313, -.00687303427, -134820) != Poly3(2367278, 0, -.00687303427, -134820));
    EXPECT_TRUE(Poly3(2367278, 347834.56313, -.00687303427, -134820) != Poly3(2367278, 347834.56313, 0, -134820));
    EXPECT_TRUE(Poly3(2367278, 347834.56313, -.00687303427, -134820) != Poly3(2367278, 347834.56313, -.00687303427, 0));
}

TEST(Poly3Test, testEval)
{
    Poly3 f(1, 2, 3, 4);
    EXPECT_EQ(f.eval(0), 1);
    EXPECT_EQ(f.eval(1), 10);
    EXPECT_EQ(f.eval(2), 49);
    EXPECT_EQ(f.eval(3), 142);
}

TEST(Poly3Test, testEvalDerivative)
{
    Poly3 f(1, 2, 3, 4);
    EXPECT_EQ(f.evalDerivative(0), 2);
    EXPECT_EQ(f.evalDerivative(1), 20);
    EXPECT_EQ(f.evalDerivative(2), 62);
    EXPECT_EQ(f.evalDerivative(3), 128);
}

TEST(Poly3Test, test2nEvalDerivative)
{
    Poly3 f(1, 2, 3, 4);
    EXPECT_EQ(f.eval2ndDerivative(0), 6);
    EXPECT_EQ(f.eval2ndDerivative(1), 30);
    EXPECT_EQ(f.eval2ndDerivative(2), 54);
    EXPECT_EQ(f.eval2ndDerivative(3), 78);
}

TEST(Poly3Test, testAntiDerivative1)
{
    Poly3 f(7, 2, -3, 5);
    EXPECT_EQ(f.evalAntiDerivative(0), 0);
    EXPECT_EQ(f.evalAntiDerivative(1), 33.0 / 4);
    EXPECT_EQ(f.evalAntiDerivative(2), 30);
    EXPECT_EQ(f.evalAntiDerivative(3), 417.0 / 4);
}

static void testTranslateImpl(const Poly3& testPoly3)
{
    Poly3 translated = testPoly3.translate(10.0);
    EXPECT_NEAR(testPoly3.eval(2.5), translated.eval(12.5), 0.001);
    EXPECT_NEAR(testPoly3.eval(-2.5), translated.eval(7.5), 0.001);
    EXPECT_NEAR(testPoly3.eval(-10.5), translated.eval(-0.5), 0.001);
}

TEST(Poly3Test, testTranslatePoly3)
{
    testTranslateImpl(Poly3(0.0, 0.0, 0.0, 1.0));
    testTranslateImpl(Poly3(-1.0, 2.3, 2.6, -0.5));
    testTranslateImpl(Poly3(3.0, -2.3, -2.8, 0.0));
}

static void testScaleImpl(const Poly3& testPoly3)
{
    Poly3 scaled = testPoly3.scale(10.0);
    EXPECT_NEAR(testPoly3.eval(25), scaled.eval(2.5), 0.001);
    EXPECT_NEAR(testPoly3.eval(-25), scaled.eval(-2.5), 0.001);
    EXPECT_NEAR(testPoly3.eval(-105), scaled.eval(-10.5), 0.001);
}

TEST(Poly3Test, testScalePoly3)
{
    testScaleImpl(Poly3(0.0, 0.0, 0.0, 1.0));
    testScaleImpl(Poly3(-1.0, 2.3, 2.6, -0.5));
    testScaleImpl(Poly3(3.0, -2.3, -2.8, 0.0));
}

TEST(Poly3Test, testMaxValueInInterval)
{
    EXPECT_EQ(Poly3(2.0, 1.0, 0.0, 0.0).maxValueInInterval(0, 1), 3);
    EXPECT_EQ(Poly3(2.0, -1.0, 0.0, 0.0).maxValueInInterval(0, 1), 2);
    EXPECT_EQ(Poly3(2.0, -1.0, 1.0, 0.0).maxValueInInterval(-1, 1), 4);
    Poly3 testPoly(7.5346346, 2.32, -2.213, 0.5);
    EXPECT_NEAR(testPoly.maxValueInInterval(0, 1), 8.2462, 0.0001);
    EXPECT_NEAR(testPoly.maxValueInInterval(0, 0.5), 8.20388, 0.0001);
    EXPECT_NEAR(testPoly.maxValueInInterval(1, 4), 13.4066, 0.0001);
}

TEST(Poly3Test, testMinValueInInterval)
{
    EXPECT_EQ(Poly3(-2.0, -1.0, 0.0, 0.0).minValueInInterval(0, 1), -3);
    EXPECT_EQ(Poly3(-2.0, 1.0, 0.0, 0.0).minValueInInterval(0, 1), -2);
    EXPECT_EQ(Poly3(-2.0, 1.0, -1.0, 0.0).minValueInInterval(-1, 1), -4);
    Poly3 testPoly(-7.5346346, -2.32, 2.213, -0.5);
    EXPECT_NEAR(testPoly.minValueInInterval(0, 1), -8.2462, 0.0001);
    EXPECT_NEAR(testPoly.minValueInInterval(0, 0.5), -8.20388, 0.0001);
    EXPECT_NEAR(testPoly.minValueInInterval(1, 4), -13.4066, 0.0001);
}

}}  // namespace aid::xodr