#include <gtest/gtest.h>

#include "xodr_validation/polynomial.h"

#include "../test_config.h"
namespace aid { namespace xodr {

TEST(PolynomialTest, testAddPolynomials)
{
    Polynomial a({0.0, 1.0, 2.0});
    Polynomial b({-5.0, 10.0});
    Polynomial c({-5.0});
    Polynomial ab = a + b;
    Polynomial ba = b + a;
    Polynomial ac = a + c;
    Polynomial ca = c + a;
    Polynomial bc = b + c;
    Polynomial cb = c + b;
    Polynomial abc = ab + c;
    Polynomial cba = cb + a;
    EXPECT_EQ(ab, Polynomial({-5.0, 11.0, 2.0}));
    EXPECT_EQ(ab, ba);
    EXPECT_EQ(bc, Polynomial({-10.0, 10.0}));
    EXPECT_EQ(bc, cb);
    EXPECT_EQ(ac, ca);
    EXPECT_EQ(abc, Polynomial({-10.0, 11.0, 2.0}));
    EXPECT_EQ(abc, cba);
}

TEST(PolynomialTest, testMultiplyBySingleTerm)
{
    Polynomial a({0.0, 1.0, 2.0});
    Polynomial b({-5.0, 10.0});
    Polynomial c{};
    EXPECT_EQ(a.multiplyBySingleTermPolynomial(2.0, 2), Polynomial({0.0, 0.0, 0.0, 2.0, 4.0}));
    EXPECT_EQ(b.multiplyBySingleTermPolynomial(-1.0, 0), Polynomial({5.0, -10.0}));
    EXPECT_EQ(c.multiplyBySingleTermPolynomial(0.0, 5), Polynomial({0.0, 0.0, 0.0, 0.0, 0.0}));
}

TEST(PolynomialTest, testNegate)
{
    Polynomial a({0.0, 1.0, 2.0});
    EXPECT_EQ(-a, Polynomial({0.0, -1.0, -2.0}));
}

TEST(PolynomialTest, testOperatorSubtract)
{
    Polynomial a({0.0, 1.0, 2.0});
    Polynomial b({-5.0, 10.0});
    a -= b;
    EXPECT_EQ(a, Polynomial({5.0, -9.0, 2.0}));
}

TEST(PolynomialTest, testEval)
{
    Polynomial a({0, 1, 2});
    Polynomial b({5, 0, -2});
    EXPECT_EQ(a.eval(-2.0), 6);
    EXPECT_EQ(a.eval(-1.0), 1);
    EXPECT_EQ(a.eval(0.0), 0);
    EXPECT_EQ(a.eval(3.0), 21.0);
    EXPECT_EQ(b.eval(-2.0), -3);
    EXPECT_EQ(b.eval(-1.0), 3);
    EXPECT_EQ(b.eval(0.0), 5);
    EXPECT_EQ(b.eval(3.0), -13);
}

TEST(PolynomialTest, testDerivative)
{
    Polynomial a({0, 1, 2});
    Polynomial b({5, 1, -2});
    Polynomial c;
    EXPECT_EQ(a.derivative(), Polynomial({1, 4}));
    EXPECT_EQ(b.derivative(), Polynomial({1, -4}));
    EXPECT_EQ(c.derivative(), c);
}

TEST(PolynomialTest, testHasRootsInInterval)
{
    Polynomial a({-1, 0, 1});
    EXPECT_TRUE(a.hasRootsInInterval(-2, 2));
    EXPECT_TRUE(a.hasRootsInInterval(0.99, 1.01));
    EXPECT_FALSE(a.hasRootsInInterval(1.01, 100));
    Polynomial b({1, 0, -1});
    EXPECT_TRUE(b.hasRootsInInterval(0, 2));
    EXPECT_TRUE(b.hasRootsInInterval(0.99, 1.01));
    EXPECT_FALSE(b.hasRootsInInterval(1.01, 100));
    Polynomial c({0, 0, 1});
    EXPECT_TRUE(c.hasRootsInInterval(-0.01, 0.01));
}
}}  // namespace aid::xodr
