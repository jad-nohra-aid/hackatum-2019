#pragma once

#include "xodr/xodr_map.h"

namespace aid { namespace xodr {

/**
 * @brief A bounding rectangle.
 */
struct BoundingRect
{
    /**
     * @brief The min corner of the bounding rect.
     */
    Eigen::Vector2d min_;

    /**
     * @brief The max corner of the bounding rect.
     */
    Eigen::Vector2d max_;

    /**
     * @brief Expands the bounding rect to include the given point.
     *
     * This will set this bounding rect to the smallest bounding rect which
     * includes both the original bounding rect and the given point.
     *
     * @param pt            The point to expand this BoundingRect by.
     */
    void expand(Eigen::Vector2d pt);
};

/**
 * @brief Computes an approximation of the bounding rectangle of the given
 * xodrMap.
 *
 * @param xodrMap           The XodrMap whose bounding rect to compute.
 * @returns                 The bounding rect.
 */
BoundingRect xodrMapApproxBoundingRect(const XodrMap& xodrMap);

}}  // namespace aid::xodr