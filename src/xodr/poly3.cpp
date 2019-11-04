#include "poly3.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>

namespace aid { namespace xodr {

namespace {

template <typename TCompare>
double extremeValueInInterval(const Poly3& poly, double startT, double endT)
{
    assert(startT <= endT);

    constexpr double EPSILON = 1e-6;

    constexpr TCompare compare;

    double extreme = std::max(poly.eval(startT), poly.eval(endT), compare);

    // If this is actually a quadratic function
    if (std::abs(poly.d_) < EPSILON)
    {
        // If this is actually a linear function
        if (std::abs(poly.c_) < EPSILON)
        {
            return extreme;
        }
        double root = -poly.b_ / (2 * poly.c_);
        if (root < startT || root > endT)
        {
            return extreme;
        }
        return std::max(extreme, poly.eval(root), compare);
    }

    double derivDiscSq = 4 * poly.c_ * poly.c_ - 12 * poly.d_ * poly.b_;
    if (derivDiscSq > 0)
    {
        double derivativeDiscriminant = std::sqrt(derivDiscSq);
        double rootA = (derivativeDiscriminant - 2 * poly.c_) / (6 * poly.d_);
        double rootB = (-derivativeDiscriminant - 2 * poly.c_) / (6 * poly.d_);
        if (rootA > startT && rootA < endT)
        {
            extreme = std::max(extreme, poly.eval(rootA), compare);
        }
        if (rootB > startT && rootB < endT)
        {
            extreme = std::max(extreme, poly.eval(rootB), compare);
        }
        return extreme;
    }
    else if (derivDiscSq > -EPSILON)
    {
        double root = poly.c_ / (-3 * poly.d_);
        return std::max(extreme, poly.eval(root), compare);
    }
    return extreme;
}
}  // namespace

double Poly3::maxValueInInterval(double startT, double endT) const
{
    return extremeValueInInterval<std::less<double>>(*this, startT, endT);
}

double Poly3::minValueInInterval(double startT, double endT) const
{
    return extremeValueInInterval<std::greater<double>>(*this, startT, endT);
}

Poly3 Poly3::translate(double offset) const
{
    Poly3 result;
    result.d_ = d_;
    result.c_ = (-3 * offset * d_ + c_);
    result.b_ = (3 * offset * offset * d_ - 2 * offset * c_ + b_);
    result.a_ = -offset * offset * offset * d_ + offset * offset * c_ - offset * b_ + a_;
    return result;
}

Poly3 Poly3::scale(double factor) const
{
    Poly3 result = *this;
    result.b_ *= factor;
    result.c_ *= (factor * factor);
    result.d_ *= (factor * factor * factor);
    return result;
}

}}  // namespace aid::xodr