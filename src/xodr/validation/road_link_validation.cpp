#include "validation/road_link_validation.h"

#include <sstream>

#include "validation/lane_link_validation.h"
#include "xodr_map.h"

namespace aid { namespace xodr {

/**
 * @brief Validates the road and lane links from the given road contact point.
 *
 * This function implements a single iteration in the validateLinks() function.
 *
 * @param map           The XodrMap which contains the data to validate.
 * @param roadIdx       The index of the road which contains the links to validate.
 * @param linkType      The type of the links to validate.
 * @returns             True if validation succeeded, false if there was at
 *                      least a single error.
 */
static bool validateLinksIteration(const XodrMap& map, RoadContactPointKey contactPointKey,
                                   std::vector<std::unique_ptr<LinkValidationError>>& errors);

/**
 * @brief Gets the RoadLink for the given road contact point.
 *
 * @param map           The XodrMap which contains the link.
 * @param key           The key of the contact point corresponding to the link.
 * @return              The RoadLink.
 */
static const RoadLink& roadLinkForRoadContactPoint(const XodrMap& map, RoadContactPointKey key);

bool validateLinks(const XodrMap& map, std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    const auto& roads = map.roads();
    for (int i = 0; i < static_cast<int>(roads.size()); i++)
    {
        success &= validateRoadInternalLaneLinks(map, i, errors);
        success &= validateLinksIteration(map, RoadContactPointKey(i, ContactPoint::START), errors);
        success &= validateLinksIteration(map, RoadContactPointKey(i, ContactPoint::END), errors);
    }

    return success;
}

static bool validateLinksIteration(const XodrMap& map, RoadContactPointKey contactPointKey,
                                   std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    const RoadLink& link = roadLinkForRoadContactPoint(map, contactPointKey);

    switch (link.elementType())
    {
        case RoadLink::ElementType::NOT_SPECIFIED:
            return true;

        case RoadLink::ElementType::ROAD:
        {
            RoadContactPointKey toContactPointKey(link.elementRef().index(), link.contactPoint());

            const Road& toRoad = map.roads()[link.elementRef().index()];
            if (toRoad.junctionRef().hasValue())
            {
                DirectLinkToJunctionRoadError* error = new DirectLinkToJunctionRoadError();
                error->aContactPointKey_ = contactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                errors.emplace_back(error);
                return false;
            }
            else
            {
                return validateRoadRoadLink(map, contactPointKey, toContactPointKey, errors);
            }
        }

        case RoadLink::ElementType::JUNCTION:
        {
            bool success = true;

            int junctionIdx = link.elementRef().index();
            const auto& connections = map.junctions()[junctionIdx].connections();
            for (int i = 0; i < static_cast<int>(connections.size()); i++)
            {
                const Junction::Connection& conn = connections[i];

                if (conn.incomingRoad().index() != contactPointKey.roadIdx_)
                {
                    continue;
                }

                int destRoadIdx = conn.connectingRoad().index();
                success &= validateIncomingConnectingLink(map, contactPointKey,
                                                          RoadContactPointKey(destRoadIdx, conn.contactPoint()),
                                                          JunctionConnectionKey(junctionIdx, i), errors);
            }

            return success;
        }

        default:
            assert(!"Invalid link element type.");
            return false;
    }
}

bool validateRoadRoadLink(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                          RoadContactPointKey toContactPointKey,
                          std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    const RoadLink& backLink = roadLinkForRoadContactPoint(map, toContactPointKey);

    switch (backLink.elementType())
    {
        case RoadLink::ElementType::NOT_SPECIFIED:
        {
            RoadBackLinkNotSpecifiedError* error = new RoadBackLinkNotSpecifiedError();
            error->aContactPointKey_ = fromContactPointKey;
            error->bContactPointKey_ = toContactPointKey;
            error->aToBJunctionIdx_ = -1;
            errors.emplace_back(error);
            return false;
        }

        case RoadLink::ElementType::ROAD:
            if (backLink.elementRef().index() != fromContactPointKey.roadIdx_ ||
                backLink.contactPoint() != fromContactPointKey.contactPoint_)
            {
                RoadLinkMisMatchError* error = new RoadLinkMisMatchError();
                error->aContactPointKey_ = fromContactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                error->cContactPointKey_ = RoadContactPointKey(backLink.elementRef().index(), backLink.contactPoint());
                error->aToBJunctionIdx_ = -1;
                errors.emplace_back(error);
                return false;
            }
            else
            {
                return validateRoadRoadLaneLinks(map, fromContactPointKey, toContactPointKey, errors);
            }

        case RoadLink::ElementType::JUNCTION:
        {
            int backLinkJunctionIdx = backLink.elementRef().index();
            const auto& junctions = map.junctions();
            const Junction& backLinkJunction = junctions[backLinkJunctionIdx];
            const Junction::Connection* connection = backLinkJunction.findConnection(
                toContactPointKey.roadIdx_, fromContactPointKey.roadIdx_, fromContactPointKey.contactPoint_);
            if (connection)
            {
                // 'fromRoad' is part of a junction. The back link is the
                // incoming connection.
                return validateConnectingIncomingLaneLinks(map, fromContactPointKey, toContactPointKey, *connection,
                                                           errors);
            }
            else if (backLinkJunction.hasOutgoingConnection(fromContactPointKey.roadIdx_,
                                                            fromContactPointKey.contactPoint_))
            {
                // 'fromRoad' is part of a junction. The back link is the
                // outgoing connection.
                return validateConnectingOutgoingLaneLinks(map, fromContactPointKey, toContactPointKey, errors);
            }
            else
            {
                RoadBackLinkNotSpecifiedInJunctionError* error = new RoadBackLinkNotSpecifiedInJunctionError();
                error->aContactPointKey_ = fromContactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                error->aToBJunctionIdx_ = -1;
                error->backLinkJunctionIdx_ = backLinkJunctionIdx;
                errors.emplace_back(error);
                return false;
            }
        }

        default:
            assert(!"Invalid link element type.");
            return false;
    }
}

bool validateIncomingConnectingLink(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                    RoadContactPointKey toContactPointKey, JunctionConnectionKey connectionKey,
                                    std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    const RoadLink& backLink = roadLinkForRoadContactPoint(map, toContactPointKey);
    const Junction& junction = map.junctions()[connectionKey.junctionIdx_];
    const Junction::Connection& conn = junction.connections()[connectionKey.connectionIdx_];

    switch (backLink.elementType())
    {
        case RoadLink::ElementType::NOT_SPECIFIED:
        {
            RoadBackLinkNotSpecifiedError* error = new RoadBackLinkNotSpecifiedError();
            error->aContactPointKey_ = fromContactPointKey;
            error->bContactPointKey_ = toContactPointKey;
            error->aToBJunctionIdx_ = connectionKey.junctionIdx_;
            errors.emplace_back(error);
            return false;
        }

        case RoadLink::ElementType::ROAD:
            if (backLink.elementRef().index() != fromContactPointKey.roadIdx_ ||
                backLink.contactPoint() != fromContactPointKey.contactPoint_)
            {
                RoadLinkMisMatchError* error = new RoadLinkMisMatchError();
                error->aContactPointKey_ = fromContactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                error->cContactPointKey_ = RoadContactPointKey(backLink.elementRef().index(), backLink.contactPoint());
                error->aToBJunctionIdx_ = connectionKey.junctionIdx_;
                errors.emplace_back(error);
                return false;
            }
            else
            {
                return validateIncomingConnectingLaneLinks(map, fromContactPointKey, toContactPointKey, connectionKey,
                                                           errors);
            }

        case RoadLink::ElementType::JUNCTION:
        {
            int backLinkJunctionIdx = backLink.elementRef().index();
            const auto& junctions = map.junctions();
            const Junction& backLinkJunction = junctions[backLinkJunctionIdx];
            if (backLinkJunction.hasConnection(toContactPointKey.roadIdx_, fromContactPointKey.roadIdx_,
                                               fromContactPointKey.contactPoint_))
            {
                // Both 'fromRoad' and 'toRoad' are part of a junction.
                // Since the forward link (from 'fromRoad' to 'toRoad') was the
                // incoming connection, the back link must be the outgoing one.

                InconsistentJunctionPathDirectionsError* error = new InconsistentJunctionPathDirectionsError();
                error->aContactPointKey_ = fromContactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                error->aToBJunctionIdx_ = connectionKey.junctionIdx_;
                error->bToAJunctionIdx_ = backLinkJunctionIdx;
                errors.emplace_back(error);
                return false;
            }
            else if (backLinkJunction.hasOutgoingConnection(fromContactPointKey.roadIdx_,
                                                            fromContactPointKey.contactPoint_))
            {
                // Both 'fromRoad' and 'toRoad' are paths. The connection from
                // 'fromRoad' to 'toRoad' is the incoming connection, the
                // connection from 'toRoad' to 'fromRoad' the outgoing one.
                return validateConnectingConnectingLaneLinks(map, fromContactPointKey, toContactPointKey, conn, errors);
            }
            else
            {
                RoadBackLinkNotSpecifiedInJunctionError* error = new RoadBackLinkNotSpecifiedInJunctionError();
                error->aContactPointKey_ = fromContactPointKey;
                error->bContactPointKey_ = toContactPointKey;
                error->aToBJunctionIdx_ = connectionKey.junctionIdx_;
                error->backLinkJunctionIdx_ = backLinkJunctionIdx;
                errors.emplace_back(error);
                return false;
            }
        }

        default:
            assert(!"Invalid link element type.");
            return false;
    }
}

static const RoadLink& roadLinkForRoadContactPoint(const XodrMap& map, RoadContactPointKey key)
{
    const Road& road = map.roads()[key.roadIdx_];
    return road.roadLink(linkTypeForContactPoint(key.contactPoint_));
}

std::string RoadBackLinkNotSpecifiedError::description(const XodrMap& map) const
{
    std::string aContactPtDesc = aContactPointKey_.toString(map);
    std::string bContactPtDesc = bContactPointKey_.toString(map);

    if (aToBJunctionIdx_ == -1)
    {
        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPtDesc << " is connected to " << bContactPtDesc
             << ", but no link from " << bContactPtDesc << " to " << aContactPtDesc << " is specified.";
        return desc.str();
    }
    else
    {
        const Junction& junction = map.junctions()[aToBJunctionIdx_];

        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPtDesc << " is connected to " << bContactPtDesc
             << " (through junction " << junction.id() << ", but no link from " << bContactPtDesc << " to "
             << aContactPtDesc << " is specified.";
        return desc.str();
    }
}

std::string RoadBackLinkNotSpecifiedInJunctionError::description(const XodrMap& map) const
{
    std::string aContactPtDesc = aContactPointKey_.toString(map);
    std::string bContactPtDesc = bContactPointKey_.toString(map);

    const auto& junctions = map.junctions();
    const Junction& backLinkJunction = junctions[backLinkJunctionIdx_];

    if (aToBJunctionIdx_ == -1)
    {
        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPtDesc << " is connected to " << bContactPtDesc << ", but "
             << bContactPtDesc << " links to junction " << backLinkJunction.id()
             << ", which doesn't contain a connection back to " << aContactPtDesc << ".";
        return desc.str();
    }
    else
    {
        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPtDesc << " is connected to " << bContactPtDesc
             << " (through junction " << junctions[aToBJunctionIdx_].id() << "), but " << bContactPtDesc
             << " links to junction " << backLinkJunction.id() << ", which doesn't contain a connection back to "
             << aContactPtDesc << ".";
        return desc.str();
    }
}

std::string RoadLinkMisMatchError::description(const XodrMap& map) const
{
    std::string aContactPointDesc = aContactPointKey_.toString(map);
    std::string bContactPointDesc = bContactPointKey_.toString(map);
    std::string cContactPointDesc = cContactPointKey_.toString(map);

    if (aToBJunctionIdx_ == -1)
    {
        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPointDesc << " is connected to " << bContactPointDesc
             << ", but " << bContactPointDesc << " links to " << cContactPointDesc << ".";
        return desc.str();
    }
    else
    {
        const Junction& aToBJunction = map.junctions()[aToBJunctionIdx_];

        std::stringstream desc;
        desc << "Road links aren't symmetric: " << aContactPointDesc << " is connected to " << bContactPointDesc
             << " (through junction " << aToBJunction.id() << "), but " << bContactPointDesc << " links to "
             << cContactPointDesc << ".";
        return desc.str();
    }
}

std::string DirectLinkToJunctionRoadError::description(const XodrMap& map) const
{
    const Road& bRoad = map.roads()[bContactPointKey_.roadIdx_];

    std::string aContactPointDesc = aContactPointKey_.toString(map);
    std::string bContactPointDesc = bContactPointKey_.toString(map);

    std::stringstream desc;
    desc << "Road " << bRoad.id() << " is part of a junction, so " << aContactPointDesc
         << " shouldn't link directly to " << bContactPointDesc << ".";
    return desc.str();
}

std::string InconsistentJunctionPathDirectionsError::description(const XodrMap& map) const
{
    std::string aContactPointDesc = aContactPointKey_.toString(map);
    std::string bContactPointDesc = bContactPointKey_.toString(map);

    const Junction& aToBJunction = map.junctions()[aToBJunctionIdx_];
    const Junction& bToAJunction = map.junctions()[bToAJunctionIdx_];

    std::stringstream desc;
    desc << "Inconsistent direction of adjacent junction paths: The connection from " << aContactPointDesc << " to "
         << bContactPointDesc << " in junction " << aToBJunction.id() << " is incoming, so the connection from "
         << bContactPointDesc << " to " << aContactPointDesc << " in junction " << bToAJunction.id()
         << " should be outgoing.";
    return desc.str();
}

}}  // namespace aid::xodr
