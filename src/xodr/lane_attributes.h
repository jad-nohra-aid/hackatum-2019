#pragma once

#include <string>

#include "xodr_reader.h"
#include "units.h"

namespace aid { namespace xodr {

/**
 * @brief The material of (a cross-section of) a lane.
 */
class LaneMaterial
{
  public:
    /**
     * @brief Creates an uninitialized LaneMaterial.
     */
    LaneMaterial() = default;

    /**
     * @brief Parses a LaneMaterial using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneMaterial> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneMaterial becomes active.
     */
    double sOffset() const { return sOffset_; };

    /**
     * @returns The surface material.
     */
    const std::string& surface() const { return surface_; }

    /**
     * @returns The friction of the material.
     */
    double friction() const { return friction_; }

    /**
     * @returns The roughness of the material.
     */
    double roughness() const { return roughness_; }

  private:
    class AttribParsers;

    double sOffset_;
    std::string surface_;
    double friction_;
    double roughness_;
};

/**
 * @brief The visibility information of (a cross-section of) a lane.
 */
class LaneVisibility
{
  public:
    /**
     * @brief Creates an uninitialized LaneVisibility.
     */
    LaneVisibility() = default;

    /**
     * @brief Parses a LaneVisibility using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneVisibility> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneVisibility becomes active.
     */
    double sOffset() const { return sOffset_; }

    /**
     * @brief Returns the visibility distance (in meters) in the forward direction.
     *
     * The forward direction is the forward direction of the lane. For left
     * lanes (assuming right-hand traffic), this direction is opposite to the
     * road's forward direction, for right lanes, it's the same.
     *
     * @returns The forward visibility distance.
     */
    double forward() const { return forward_; }

    /**
     * @brief Returns the visibility distance (in meters) in the backward direction.
     *
     * The backward direction is the backward direction of the lane. For left
     * lanes (assuming right-hand traffic), this direction is opposite to the
     * road's backward direction, for right lanes, it's the same.
     *
     * @returns The backward visibility distance.
     */
    double back() const { return back_; }

    /**
     * @returns The visibility distance (in meters) in the left direction.
     */
    double left() const { return left_; }

    /**
     * @returns The visibility distance (in meters) in the right direction.
     */
    double right() const { return right_; }

  private:
    class AttribParsers;

    double sOffset_;
    double forward_;
    double back_;
    double left_;
    double right_;
};

/**
 * @brief The speed limit of (a cross-section of) a lane.
 */
class LaneSpeedLimit
{
  public:
    /**
     * @brief Creates an uninitialized LaneSpeedLimit.
     */
    LaneSpeedLimit() = default;

    /**
     * @brief Parses a LaneSpeedLimit using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneSpeedLimit> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneSpeedLimit becomes active.
     */
    double sOffset() const { return sOffset_; }

    /**
     * @brief Returns the speed limit, expressed in the unit returned by the
     * unit() function.
     *
     * If unit() returns SpeedUnit::NOT_SPECIFIED, then it's in meters per second.
     *
     * @returns The speed limit.
     */
    double maxSpeed() const { return maxSpeed_; }

    /**
     * @return The unit used to express the speed limit.
     */
    SpeedUnit unit() const { return unit_; }

  private:
    class AttribParsers;

    double sOffset_;
    double maxSpeed_;
    SpeedUnit unit_;
};

/**
 * @brief The access restriction of (a cross-section of) a lane.
 */
class LaneAccess
{
  public:
    /**
     * @brief Creates an uninitialized LaneAccess.
     */
    LaneAccess() = default;

    /**
     * @brief Parses a LaneAccess using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneAccess> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneAccess becomes active.
     */
    double sOffset() const { return sOffset_; }

    /**
     * @returns The access restriction.
     */
    const std::string& restriction() const { return restriction_; }

  private:
    class AttribParsers;

    double sOffset_;
    std::string restriction_;
};

/**
 * @brief The height of (a cross-section of) a lane.
 */
class LaneHeight
{
  public:
    /**
     * @brief Creates an uninitialized LaneHeight.
     */
    LaneHeight() = default;

    /**
     * @brief Parses a LaneHeight using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneHeight> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneHeight becomes active.
     */
    double sOffset() const { return sOffset_; }

    /**
     * @returns The inner offset from road level.
     */
    double inner() const { return inner_; }

    /**
     * @returns The outer offset from road level.
     */
    double outer() const { return outer_; }

  private:
    class AttribParsers;

    double sOffset_;
    double inner_;
    double outer_;
};

/**
 * @brief A rule for (a cross-section of) a lane.
 */
class LaneRule
{
  public:
    /**
     * @brief Creates an uninitialized LaneRule.
     */
    LaneRule() = default;

    /**
     * @brief Parses a LaneRule using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneMaterial.
     */
    static XodrParseResult<LaneRule> parseXml(XodrReader& xml);

    /**
     * @returns The s-offset, relative to the beginning of the enclosing lane
     * section, of the position where this LaneRule becomes active.
     */
    double sOffset() const { return sOffset_; }

    /**
     * @returns The human readable rule text.
     */
    const std::string& value() const { return value_; }

  private:
    class AttribParsers;

    double sOffset_;
    std::string value_;
};

}}  // namespace aid::xodr