#pragma once

#include <boost/optional.hpp>

#include "xodr_reader.h"
#include "reference_line.h"
#include "elevation.h"
#include "lane_section.h"
#include "road_link.h"
#include "road_object.h"

namespace aid { namespace xodr {

/**
 * @brief A road in an xodr map.
 *
 * A road consists of one or more lanes and can at most have a single
 * predecessor and a single successor. Places where the road network branches
 * into multiple other roads are described using @ref Junction's instead.
 */
class Road
{
  public:
    /**
     * @brief Creates an uninitialized road.
     */
    Road() = default;

    /**
     * @brief Creates a copy of the given source road.
     *
     * @param src       The source road.
     */
    Road(const Road& src) = delete;

    /**
     * @brief Moves the given source road into this road.
     *
     * @param src       The source road.
     */
    Road(Road&& src) = default;

    /**
     * @brief Parses a Road using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting Road.
     */
    static XodrParseResult<Road> parseXml(XodrReader& xml);

    /**
     * @returns The name of this road.
     */
    const std::string& name() const { return name_; }

    /**
     * @returns The id of this road.
     */
    const std::string& id() const { return id_; }

    /**
     * @returns The id of the junction this road is a part of, or -1 if it isn't
     * part of a junction
     */
    const XodrObjectReference& junctionRef() const { return junctionRef_; }

    /**
     * @returns The length of this road.
     */
    double length() const { return length_; }

    /**
     * @returns The reference line of this road.
     */
    const ReferenceLine& referenceLine() const { return referenceLine_; }

    /**
     * @returns Whether this road has an elevation profile.
     */
    bool hasElevationProfile() const { return static_cast<bool>(elevationProfile_); }

    /**
     * @brief Gets the elevation profile of this road.
     *
     * This function should only be called if hasElevationProfile() is true.
     *
     * @return The elevation profile.
     */
    const ElevationProfile& elevationProfile() const;

    /**
     * @returns The lane sections of this road.
     */
    const std::vector<LaneSection>& laneSections() const { return laneSections_; }

    /**
     * @returns The road objects associated with this road.
     */
    const std::vector<RoadObject>& roadObjects() const { return roadObjects_; }

    /**
     * @returns The RoadLink object describing the predecessor of this road.
     */
    const RoadLink& predecessor() const { return links_.predecessor(); }

    /**
     * @returns The RoadLink object describing the successor of this road.
     */
    const RoadLink& successor() const { return links_.successor(); }

    /**
     * @brief A function which returns either the precessor or successor
     * RoadLink of this object, depending on the value of the @p roadLinkType
     * parameter.
     *
     * This function is useful when writing generic code which can be used with
     * both predecessor and successor links.
     *
     * @param roadLinkType  The RoadLinkType indicating which RoadLink to return.
     * @returns             The RoadLink.
     */
    const RoadLink& roadLink(RoadLinkType roadLinkType) const;

    /**
     * @brief Resolves the XodrObjectReference references in this road.
     *
     * See XodrObjectReference::resolve() for more details.
     *
     * @param idToIndexMaps   The mappings from identifiers to indices.
     */
    void resolveReferences(const IdToIndexMaps& idToIndexMaps);

    /**
     * @brief Gets the beginning of the range of global lane indices used by the
     * lanes in this road.
     *
     * The global lane indices (see LaneSection::Lane::globalIndex()) of all
     * lanes belonging to the same road are guaranteed to form a consecutive
     * range.
     *
     * This range consists of the global lane indices satisfying:
     *
     *   road.globalLaneIndicesBegin() <= globalLaneIndex < road.globalLaneIndicesEnd().
     *
     */
    int globalLaneIndicesBegin() const;

    /**
     * @brief Gets the end of the range of global lane indices used by the lanes
     * in this road.
     *
     * The global lane indices (see LaneSection::Lane::globalIndex()) of all
     * lanes belonging to the same road are guaranteed to form a consecutive
     * range.
     *
     * This range consists of the global lane indices satisfying:
     *
     *   road.globalLaneIndicesBegin() <= globalLaneIndex < road.globalLaneIndicesEnd().
     *
     */
    int globalLaneIndicesEnd() const;

    /**
     * @brief Gets the index of the lane section which contains the road's
     * contact point of the given type.
     *
     * @param contactPoint  The contact point type.
     * @return              The index of the lane section.
     */
    int laneSectionIndexForContactPoint(ContactPoint contactPoint) const;

    /**
     * @brief Gets the lane section which contains the road's contact point of
     * the given type.
     *
     * If contactPoint is START, then this is the first lane section, if contact
     * point is END, then this is the last lane section.
     *
     * @param contactPoint  The contact point type.
     * @return              The lane section.
     */
    LaneSection& laneSectionForContactPoint(ContactPoint contactPoint);

    /**
     * @brief Gets the lane section which contains the road's contact point of
     * the given type.
     *
     * If contactPoint is START, then this is the first lane section, if contact
     * point is END, then this is the last lane section.
     *
     * @param contactPoint  The contact point type.
     * @return              The lane section.
     */
    const LaneSection& laneSectionForContactPoint(ContactPoint contactPoint) const;

    /**
     * @brief Gets the index of the lane section with the lanes whose links of
     * the given link type are external.
     *
     * If linkType is PREDECESSOR, then this is the first lane section, if
     * linkType is SUCCESSOR, then this is the last lane section.
     *
     * @param linkType      The link type.
     * @return              The index of the lane section.
     */
    int laneSectionIndexForExternalLinkType(RoadLinkType linkType) const;

    /**
     * @brief Gets the lane section with the lanes whose links of the given link
     * type are external.
     *
     * If linkType is PREDECESSOR, then this is the first lane section, if
     * linkType is SUCCESSOR, then this is the last lane section.
     */
    LaneSection& laneSectionForExternalLinkType(RoadLinkType linkType);

    /**
     * @brief Gets the lane section with the lanes whose links of the given link
     * type are external.
     *
     * If linkType is PREDECESSOR, then this is the first lane section, if
     * linkType is SUCCESSOR, then this is the last lane section.
     */
    const LaneSection& laneSectionForExternalLinkType(RoadLinkType linkType) const
    {
        return const_cast<Road*>(this)->laneSectionForExternalLinkType(linkType);
    }

    /**
     * @brief Validates this Road.
     *
     * An exception is thrown if validation doesn't pass.
     */
    void validate() const;

  public:
    /**
     * @brief Sets the predecessor of this Road.
     *
     * This function should only be used from unit tests.
     *
     * @param predecessor   The predecessor RoadLink.
     */
    void test_setPredecessor(const RoadLink& predecessor) { links_.test_setPredecessor(predecessor); }

    /**
     * @brief Sets the successor of this Road.
     *
     * This function should only be used from unit tests.
     *
     * @param successor     The successor RoadLink.
     */
    void test_setSuccessor(const RoadLink& successor) { links_.test_setSuccessor(successor); }

  public:
    // Test interface

    /**
     * @brief Gives write access to the lane with the given index.
     *
     * This function should only be used from unit tests.
     */
    LaneSection& test_laneSection(int i) { return laneSections_[i]; }

  private:
    class AttribParsers;
    class ChildElemParsers;
    class LaneChildElemParsers;
    class ObjectsChildElemParsers;

    std::string name_;
    std::string id_;
    XodrObjectReference junctionRef_;

    double length_;
    ReferenceLine referenceLine_;
    boost::optional<ElevationProfile> elevationProfile_;
    std::vector<LaneSection> laneSections_;
    std::vector<RoadObject> roadObjects_;

    RoadLinks links_;
};

}}  // namespace aid::xodr
