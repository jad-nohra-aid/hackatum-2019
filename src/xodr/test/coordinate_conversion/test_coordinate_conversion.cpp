#include "coordinate_conversion/coordinate_conversion.h"

#include <gtest/gtest.h>

namespace aid { namespace sim { namespace xodr {

TEST(CoordinateConversionTest, testConvert)
{
    CoordinateConversion coordinateConversion;
    coordinateConversion.init("+proj=merc +lat_ts=56.5 +ellps=GRS80", "+proj=utm +zone=32 +ellps=WGS84");

    {
        Eigen::Vector2d res = coordinateConversion.convert(Eigen::Vector2d(3399483.80, 752085.60));
        EXPECT_NEAR(res.x(), 6103992.36, .01);
        EXPECT_NEAR(res.y(), 1924052.47, .01);
    }

    {
        Eigen::Vector2d res = coordinateConversion.convert(Eigen::Vector2d(1251637.12, 451236.11));
        EXPECT_NEAR(res.x(), 1757748.54, .01);
        EXPECT_NEAR(res.y(), 829161.88, .01);
    }

    {
        Eigen::Vector2d res = coordinateConversion.convert(Eigen::Vector2d(0, 0));
        EXPECT_NEAR(res.x(), -505646.90, .01);
        EXPECT_NEAR(res.y(), 0, .01);
    }

    {
        Eigen::Vector2d res = coordinateConversion.convert(Eigen::Vector2d(3399483.80, -752085.60));
        EXPECT_NEAR(res.x(), 6103992.36, .01);
        EXPECT_NEAR(res.y(), -1924052.47, .01);
    }
}

TEST(CoordinateConversionTest, testConvertArray)
{
    CoordinateConversion coordinateConversion;
    coordinateConversion.init("+proj=merc +lat_ts=56.5 +ellps=GRS80", "+proj=utm +zone=32 +ellps=WGS84");

    std::vector<Eigen::Vector2d> pts = {Eigen::Vector2d(3399483.80, 752085.60), Eigen::Vector2d(1251637.12, 451236.11),
                                        Eigen::Vector2d(0, 0), Eigen::Vector2d(3399483.80, -752085.60)};

    coordinateConversion.convert(pts);

    Eigen::Vector2d expected[] = {Eigen::Vector2d(6103992.36, 1924052.47), Eigen::Vector2d(1757748.54, 829161.88),
                                  Eigen::Vector2d(-505646.90, 0), Eigen::Vector2d(6103992.36, -1924052.47)};

    for (size_t i = 0; i < pts.size(); i++)
    {
        EXPECT_NEAR(pts[i].x(), expected[i].x(), .01);
        EXPECT_NEAR(pts[i].y(), expected[i].y(), .01);
    }
}

struct VecWithStriding
{
    Eigen::Vector2d pos_;
    double striding_[5];

    VecWithStriding(double x, double y) : pos_(x, y) {}
};

TEST(CoordinateConversionTest, testConvertArrayWithStriding)
{
    CoordinateConversion coordinateConversion;
    coordinateConversion.init("+proj=merc +lat_ts=56.5 +ellps=GRS80", "+proj=utm +zone=32 +ellps=WGS84");

    std::vector<VecWithStriding> pts = {VecWithStriding(3399483.80, 752085.60), VecWithStriding(1251637.12, 451236.11),
                                        VecWithStriding(0, 0), VecWithStriding(3399483.80, -752085.60)};

    coordinateConversion.convert(reinterpret_cast<Eigen::Vector2d*>(pts.data()), static_cast<int>(pts.size()),
                                 sizeof(VecWithStriding));

    VecWithStriding expected[] = {VecWithStriding(6103992.36, 1924052.47), VecWithStriding(1757748.54, 829161.88),
                                  VecWithStriding(-505646.90, 0), VecWithStriding(6103992.36, -1924052.47)};

    for (size_t i = 0; i < pts.size(); i++)
    {
        EXPECT_NEAR(pts[i].pos_.x(), expected[i].pos_.x(), .01);
        EXPECT_NEAR(pts[i].pos_.y(), expected[i].pos_.y(), .01);
    }
}

}}}  // namespace aid::sim::xodr
