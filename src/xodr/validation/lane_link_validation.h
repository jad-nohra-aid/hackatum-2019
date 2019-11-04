#pragma once

#include <memory>

#include "validation/link_validation_base.h"
#include "xodr_map_keys.h"
#include "junction.h"

namespace aid { namespace xodr {

class XodrMap;
class LaneSection;

/**
 * @brief Validates the lane links from the 'from' lane section to the 'to'
 * section, for two lane sections which are connected at the given contact points.
 *
 * @param fromContactPointKey   The contact point on the 'from' lane section.
 * @param toContactPointKey     the contact point on the 'to' lane section.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateRoadRoadLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                               RoadContactPointKey toContactPointKey,
                               std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the lane links from the 'from' lane section to the 'to'
 * section, for two lane sections which are connected at the given contact
 * points, when 'from' is part of a connecting road and 'to' part of an incoming road.
 *
 * @param fromContactPointKey   The contact point on the 'from' lane section.
 * @param toContactPointKey     the contact point on the 'to' lane section.
 * @param backLinkConnection    The junction connection which contains the links
 *                              from 'to' back to 'from'.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateConnectingIncomingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey,
                                         const Junction::Connection& backLinkConnection,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the lane links from the 'from' lane section to the 'to'
 * section, for two lane sections which are connected at the given contact
 * points, when 'from' is part of a connecting road and 'to' part of an outgoing road.
 *
 * @param fromContactPointKey   The contact point on the 'from' lane section.
 * @param toContactPointKey     the contact point on the 'to' lane section.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateConnectingOutgoingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the lane links from the 'from' lane section to the 'to'
 * section, for two lane sections which are connected at the given contact
 * points, when 'from' is part of an incoming road and 'to' part of a connecting road.
 *
 * @param fromContactPointKey   The contact point on the 'from' lane section.
 * @param toContactPointKey     the contact point on the 'to' lane section.
 * @param backLinkConnection    The junction connection which contains the links
 *                              from 'to' back to 'from'.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateIncomingConnectingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey, JunctionConnectionKey connectionKey,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the lane links from the 'from' lane section to the 'to'
 * section, for two lane sections which are connected at the given contact
 * points, when both lane sections are part of roads which are connecting roads
 * (both in different junctions).
 *
 * @param fromContactPointKey   The contact point on the 'from' lane section.
 * @param toContactPointKey     the contact point on the 'to' lane section.
 * @param backLinkConnection    The junction connection which contains the links
 *                              from 'to' back to 'from'.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateConnectingConnectingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                           RoadContactPointKey toContactPointKey,
                                           const Junction::Connection& linkConnection,
                                           std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the internal lane links in the given road.
 *
 * Lane links are internal when they're between lanes in lane sections which
 * are both part of the same road.
 */
bool validateRoadInternalLaneLinks(const XodrMap& map, int roadIdx,
                                   std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the lane links from 'fromSection' to 'toSection'.
 *
 * Specifically, it validates whether:
 *  - Lane links in 'fromSection' refer to valid lanes in 'toSection'.
 *  - Lane links are symmetric: If a lane in 'fromSection' links to a certain
 *    lane 'toSection' then that lane in 'toSection' must link back to the
 *    original lane in 'fromSection' as well.
 *  - Whether no (non-center) lanes are linked to the center lane.
 *
 * An exception is thrown if validation doesn't pass.
 *
 * Note that to fully validate the links between two lane sections, this
 * function must be called in both directions, that is:
 *
 *   validateLaneLinks(a, b, aToBLinkType, bToALinkType);
 *   validateLaneLinks(b, a, bToALinkType, aToBLinkType);
 *
 * @param fromSection           The LaneSection containing the lanes from which
 *                              the link to validate originate.
 * @param toSection             The LaneSection containing the target lanes of
 *                              the links to validate.
 * @param fromContactPointKey   The contact point on fromSection which connects
 *                              it to toSection.
 * @param toContactPointKey     The contact point on toSection which connects it
 *                              to fromSection.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateLaneLinks(const LaneSection& fromSection, const LaneSection& toSection,
                       const LaneSectionContactPointKey& fromContactPointKey,
                       const LaneSectionContactPointKey& toContactPointKey,
                       std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief The base class for link validation errors which relate to lane links
 * (as opposed to road links).
 *
 * Lane link validation is always done between two adjacent lane sections
 * (called A and B), and these sections are connected at the contact points
 * referenced by aSectionContactPointKey_ and  bSectionContactPointKey_.
 */
class LaneLinkValidationError : public LinkValidationError
{
  public:
    /**
     * @brief The contact point on the A lane section.
     *
     * This is the contact point which links to bSectionContactPointKey_.
     */
    LaneSectionContactPointKey aSectionContactPointKey_;

    /**
     * @brief The contact point on the B lane section.
     *
     * This is the contact point which links to aSectionContactPointKey_.
     */
    LaneSectionContactPointKey bSectionContactPointKey_;
};

/**
 * @brief An error indicating that lane link symmetry is violated because of a
 * missing back-link.
 *
 * For any 2 lanes, if lane A links to lane B, then lane B must link back to
 * lane A. This error is generated when a link from lane A to a lane B exists,
 * but a link from lane B, which should link back to lane A, isn't specified.
 */
class LaneBackLinkNotSpecified : public LaneLinkValidationError
{
  public:
    virtual std::string description(const XodrMap& map) const override;

    /**
     * @brief The id of lane A.
     */
    LaneID aLaneId_;

    /**
     * @brief The id of lane B.
     */
    LaneID bLaneId_;
};

/**
 * @brief An error indicating that lane link symmetry is violated because of
 * mismatching lane links.
 *
 * For any 2 lanes, if lane A links to lane B, then lane B must link back to
 * lane A. This error is generated when a link from lane A to a lane B exists,
 * but lane A links back to a different lane C instead (which must still be in
 * lane section A).
 */
class LaneLinkMisMatch : public LaneLinkValidationError
{
  public:
    /**
     * @brief Provides a human readable description of this error
     *
     * @param map           The XodrMap to which this error applies.
     * @return              The error message.
     */
    virtual std::string description(const XodrMap& map) const override;

    /**
     * @brief The id of lane A.
     */
    LaneID aLaneId_;

    /**
     * @brief The id of lane B.
     */
    LaneID bLaneId_;

    /**
     * @brief The id of lane C.
     */
    LaneID cLaneId_;
};

/**
 * @brief An error indicating that a non-center-lane in lane section A links
 * to the center lane of lane section B.
 */
class LaneLinkToCenterLaneError : public LaneLinkValidationError
{
  public:
    /**
     * @brief Provides a human readable description of this error
     *
     * @param map           The XodrMap to which this error applies.
     * @return              The error message.
     */
    virtual std::string description(const XodrMap& map) const override;

    /**
     * @brief The id of the lane in section A which links to the center-lane
     * of section B.
     */
    LaneID fromLaneId_;
};

/**
 * @brief An error indicating that a lane in lane section A tries to a link to
 * a lane which falls outside the range of lanes in section B.
 */
class LaneLinkTargetOutOfRange : public LaneLinkValidationError
{
  public:
    /**
     * @brief Provides a human readable description of this error
     *
     * @param map           The XodrMap to which this error applies.
     * @return              The error message.
     */
    virtual std::string description(const XodrMap& map) const override;

    /**
     * @brief The ID of the lane in section A, which tries to link to the
     * non-existing lane.
     */
    LaneID aLaneId_;

    /**
     * @brief The (out of range) ID of the lane in lane section B to which lane
     * A is trying to link.
     */
    LaneID bLaneId_;
};

/**
 * @brief An error indicating that two lanes of opposing driving directions are linked.
 */
class LaneLinkOpposingDirections : public LaneLinkValidationError
{
  public:
    /**
     * @brief Provides a human readable description of this error
     *
     * @param map           The XodrMap to which this error applies.
     * @return              The error message.
     */
    virtual std::string description(const XodrMap& map) const override;

    /**
     * @brief The id of lane A.
     */
    LaneID aLaneId_;

    /**
     * @brief The id of lane B.
     */
    LaneID bLaneId_;
};

}}  // namespace aid::xodr
