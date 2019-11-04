#pragma once

#include "poly3.h"

#include "xml/xml_parse_result.h"
#include "xodr_reader.h"

#include <vector>

namespace aid { namespace xodr {

/**
 * @brief The elevation profile of a road.
 *
 * The elevation profile defines the roads elevation along its reference line.
 */
class ElevationProfile
{
  public:
    /**
     * @brief Constructs an uninitialized ElevationProfile.
     */
    ElevationProfile() = default;

    /**
     * @brief Parsers an ElevationProfile using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting ElevationProfile.
     */
    static XodrParseResult<ElevationProfile> parseXml(XodrReader& xml);

    /**
     * @brief A segment of an elevation profile whose elevation is described
     * using a single cubic polynomials.
     */
    class Elevation
    {
      public:
        /**
         * @brief Constructs an uninitialized Elevation.
         */
        Elevation() = default;

        /**
         * @brief Constructs an Elevation from the given values;
         */
        Elevation(double sCoord, const Poly3& poly3);

        /**
         * @brief Parsers an Elevation using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Elevation.
         */
        static XodrParseResult<Elevation> parseXml(XodrReader& xml);

        /**
         * @brief Returns the s-coordinate of beginning of this elevation
         * segment.
         *
         * The end s-coordinate is either the s-coordinate of the next elevation
         * segment, or s-coordinate of the end of the road's reference line,
         * if this is the last segment.
         *
         * @return The s-coordinate.
         */
        double sCoord() const { return sCoord_; }

        /**
         * @brief Returns the cubic polynomial describing the elevation profile
         * of this segment.
         *
         * This input to this polynomial function is the s-coordinate of the
         * position on the reference line, relative to the start s-coordinate
         * of this segment (see sCoord()).
         *
         * @return The polynomial.
         */
        const Poly3& poly3() const { return poly3_; }

      private:
        class AttribParsers;

        void setA(double a) { poly3_.a_ = a; }
        void setB(double b) { poly3_.b_ = b; }
        void setC(double c) { poly3_.c_ = c; }
        void setD(double d) { poly3_.d_ = d; }

        double sCoord_;
        Poly3 poly3_;
    };

    /**
     * @brief Returns the elevation segments of this elevation profile.
     *
     * The elevation segments are ordered by increasing Elevation::sCoord().
     *
     * @return The elevation segments.
     */
    const std::vector<Elevation>& elevations() const { return elevations_; }

  private:
    class ChildElemParsers;

    std::vector<Elevation> elevations_;
};

}}  // namespace aid::xodr