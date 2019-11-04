#include "coordinate_conversion/coordinate_conversion.h"

#include <stdexcept>
#include <cassert>
#include <proj.h>

namespace aid { namespace xodr {

void CoordinateConversion::init(const std::string& fromProjStr, const std::string& toProjStr)
{
    assert(!projContext_);

    projContext_ = proj_context_create();
    assert(projContext_);

    fromProj_ = proj_create(projContext_, fromProjStr.c_str());
    if (fromProj_ == nullptr)
    {
        std::stringstream err;
        err << "Invalid geo-reference string: " << proj_errno_string(proj_context_errno(projContext_));
        throw std::runtime_error(err.str());
    }

    toProj_ = proj_create(projContext_, toProjStr.c_str());
    if (toProj_ == nullptr)
    {
        std::stringstream err;
        err << "Invalid geo-reference string: " << proj_errno_string(proj_context_errno(projContext_));
        throw std::runtime_error(err.str());
    }
}

CoordinateConversion::~CoordinateConversion()
{
    if (fromProj_)
    {
        proj_destroy(fromProj_);
    }

    if (toProj_)
    {
        proj_destroy(toProj_);
    }

    if (projContext_)
    {
        proj_context_destroy(projContext_);
    }
}

Eigen::Vector2d CoordinateConversion::convert(const Eigen::Vector2d v) const
{
    PJ_COORD coord = proj_trans(fromProj_, PJ_INV, proj_coord(v.x(), v.y(), 0, 0));
    coord = proj_trans(toProj_, PJ_FWD, coord);
    return Eigen::Vector2d(coord.v[0], coord.v[1]);
}

void CoordinateConversion::convert(Eigen::Vector2d* points, int numPoints, int striding) const
{
    static_assert(sizeof(Eigen::Vector2d) == 2 * sizeof(double), "Eigen::Vector2d should consist of 2 double fields.");

    double* pointsDoubles = reinterpret_cast<double*>(points);

    proj_trans_generic(fromProj_, PJ_INV, pointsDoubles, striding, numPoints, pointsDoubles + 1, striding, numPoints,
                       nullptr, 0, 0, nullptr, 0, 0);

    int err = proj_errno(fromProj_);
    if (err)
    {
        throw std::runtime_error(proj_errno_string(err));
    }

    proj_trans_generic(toProj_, PJ_FWD, pointsDoubles, striding, numPoints, pointsDoubles + 1, striding, numPoints,
                       nullptr, 0, 0, nullptr, 0, 0);

    err = proj_errno(toProj_);
    if (err)
    {
        throw std::runtime_error(proj_errno_string(err));
    }
}

void CoordinateConversion::convert(std::vector<Eigen::Vector2d>& points) const
{
    convert(points.data(), static_cast<int>(points.size()));
}

}}  // namespace aid::xodr
