#include "bounding_rect.h"

#include <cfloat>

namespace aid { namespace xodr {

void BoundingRect::expand(Eigen::Vector2d pt)
{
    if (pt.x() < min_.x()) min_.x() = pt.x();
    if (pt.y() < min_.y()) min_.y() = pt.y();

    if (pt.x() > max_.x()) max_.x() = pt.x();
    if (pt.y() > max_.y()) max_.y() = pt.y();
}

BoundingRect xodrMapApproxBoundingRect(const XodrMap& xodrMap)
{
    BoundingRect ret;
    ret.min_ = Eigen::Vector2d(DBL_MAX, DBL_MAX);
    ret.max_ = Eigen::Vector2d(-DBL_MAX, -DBL_MAX);

    for (const Road& road : xodrMap.roads())
    {
        const ReferenceLine& refLine = road.referenceLine();
        for (int i = 0; i < refLine.numGeometries(); i++)
        {
            ret.expand(refLine.geometry(i).startVertex().position_);
        }

        ret.expand(refLine.endVertex().position_);
    }

    return ret;
}

}}  // namespace aid::xodr