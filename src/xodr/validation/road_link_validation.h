#pragma once

#include <string>
#include <memory>

#include "validation/link_validation_base.h"
#include "xodr_map_keys.h"

namespace aid { namespace xodr {

class XodrMap;
class LaneSection;

/**
 * @brief Validates the links (road links and lane links) in the given XodrMap.
 *
 * @param map           The XodrMap whose links to validate.
 * @param errors        The vector to which errors will be appended if
 *                      validation fails.
 * @returns             True if validation succeeded, false if there was at
 *                      least a single error.
 */
bool validateLinks(const XodrMap& map, std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the links (both road and lane links) between the 'from'
 * contact point and the 'to' contact point.
 *
 * This function should be used when the link from the 'from' contact point to
 * the 'to' contact point is of type RoadLink::ElementType::ROAD.
 *
 * If the link is of type RoadLink::ElementType::JUNCTION, use
 * validateIncomingConnectingLink() instead.
 *
 * The road link validation functions are directed, so to fully validate a pair
 * of road contact points A and B, two calls to one of the road validation
 * functions need to be made: One for links from A to B, and one for links from
 * B to A.
 *
 * @param map                   The XodrMap which contains the data to validate.
 * @param fromContactPointKey   The key of the 'from' contact point.
 * @param toContactPointKey     The key of the 'to' contact point.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateRoadRoadLink(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                          RoadContactPointKey toContactPointKey,
                          std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Validates the links (both road and lane links) between the 'from'
 * contact point and the 'to' contact point, for the case when 'from' is
 * connected to 'to' through a junction.
 *
 * See validateRoadRoadLink() for more details.
 *
 * @param map                   The XodrMap which contains the data to validate.
 * @param fromContactPointKey   The key of the 'from' contact point.
 * @param toContactPointKey     The key of the 'to' contact point.
 * @param connectionKey         The key of the junction connection which
 *                              connects 'from' to 'to'.
 * @param errors                The vector to which errors will be appended if
 *                              validation fails.
 * @returns                     True if validation succeeded, false if there was
 *                              at least a single error.
 */
bool validateIncomingConnectingLink(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                    RoadContactPointKey toContactPointKey, JunctionConnectionKey connectionKey,
                                    std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief The base class for link validation errors which relate to road links
 * (as opposed to lane links).
 *
 * Road link validation errors always occur in the context of a road contact
 * point A which links to a different road contact point B.
 */
class RoadLinkValidationError : public LinkValidationError
{
  public:
    /**
     * @brief The key of contact point A.
     */
    RoadContactPointKey aContactPointKey_;

    /**
     * @brief The key of contact point B.
     */
    RoadContactPointKey bContactPointKey_;
};

/**
 * @brief An error indicating that road link symmetry is violated because of a
 * missing back-link.
 *
 * For any 2 roads contact points A and B, if contact point A links to contact
 * point B, then contact point B must link back to contact point A. This error
 * is generated when a link from contact point A to a contact point B exists,
 * but a link from contact point B, which should link back to contact point A,
 * isn't specified.
 */
class RoadBackLinkNotSpecifiedError : public RoadLinkValidationError
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
     * @brief If specified (ie, not -1), the index of the junction which
     * contains the connection from contact point A to contact point B. If not
     * specified (ie, -1), then contact point A directly links to contact point B.
     */
    int aToBJunctionIdx_;
};

/**
 * @brief An error indicating that road link symmetry is violated because of a
 * missing back-link in a junction.
 *
 * For any 2 road contact points A and B, if contact point A links to contact
 * point B, then contact point B must link back to contact point A. This error
 * is generated when a link from contact point A to contact point B exists, but
 * contact point B links to a junction which doesn't contain a connection back
 * to contact point A.
 */
class RoadBackLinkNotSpecifiedInJunctionError : public RoadLinkValidationError
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
     * @brief If specified (ie, not -1), the index of the junction which
     * contains the connection from contact point A to contact point B. If not
     * specified (ie, -1), then contact point A directly links to contact point B.
     */
    int aToBJunctionIdx_;

    /**
     * @brief The junction connected to contact point b, which was expected to
     * contain a link back to contact point a, but actually doesn't.
     */
    int backLinkJunctionIdx_;
};

/**
 * @brief An error indicating that road link symmetry is violated because of
 * mismatching road links.
 *
 * For any 2 road contact points A and B, if contact point A links to contact
 * point B, then contact point B must link back to contact point A. This error
 * is generated when a link from contact point A to a contact point B exists,
 * but the link in B which should link back to A instead links to a different
 * road contact point C.
 */
class RoadLinkMisMatchError : public RoadLinkValidationError
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
     * @brief The key of contact point C.
     *
     * This is the contact point to which the link from contact point B, which
     * was expected to link back to contact point A, actually links.
     */
    RoadContactPointKey cContactPointKey_;

    /**
     * @brief If specified (ie, not -1), the index of the junction which
     * contains the connection from contact point A to contact point B. If not
     * specified (ie, -1), then contact point A directly links to contact point B.
     */
    int aToBJunctionIdx_ = -1;
};

/**
 * @brief An error indicating that a direct link to a road in a junction was found.
 *
 * For any 2 road contact points A and B, if B is part of a junction, then the
 * connection from A to B should by specified as a connection in the junction.
 * This error is generated when B is part of a junction, yet contact point A
 * tries to connect to it directly.
 */
class DirectLinkToJunctionRoadError : public RoadLinkValidationError
{
  public:
    /**
     * @brief Provides a human readable description of this error
     *
     * @param map           The XodrMap to which this error applies.
     * @return              The error message.
     */
    virtual std::string description(const XodrMap& map) const override;
};

/**
 * @brief An error indicating that adjacent junction paths (both in different
 * junctions) have inconsistent directions.
 */
class InconsistentJunctionPathDirectionsError : public RoadLinkValidationError
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
     * @brief The junction which contains the connection from contact point A to
     * contact point B.
     */
    int aToBJunctionIdx_;

    /**
     * @brief The junction which contains the connection from contact point B to
     * contact point A.
     */
    int bToAJunctionIdx_;
};

}}  // namespace aid::xodr
