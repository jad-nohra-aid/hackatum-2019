#pragma once

#include <Eigen/Dense>

#include "xodr_reader.h"
#include "poly3.h"
#include "reference_line.h"
#include "road_link.h"
#include "lane_id.h"
#include "lane_attributes.h"

namespace aid { namespace xodr {

class Road;

enum class LaneType : int;

/**
 * @brief A lane section describes the lanes in a given cross-section of a road.
 */
class LaneSection
{
  public:
    friend class Road;

    /**
     * Creates an empty lane section.
     */
    LaneSection() = default;

    /**
     * @brief Parses a LaneSection using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting LaneSection.
     */
    static XodrParseResult<LaneSection> parseXml(XodrReader& xml);

    class WidthPoly3;

    /**
     * @brief A lane in a lane section.
     */
    class Lane
    {
        friend class LaneSection;

      public:
        /**
         * @brief Creates an empty lane.
         */
        Lane();

        /**
         * @brief Parses a Lane using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Lane.
         */
        static XodrParseResult<Lane> parseXml(XodrReader& xml);

        /**
         * @brief Gets the id of this lane.
         */
        LaneID id() const { return id_; }

        /**
         * @brief Gets the type of this lane.
         */
        LaneType type() const { return type_; }

        /**
         * @brief Gets whether this lane should be kept level.
         *
         * If this is false, then superelevation and crossfall (if present) are
         * applied, if true, then these are ignored for this lane.
         */
        bool level() const { return level_; }

        /**
         * @brief Gets whether this lane has a predecessor.
         *
         * @returns         True if it has a predecessor, false otherwise.
         */
        bool hasPredecessor() const;

        /**
         * @brief Gets the predecessor of this lane.
         *
         * This function shouldn't be called if hasPredecessor() is false.
         *
         * @returns         The lane id of the predecessor lane.
         */
        LaneID predecessor() const;

        /**
         * @brief Gets whether this lane has a successor.
         *
         * @returns         True if it has a successor, false otherwise.
         */
        bool hasSuccessor() const;

        /**
         * @brief Gets the successor of this lane.
         *
         * This function shouldn't be called if hasSuccessor() is false.
         *
         * @returns         The lane id of the successor lane.
         */
        LaneID successor() const;

        /**
         * @brief Gets whether this lane has a link (predecessor/successor) of
         * the given type.
         *
         * @returns         True if this lane has a link.
         */
        bool hasLink(RoadLinkType roadLinkType) const;

        /**
         * @brief Gets the link (predecessor/successor) of the given type.
         *
         * This function shouldn't be called if hasLink(roadLinkType) returns false.
         *
         * @returns         The lane id of the linked lane.
         */
        LaneID link(RoadLinkType roadLinkType) const;

        /**
         * @brief Gets the width polynomials of this lane.
         *
         * The width polynomials together form a piecewise cubic curve which
         * specifies the shape of the far boundary.
         *
         * Specifically, given an s-coordinate which falls in the range of a
         * WidthPoly3 p, and a point on the near boundary with st-coordinates
         * (s, t), then the point (s, t + p.eval(s - p.sOffset)) is on the far boundary.
         *
         * @returns         A vector with this lane's width polynomials.
         */
        const std::vector<WidthPoly3>& widthPoly3s() const { return widthPoly3s_; }

        /**
         * @name Lane attributes
         *
         * Various types of lane attributes can be associated with a
         * cross-sections of each lane.
         *
         * The s-offset of a lane attributes specifies the offset in
         * s-coordinates from the beginning of the lane section to the position
         * where the attribute becomes active. The attribute remains active
         * until the beginning of the next lane attribute of the same type.
         *
         * @{
         */

        /**
         * @returns The LaneMaterial attributes associated with this lane.
         */
        const std::vector<LaneMaterial>& materials() const { return materials_; }

        /**
         * @returns The LaneVisibility attributes associated with this lane.
         */
        const std::vector<LaneVisibility>& visibilities() const { return visibilities_; }

        /**
         * @returns The LaneSpeedLimit attributes associated with this lane.
         */
        const std::vector<LaneSpeedLimit>& speedLimits() const { return speedLimits_; };

        /**
         * @returns The LaneAccess attributes associated with this lane.
         */
        const std::vector<LaneAccess>& accesses() const { return accesses_; };

        /**
         * @returns The LaneHeight attributes associated with this lane.
         */
        const std::vector<LaneHeight>& heights() const { return heights_; }

        /**
         * @returns The LaneRule attributes associated with this lane.
         */
        const std::vector<LaneRule>& rules() const { return rules_; }

        /** @} */

        /**
         * @brief Gets the global index (within the XodrMap) of this lane.
         *
         * The global index is useful when there's additional data which should
         * associated with each lane in an XodrMap. In this case, you'd allocate
         * an array with XodrMap::totalNumLanes() elements, and use the global
         * index to get the specific element associated with each lane.
         *
         * @returns             The global index.
         */
        int globalIndex() const { return globalIndex_; }

        /**
         * @brief Validates this Lane.
         *
         * An exception is thrown if validation doesn't pass.
         *
         * @param maxSCoord     The max s-coordinate of the s-range of this lane
         *                      (this is the same for all lanes in a lane
         *                      section, so it's only stored once in the lane
         *                      section, not in the individual lanes).
         */
        void validate(double maxSCoord) const;

        /**
         * @brief  Finds the width of the lane at the given s-coordinate
         *
         * @param s         The s-coordinate
         * @returns         The width of the lane.
         *
         */
        double widthAtSCoord(const double s) const;

      public:
        /**
         * @brief Sets the predecessor of this lane.
         *
         * This function should only be used from unit tests.
         *
         * @param predecessor   The predecessor LaneID, or LaneID::null() if
         *                      there is no predecessor.
         */
        void test_setPredecessor(LaneIDOpt predecessor) { predecessor_ = predecessor; }

        /**
         * @brief Sets the successor of this lane.
         *
         * This function should only be used from unit tests.
         *
         * @param successor     The successor LaneID, or LaneID::null() if there
         *                      is no successor.
         */
        void test_setSuccessor(LaneIDOpt successor) { successor_ = successor; }

      private:
        class AttribParsers;
        class ChildElemParsers;
        class LinkChildElemParsers;

        LaneID id_;
        LaneType type_;
        bool level_;

        std::vector<WidthPoly3> widthPoly3s_;

        std::vector<LaneMaterial> materials_;
        std::vector<LaneVisibility> visibilities_;
        std::vector<LaneSpeedLimit> speedLimits_;
        std::vector<LaneAccess> accesses_;
        std::vector<LaneHeight> heights_;
        std::vector<LaneRule> rules_;

        LaneIDOpt predecessor_;
        LaneIDOpt successor_;

        int globalIndex_;
    };

    /**
     * @brief A polygon in the piecewise cubic curve describing the lane width.
     */
    class WidthPoly3
    {
      public:
        /**
         * @brief Creates an uninitialized WidthPoly3.
         */
        WidthPoly3() = default;

        /**
         * @brief Creates a WidthPoly3 from a given sOffset and polynomial.
         *
         * @param sOffset   The sOffset from the s-coordinate of the beginning of the lane section to the beginning of
         * the s-range of this polygon.
         * @param poly3     The polynomial.
         */
        WidthPoly3(double sOffset, Poly3 poly3) : sOffset_(sOffset), poly3_(poly3) {}

        /**
         * @brief Parses a WidthPoly3 using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting WidthPoly3.
         */
        static XodrParseResult<WidthPoly3> parseXml(XodrReader& xml);

        /**
         * @brief Gets the sOffset from the s-coordinate of the beginning of the
         * lane section to beginning of the s-range of this polygon.
         *
         * @returns         The s offset.
         */
        double sOffset() const { return sOffset_; }

        /**
         * @brief Gets the polynomial.
         */
        const Poly3& poly3() const { return poly3_; }

      private:
        class AttribParsers;

        void setA(double a) { poly3_.a_ = a; }
        void setB(double b) { poly3_.b_ = b; }
        void setC(double c) { poly3_.c_ = c; }
        void setD(double d) { poly3_.d_ = d; }

        double sOffset_;
        Poly3 poly3_;
    };

    /**
     * @brief A tessellation of a boundary, represented as a list of lateral
     * positions (t-coordinates).
     */
    struct BoundaryTessellation
    {
        /**
         * @brief The lateral positions of the points on the boundary.
         *
         * This array is a parallel array to the reference line tesselation
         * passed to the @ref tessellateLaneBoundaries function. To go from a
         * lateral position to a full 2D point, the information of the
         * corresponding entry in the reference line is also needed.
         */
        std::vector<double> lateralPositions_;
    };

    /**
     * @brief A tessellation of a boundary, represented in cartesian coordinates.
     */
    struct BoundaryCurveTessellation
    {
        /**
         * @brief The positions of the points on the boundary.
         */
        std::vector<Eigen::Vector2d> vertices_;
    };

    /**
     * @brief A tessellation of a lane, represented in terms of a polyline
     * describing the lane's center line as well as the variances, which
     * describe the distance to the lane's boundaries in the direction parallel
     * to the center line.
     */
    struct CenterLineTessellation
    {
        /**
         * @brief The vertices of the center line.
         */
        std::vector<Eigen::Vector2d> vertices_;

        /**
         * @brief The variances of each vertex of the center line.
         */
        std::vector<double> variances_;
    };

    /**
     * @brief Tessellates the lane boundaries into polylines with vertices
     * specified in terms of their lateral position (t-coordinates).
     *
     * The first boundary in the resulting vector is the left boundary of the
     * left-most lane, the subsequent boundaries are those between adjacent
     * lanes, going from left to right until the last boundary which is the
     * right boundary of the right-most lane.
     *
     * The s-coordinates of the locations where boundary vertices are created
     * are taken from the @p refLineTessellation.
     *
     * @param refLineTessellation   The tessellation of the reference line.
     * @returns             A vector of BoundaryTessellation's.
     */
    std::vector<BoundaryTessellation> tessellateLaneBoundaries(
        const ReferenceLine::Tessellation& refLineTessellation) const;

    /**
     * @brief Tessellates the lane boundaries into polylines with vertices
     * specified in cartesian coordinates.
     *
     * The first boundary in the resulting vector is the left boundary of the
     * left-most lane, the subsequent boundaries are those between adjacent
     * lanes, going from left to right until the last boundary which is the
     * right boundary of the right-most lane.
     *
     * @param refLineTessellation   The tessellation of the reference line.
     * @returns             A vector of BoundaryCurveTessellation's.
     */
    std::vector<BoundaryCurveTessellation> tessellateLaneBoundaryCurves(
        const ReferenceLine::Tessellation& refLineTessellation) const;

    /**
     * @brief Tessellates the lanes into the center line plus variance form.
     *
     * The first boundary in the resulting vector is the left boundary of the
     * left-most lane, the subsequent boundaries are those between adjacent
     * lanes, going from left to right until the last boundary which is the
     * right boundary of the right-most lane.
     *
     * @param refLineTessellation   The tessellation of the reference line. The
     *                      resulting tessellation will have vertices at the
     *                      same s-coordinates as the vertices in this tessellation.
     * @returns             A vector of BoundaryTessellation's.
     */
    std::vector<CenterLineTessellation> tessellateLaneCenterLines(
        const ReferenceLine::Tessellation& refLineTessellation) const;

    /**
     * @brief A struct to hold the return values of the
     * tessellateLaneBoundaryCurvesAndCenterLines() function.
     */
    struct BoundaryCurveAndCenterLineTessellations
    {
        /**
         * The boundary curve tessellations. This part matches the result of the
         * tessellateLaneBoundaryCurves() function. See that function for more details.
         */
        std::vector<BoundaryCurveTessellation> boundaryCurveTessellations_;

        /**
         * The boundary curve tessellations. This part matches the result of the
         * tessellateLaneBoundaryCurves() function. See that function for more details.
         */
        std::vector<CenterLineTessellation> centerLineTessellations_;
    };

    /**
     * @brief Simultaneously computes the boundary curve tessellation and
     * center line tessellation.
     *
     * This function gives the same result as calling the
     * tessellateLaneBoundaryCurves() and tessellateLaneCenterLines() functions
     * with the given input @p refLineTessellation. See these two functions for
     * more details.
     *
     * @param refLineTessellation   The tessellation of the reference line. The
     *                      resulting tessellation will have vertices at the
     *                      same s-coordinates as the vertices in this tessellation.
     * @return              The structure containing the two resulting tessellations.
     */
    BoundaryCurveAndCenterLineTessellations tessellateLaneBoundaryCurvesAndCenterLines(
        const ReferenceLine::Tessellation& refLineTessellation) const;

    /**
     * @brief The beginning of the s-range of this lane section.
     *
     * This s-range is relative to the beginning of the road this lane section
     * is a part of.
     *
     * @returns             The start s coordinate.
     */
    double startS() const { return startS_; }

    /**
     * @brief The end of the s-range of this lane section.
     *
     * This s-range is relative to the beginning of the road this lane section
     * is a part of.
     *
     * @returns             The end s coordinate.
     */
    double endS() const { return endS_; }

    /**
     * @brief Gets whether this lane section is single sided.
     *
     * If a lane section is single sided, then it will only contain lanes on
     * either the left side or right side of the reference line, but not on both sides.
     *
     * @returns             True if the lane section is single sided, false otherwise.
     */
    bool singleSided() const { return singleSided_; }

    /**
     * @brief The number of lanes which are on the left side of the reference line.
     *
     * The first numLeftLanes() lanes of lanes() are the left lanes, the
     * remaining ones the right lanes.
     *
     * @returns             The number of left lanes.
     */
    int numLeftLanes() const { return numLeftLanes_; }

    /**
     * @brief The number of lanes which are on the right side of the reference line.
     *
     * The final numRightLanes() lanes of lanes() are the right lanes, the
     * preceding ones the left lanes.
     *
     * @returns             The number of right lanes.
     */
    int numRightLanes() const { return static_cast<int>(lanes_.size()) - numLeftLanes_; }

    /**
     * @brief Gets the lanes of this lane section.
     *
     * @returns A vector with the lanes.
     */
    const std::vector<Lane>& lanes() const { return lanes_; }

    /**
     * @brief Converts from a lane index to a lane identifier.
     *
     * See laneById() for an explanation of lane identifiers.
     *
     * @param idx           The lane index.
     * @returns             The identifier of the lane with the given index.
     */
    LaneID laneIndexToId(int idx) const;

    /**
     * @brief Converts from a lane identifier to a lane index.
     *
     * See laneById() for an explanation of lane identifiers.
     *
     * @param id            The lane identifier.
     * @returns             The index of the lane with the given identifier.
     */
    int laneIdToIndex(LaneID id) const;

    /**
     * @brief Gets the lane of this lane section with the given lane identifier.
     *
     * Lane ID's follow the following convention:
     * - The lane immediately to the left of the reference line has ID 1,
     * - The subsequent left lanes have increasing and consecutive ID's.
     * - The lane immediately to the right of the reference line has ID -1.
     * - The subsequent right lanes have decreasing and consecutive ID's.
     *
     * @param id            The lane ID. This must be a valid lane ID in this
     *                      lane section.
     * @returns             The lane.
     */
    const Lane& laneById(LaneID id) const { return lanes_[laneIdToIndex(id)]; }

    /**
     * @brief Validates this LaneSection.
     *
     * An exception is thrown if validation doesn't pass.
     */
    void validate() const;

  public:
    /**
     * @brief Gets the lane of this lane section with the given lane identifier.
     *
     * This function is similar to @ref laneById, with the difference that it
     * returns a non-const reference.
     *
     * This function should only be used from unit tests.
     *
     * @param id            The lane identifier.
     * @returns             The index of the lane with the given identifier.
     */
    Lane& test_laneById(LaneID id) { return lanes_[laneIdToIndex(id)]; }

    /**
     * @brief Sets the end s-coordinate of this lane section.
     *
     * This function should only be used from unit tests.
     *
     * @param endS          The end s-coordinate of the lane section.
     */
    void test_setEndS(double endS) { endS_ = endS; }

  private:
    class AttribParsers;
    class ChildElemParsers;

    static void parseLeftLanes(XodrReader& xml, XodrParseResult<LaneSection>& laneSection);
    static void parseRightLanes(XodrReader& xml, XodrParseResult<LaneSection>& laneSection);

    /**
     * @brief Tesselates the lane boundaries on one side of the reference line.
     *
     * This function is part of the implementation of @ref tessellateLaneBoundaries.
     *
     * It loops over the lanes on a given side of the reference line, from the
     * lane adjacent to the reference line, towards the outer lanes. Inside this
     * loop it constructs the far boundary of each lane, using that lane's width
     * curve and its near boundary. The near boundary will always be available,
     * either because it was the far boundary of the loop's previous iteration,
     * or because it was provided by the caller.
     *
     * @param refLineTessellation   The tessellation of the reference line.
     * @param boundaryIt    An iterator, which initially points to the boundary
     *                      which corresponds to the reference line, and will
     *                      be advanced in the direction of 'stepDir' to go to
     *                      the next boundary in the direction of the current side.
     * @param lanesBegin    The first lane on the current side.
     * @param lanesEnd      The end of the range of lanes which should be
     *                      handled by this function. The range does not
     *                      include the lane at index lanesEnd.
     * @param stepDir       The direction into which the lane indices and
     *                      boundaryIt should be advanced. This should be -1 for
     *                      the lanes on the left side of the reference line and
     *                      1 for the lanes on the right side.
     */
    void tessellateLaneBoundariesSide(const ReferenceLine::Tessellation& refLineTessellation,
                                      BoundaryTessellation* boundaryIt, int lanesBegin, int lanesEnd,
                                      int stepDir) const;

    double startS_;
    double endS_;
    bool singleSided_;

    int numLeftLanes_;
    std::vector<Lane> lanes_;
};

enum class LaneType : int
{
    NONE,
    DRIVING,
    STOP,
    SHOULDER,
    BIKING,
    SIDEWALK,
    BORDER,
    RESTRICTED,
    PARKING,
    BIDIRECTIONAL,
    MEDIAN,
    SPECIAL1,
    SPECIAL2,
    SPECIAL3,
    ROADWORKS,
    TRAM,
    RAIL,
    ENTRY,
    EXIT,
    OFF_RAMP,
    ON_RAMP,
    CONNECTING_RAMP,
    BUS,
    TAXI,
    HOV
};

}}  // namespace aid::xodr
