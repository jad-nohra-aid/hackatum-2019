#pragma once

#include <string>
#include <vector>

#include <Eigen/Dense>

struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;

struct PJconsts;
typedef struct PJconsts PJ;

namespace aid { namespace xodr {

/**
 * @brief A coordinate conversion from map coordinates using one projection to
 * map coordinates using another projection.
 *
 * The projections are specified in the proj.4 format.
 */
class CoordinateConversion
{
  public:
    /**
     * @brief Constructs an uninitialized coordinate conversion.
     *
     * You have to call init() before this CoordinateConversion can be used.
     */
    CoordinateConversion() = default;
    ~CoordinateConversion();

    /**
     * @brief Initializes this coordinate conversion.
     *
     * @param fromProjStr   The projection string of the 'from' coordinate
     *                      system, in the proj.4 format.
     * @param toProjStr     The projection string of the 'to' coordinate system,
     *                      in the proj.4 format.
     */
    void init(const std::string& fromProjStr, const std::string& toProjStr);

    /**
     * @brief Converts a single point.
     *
     * @param v             The point in the 'from' coordinate system.
     * @return              The corresponding point in the 'to' coordinate system.
     */
    Eigen::Vector2d convert(const Eigen::Vector2d v) const;

    /**
     * @brief Converts all points in the given range.
     *
     * This conversion is performed in-place.
     *
     * @param points        A pointer to the first point to convert.
     * @param numPoints     The number of points to convert.
     * @param striding      The striding (in bytes) between consecutive points.
     */
    void convert(Eigen::Vector2d* points, int numPoints, int striding = sizeof(Eigen::Vector2d)) const;

    /**
     * @brief Converts all points in the given std::vector.
     *
     * This conversion is performed in-place.
     *
     * @param points        The points to convert.
     */
    void convert(std::vector<Eigen::Vector2d>& points) const;

  private:
    PJ_CONTEXT* projContext_ = nullptr;
    PJ* fromProj_ = nullptr;
    PJ* toProj_ = nullptr;
};

}}  // namespace aid::xodr
