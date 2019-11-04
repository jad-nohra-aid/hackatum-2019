#pragma once

#include <string>
#include <cmath>
#include <memory>

#include "xodr_reader.h"
#include "road_object_outline.h"

namespace aid { namespace xodr {

/**
 * @brief The RoadObject class is used to describes objects (like poles,
 * obstacles, trees) on or along the road.
 */
class RoadObject
{
  public:
    /**
     * The type of the RoadObject.
     */
    enum class Type
    {
        NONE,
        OBSTACLE,
        CAR,
        TRUCK,
        VAN,
        BUS,
        TRAILER,
        BIKE,
        MOTORBIKE,
        TRAM,
        TRAIN,
        PEDESTRIAN,
        POLE,
        TREE,
        VEGETATION,
        BARRIER,
        BUILDING,
        PARKING_SPACE,
        WIND,
        PATCH,
    };

    /**
     * The Orientation which indicates for which direction of the parent road
     * this RoadObject is valid.
     */
    enum class Orientation
    {
        /**
         * The RoadObject is valid for the positive road direction.
         */
        POSITIVE,

        /**
         * The RoadObject is valid for the negative road direction.
         */
        NEGATIVE,

        /**
         * No orientation is specified, so the RoadObject is valid in both directions.
         */
        NONE
    };

    /**
     * @brief Parses a RoadObject using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting RoadObject.
     */
    static XodrParseResult<RoadObject> parseXml(XodrReader& xml);

    /**
     * @return The type of this RoadObject.
     */
    Type type() const { return type_; }

    /**
     * @brief Gets the name of the given @ref Type as a string.
     *
     * @param type          The type.
     * @return              The type name.
     */
    static std::string typeName(Type type);

    /**
     * @return The name of this RoadObject.
     */
    const std::string& name() const { return name_; }

    /**
     * @return The id of this RoadObject.
     */
    const std::string& id() const { return id_; }

    /**
     * @return The s-coordinate of this RoadObject's origin, within the
     * s/t-coordinate system of the road which owns this Roadobject.
     */
    double s() const { return s_; }

    /**
     * @return The t-coordinate of this RoadObject's origin, within the
     * s/t-coordinate system of the road which owns this RoadObject.
     */
    double t() const { return t_; }

    /**
     * @return The z-offset of this RoadObject's origin, relative to the owning
     * road's track level.
     */
    double zOffset() const { return zOffset_; }

    /**
     * @return The extent of this RoadObject's validity along the s-axis (should
     * be 0 for point objects).
     */
    double validLength() const { return validLength_; }

    /**
     * @return The orientation (track direction) for which this RoadObject is
     * valid. See @ref Orientation for the possible values.
     */
    Orientation orientation() const { return orientation_; }

    /**
     * @name RoadObject Geometry
     *
     * The geometry of a RoadObject can be described in one of the following 3 ways:
     *   - A box (using the 'length', 'width' and 'height' attributes).
     *   - A cylinder (using the 'radius' and 'height' attributes).
     *   - An extruded outline (using an 'outline' child element).
     *
     * To determine the type of this RoadObject's geometry, use the
     * hasBoxGeometry(), hasCylinderGeometry() and hasOutlineGeometry()
     * functions. Exactly 1 of these functions is guaranteed to return true.
     *
     * @{
     */

    /**
     * Returns whether the geometry type is a 'box'. If true, then the length(),
     * width() and height() functions can be used to get the dimensions of the box.
     *
     * @return True if the geometry type is a 'box', false otherwise.
     */
    bool hasBoxGeometry() const;

    /**
     * Gets the length of this RoadObject.
     *
     * This function should only be called if hasBoxGeometry() is true.
     *
     * @return the length of the RoadObject.
     */
    double length() const;

    /**
     * Gets the width of this RoadObject.
     *
     * This function should only be called if hasBoxGeometry() is true.
     *
     * @return The width of the RoadObject.
     */
    double width() const;

    /**
     * Returns whether the geometry type is 'cylinder'. If true, then the
     * radius() and height() functions can be used to get the dimensions of the cylinder.
     *
     * @return True if the geometry type is 'cylinder', false otherwise.
     */
    bool hasCylinderGeometry() const { return !std::isnan(radius_); }

    /**
     * Gets the radius of this RoadObject.
     *
     * This function should only be called if hasCylinderGeometry() is true.
     *
     * @return The radius of the RoadObject.
     */
    double radius() const;

    /**
     * Gets the height of this RoadObject.
     *
     * This function should only be called if either hasBoxGeometry() or
     * hasCylinderGeometry() is true.
     *
     * @return The height of the RoadObject.
     */
    double height() const;

    /**
     * Returns whether the geometry type is 'outline'. If true, then the
     * outline() function can be used to get the RoadObjectOutline object.
     *
     * @return True if the geometry type is 'outline', false otherwise.
     */
    bool hasOutlineGeometry() const { return outline_ != nullptr; }

    /**
     * Gets the outline of this RoadObject.
     *
     * This function should only be called if hasOutlineGeometry() is true.
     *
     * @returns The outline of the RoadObject.
     */
    const RoadObjectOutline& outline() const;

    /**
     * @}
     */

    /**
     * @return The heading angle of the object, relative to the road's forward direction.
     */
    double heading() const { return heading_; }

    /**
     * @return The pitch angle of the object, relative to the road's pitch.
     */
    double pitch() const { return pitch_; }

    /**
     * @return The roll angle of the object, relative to the road's roll.
     */
    double roll() const { return roll_; }

  private:
    void validateGeometry(std::vector<XodrParseError>& errors);

    class AttribParsers;
    class ChildElemParsers;

    Type type_;
    std::string name_;
    std::string id_;
    double s_;
    double t_;
    double zOffset_;
    double validLength_;
    Orientation orientation_;

    double length_;
    double width_;
    double radius_;
    double height_;
    std::unique_ptr<RoadObjectOutline> outline_;

    double heading_;
    double pitch_;
    double roll_;
};

}}  // namespace aid::xodr