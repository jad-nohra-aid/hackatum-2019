#pragma once

#include <boost/variant.hpp>

#include "xodr_reader.h"

namespace aid { namespace xodr {

/**
 * @brief A RoadObjectOutline is an object which describes custom geometry for
 * a RoadObject.
 *
 * The geometry is specified in terms of a polygonal outline, which is extruded
 * in the up direction.
 */
class RoadObjectOutline
{
  public:
    class CornerRoad;
    class CornerLocal;

    /**
     * @brief A corner of this RoadObjectOutline.
     *
     * Each corner can be either a @ref CornerRoad or a @ref CornerLocal, where
     * the difference between the two corner types is the coordinate system
     * used. See the two types for more details.
     */
    typedef boost::variant<CornerRoad, CornerLocal> Corner;

    /**
     * @brief A corner whose coordinates are specified in the s/t coordinate
     * system of the parent Road.
     *
     * This is one of the two alternatives of the @ref Corner variant.
     */
    class CornerRoad
    {
      public:
        /**
         * @brief Parses a CornerRoad using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting CornerRoad.
         */
        static XodrParseResult<CornerRoad> parseXml(XodrReader& xml);

        /**
         * @return The s-coordinate of this corner.
         */
        double s() const { return s_; }

        /**
         * @return The t-coordinate of this corner.
         */
        double t() const { return t_; }

        /**
         * @return The z-offset of this corner, relative to the road's reference line.
         */
        double dz() const { return dz_; }

        /**
         * @return The height of the object at this corner.
         */
        double height() const { return height_; }

      private:
        class AttribParsers;

        double s_;
        double t_;
        double dz_;
        double height_;
    };

    /**
     * @brief A corner whose coordinates are specified in the local coordinate
     * system of the parent RoadObject.
     *
     * This is one of the two alternatives of the @ref Corner variant.
     */
    class CornerLocal
    {
      public:
        /**
         * @brief Parses a CornerLocal using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting CornerLocal.
         */
        static XodrParseResult<CornerLocal> parseXml(XodrReader& xml);

        /**
         * @return The u-coordinate of this corner.
         */
        double u() const { return u_; }

        /**
         * @return The v-coordinate of this corner.
         */
        double v() const { return v_; }

        /**
         * @return The z-coordinate of this corner.
         */
        double z() const { return z_; }

        /**
         * @return The height of the object at this corner.
         */
        double height() const { return height_; }

      private:
        class AttribParsers;

        double u_;
        double v_;
        double z_;
        double height_;
    };

    /**
     * @brief Parses a RoadObjectOutline using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting RoadObjectOutline.
     */
    static XodrParseResult<RoadObjectOutline> parseXml(XodrReader& xml);

    /**
     * @return The corners of this RoadObjectOutline.
     */
    const std::vector<Corner>& corners() const { return corners_; }

  private:
    class ChildElemParsers;

    std::vector<Corner> corners_;
};

}}  // namespace aid::xodr