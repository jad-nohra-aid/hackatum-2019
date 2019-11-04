#include "validation/lane_link_validation.h"

#include <cassert>

#include "xodr_map.h"
#include "validation/road_link_validation.h"

namespace aid { namespace xodr {

/**
 * @brief Validates whether 'toLaneId' is in the required range.
 *
 * The valid range for 'toLaneId' is the range of all lanes in the destination
 * lane section which are of the direction matching the direction of the
 * 'from' lane.
 *
 * This test is common to many of the more specific cases, so that's why it's
 * separated into a function of its own.
 *
 * @param fromLaneId            The lane id the lane link originates from. This
 *                              lane id is assumed to be valid, and assumed to
 *                              not refer to the center lane.
 * @param toLaneId              The lane id of the target of the lane link.
 * @param roadsOpposingDirections   Whether the road containing the from lane
 *                              and the road containing the to lane have
 *                              opposing directions.
 * @param toLaneIDsMin          The (inclusive) lower bound of the range of
 *                              valid 'to' lane id's.
 * @param toLaneIDsMax          The (exclusive) upper bound of the range of
 *                              valid 'to' lane id's.
 */
static bool validateLaneLinkInRange(LaneSectionContactPointKey aSectionContactPointKey,
                                    LaneSectionContactPointKey bSectionContactPointKey, LaneID fromLaneId,
                                    LaneID toLaneId, bool roadsOpposingDirections, LaneID toLaneIDsMin,
                                    LaneID toLaneIDsMax, std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    if (toLaneId == LaneID(0))
    {
        LaneLinkToCenterLaneError* error = new LaneLinkToCenterLaneError();
        error->aSectionContactPointKey_ = aSectionContactPointKey;
        error->bSectionContactPointKey_ = bSectionContactPointKey;
        error->fromLaneId_ = fromLaneId;
        errors.emplace_back(error);
        return false;
    }

    // If the two roads have the same direction (ie, one is the
    // successor of the other, and the other the predecessor of the
    // first one), then the two lanes must be on the same side of their
    // respective reference line.
    //
    // If the two roads have opposing directions (ie, they meet head to
    // head or tail to tail), then the two lanes must be on opposite
    // sides of their respective reference lines.
    if (fromLaneId.sameSide(toLaneId) == roadsOpposingDirections)
    {
        LaneLinkOpposingDirections* error = new LaneLinkOpposingDirections();
        error->aSectionContactPointKey_ = aSectionContactPointKey;
        error->bSectionContactPointKey_ = bSectionContactPointKey;
        error->aLaneId_ = fromLaneId;
        error->bLaneId_ = toLaneId;
        errors.emplace_back(error);
        return false;
    }

    if (toLaneId < toLaneIDsMin || toLaneId > toLaneIDsMax)
    {
        LaneLinkTargetOutOfRange* error = new LaneLinkTargetOutOfRange();
        error->aSectionContactPointKey_ = aSectionContactPointKey;
        error->bSectionContactPointKey_ = bSectionContactPointKey;
        error->aLaneId_ = fromLaneId;
        error->bLaneId_ = toLaneId;
        errors.emplace_back(error);
        return false;
    }

    return true;
}

static LaneSectionContactPointKey fromRoadToLaneSectionContactPointKey(const XodrMap& map,
                                                                       RoadContactPointKey roadContactPointKey)
{
    const Road& road = map.roads()[roadContactPointKey.roadIdx_];
    int laneSectionIdx = road.laneSectionIndexForContactPoint(roadContactPointKey.contactPoint_);
    return LaneSectionContactPointKey(roadContactPointKey.roadIdx_, laneSectionIdx, roadContactPointKey.contactPoint_);
}

bool validateRoadRoadLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                               RoadContactPointKey toContactPointKey,
                               std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneSectionContactPointKey fromSectionContactPointKey =
        fromRoadToLaneSectionContactPointKey(map, fromContactPointKey);
    LaneSectionContactPointKey toSectionContactPointKey = fromRoadToLaneSectionContactPointKey(map, toContactPointKey);

    const LaneSection& fromLaneSection = laneSectionByKey(map, fromSectionContactPointKey.laneSectionKey());
    const LaneSection& toLaneSection = laneSectionByKey(map, toSectionContactPointKey.laneSectionKey());

    LaneID toLaneIDsMin(-toLaneSection.numRightLanes());
    LaneID toLaneIDsMax(toLaneSection.numLeftLanes());

    RoadLinkType linkType = linkTypeForContactPoint(fromContactPointKey.contactPoint_);
    RoadLinkType backLinkType = linkTypeForContactPoint(toContactPointKey.contactPoint_);

    // The roads are of opposing direction if the link is between two start
    // points or between to end points. Otherwise, they're of the same direction.
    bool roadsOpposingDirections = fromContactPointKey.contactPoint_ == toContactPointKey.contactPoint_;

    const auto& fromLanes = fromLaneSection.lanes();
    for (int i = 0; i < static_cast<int>(fromLanes.size()); i++)
    {
        const LaneSection::Lane& fromLane = fromLanes[i];
        if (fromLane.hasLink(linkType))
        {
            LaneID fromLaneId = fromLaneSection.laneIndexToId(i);
            LaneID toLaneId = fromLane.link(linkType);

            if (validateLaneLinkInRange(fromSectionContactPointKey, toSectionContactPointKey, fromLaneId, toLaneId,
                                        roadsOpposingDirections, toLaneIDsMin, toLaneIDsMax, errors))
            {
                const LaneSection::Lane& bLane = toLaneSection.laneById(toLaneId);
                if (bLane.hasLink(backLinkType))
                {
                    LaneID backLinkId = bLane.link(backLinkType);
                    if (backLinkId != fromLaneId)
                    {
                        LaneLinkMisMatch* error = new LaneLinkMisMatch();
                        error->aSectionContactPointKey_ = fromSectionContactPointKey;
                        error->bSectionContactPointKey_ = toSectionContactPointKey;
                        error->aLaneId_ = fromLaneId;
                        error->bLaneId_ = toLaneId;
                        error->cLaneId_ = backLinkId;
                        errors.emplace_back(error);
                        success = false;
                    }
                }
                else
                {
                    LaneBackLinkNotSpecified* error = new LaneBackLinkNotSpecified();
                    error->aSectionContactPointKey_ = fromSectionContactPointKey;
                    error->bSectionContactPointKey_ = toSectionContactPointKey;
                    error->aLaneId_ = fromLaneId;
                    error->bLaneId_ = toLaneId;
                    errors.emplace_back(error);
                    success = false;
                }
            }
            else
            {
                success = false;
            }
        }
    }

    return success;
}

bool validateConnectingIncomingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey,
                                         const Junction::Connection& backLinkConnection,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneSectionContactPointKey fromSectionContactPointKey =
        fromRoadToLaneSectionContactPointKey(map, fromContactPointKey);
    LaneSectionContactPointKey toSectionContactPointKey = fromRoadToLaneSectionContactPointKey(map, toContactPointKey);

    const LaneSection& fromLaneSection = laneSectionByKey(map, fromSectionContactPointKey.laneSectionKey());
    const LaneSection& toLaneSection = laneSectionByKey(map, toSectionContactPointKey.laneSectionKey());

    LaneID toLaneIDsMin(-toLaneSection.numRightLanes());
    LaneID toLaneIDsMax(toLaneSection.numLeftLanes());

    RoadLinkType linkType = linkTypeForContactPoint(fromContactPointKey.contactPoint_);

    // The roads are of opposing direction if the link is between two start
    // points or between to end points. Otherwise, they're of the same direction.
    bool roadsOpposingDirections = fromContactPointKey.contactPoint_ == toContactPointKey.contactPoint_;

    const auto& fromLanes = fromLaneSection.lanes();
    for (int i = 0; i < static_cast<int>(fromLanes.size()); i++)
    {
        const LaneSection::Lane& fromLane = fromLanes[i];
        if (fromLane.hasLink(linkType))
        {
            LaneID fromLaneId = fromLaneSection.laneIndexToId(i);
            LaneID toLaneId = fromLane.link(linkType);

            if (validateLaneLinkInRange(fromSectionContactPointKey, toSectionContactPointKey, fromLaneId, toLaneId,
                                        roadsOpposingDirections, toLaneIDsMin, toLaneIDsMax, errors))
            {
                LaneIDOpt backLinkIdOpt = backLinkConnection.findLaneLinkTarget(toLaneId);
                if (backLinkIdOpt)
                {
                    LaneID backLinkId = *backLinkIdOpt;
                    if (backLinkId != fromLaneId)
                    {
                        LaneLinkMisMatch* error = new LaneLinkMisMatch();
                        error->aSectionContactPointKey_ = fromSectionContactPointKey;
                        error->bSectionContactPointKey_ = toSectionContactPointKey;
                        error->aLaneId_ = fromLaneId;
                        error->bLaneId_ = toLaneId;
                        error->cLaneId_ = backLinkId;
                        errors.emplace_back(error);
                        success = false;
                    }
                }
                else
                {
                    LaneBackLinkNotSpecified* error = new LaneBackLinkNotSpecified();
                    error->aSectionContactPointKey_ = fromSectionContactPointKey;
                    error->bSectionContactPointKey_ = toSectionContactPointKey;
                    error->aLaneId_ = fromLaneId;
                    error->bLaneId_ = toLaneId;
                    errors.emplace_back(error);
                    success = false;
                }
            }
        }
    }

    return success;
}

bool validateConnectingOutgoingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneSectionContactPointKey fromSectionContactPointKey =
        fromRoadToLaneSectionContactPointKey(map, fromContactPointKey);
    LaneSectionContactPointKey toSectionContactPointKey = fromRoadToLaneSectionContactPointKey(map, toContactPointKey);

    const LaneSection& fromLaneSection = laneSectionByKey(map, fromSectionContactPointKey.laneSectionKey());
    const LaneSection& toLaneSection = laneSectionByKey(map, toSectionContactPointKey.laneSectionKey());

    LaneID toLaneIDsMin(-toLaneSection.numRightLanes());
    LaneID toLaneIDsMax(toLaneSection.numLeftLanes());

    RoadLinkType linkType = linkTypeForContactPoint(fromContactPointKey.contactPoint_);

    // The roads are of opposing direction if the link is between two start
    // points or between to end points. Otherwise, they're of the same direction.
    bool roadsOpposingDirections = fromContactPointKey.contactPoint_ == toContactPointKey.contactPoint_;

    const auto& fromLanes = fromLaneSection.lanes();
    for (int i = 0; i < static_cast<int>(fromLanes.size()); i++)
    {
        const LaneSection::Lane& fromLane = fromLanes[i];
        if (fromLane.hasLink(linkType))
        {
            LaneID fromLaneId = fromLaneSection.laneIndexToId(i);
            LaneID toLaneId = fromLane.link(linkType);

            success &= validateLaneLinkInRange(fromSectionContactPointKey, toSectionContactPointKey, fromLaneId,
                                               toLaneId, roadsOpposingDirections, toLaneIDsMin, toLaneIDsMax, errors);
        }
    }

    return success;
}

bool validateIncomingConnectingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                         RoadContactPointKey toContactPointKey, JunctionConnectionKey connectionKey,
                                         std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneSectionContactPointKey fromSectionContactPointKey =
        fromRoadToLaneSectionContactPointKey(map, fromContactPointKey);
    LaneSectionContactPointKey toSectionContactPointKey = fromRoadToLaneSectionContactPointKey(map, toContactPointKey);

    const LaneSection& fromLaneSection = laneSectionByKey(map, fromSectionContactPointKey.laneSectionKey());
    const LaneSection& toLaneSection = laneSectionByKey(map, toSectionContactPointKey.laneSectionKey());

    const Junction::Connection& connection =
        map.junctions()[connectionKey.junctionIdx_].connections()[connectionKey.connectionIdx_];

    LaneID fromLaneIDsMin(-fromLaneSection.numRightLanes());
    LaneID fromLaneIDsMax(fromLaneSection.numLeftLanes());

    LaneID toLaneIDsMin(-toLaneSection.numRightLanes());
    LaneID toLaneIDsMax(toLaneSection.numLeftLanes());

    RoadLinkType backLinkType = linkTypeForContactPoint(toContactPointKey.contactPoint_);

    // The roads are of opposing direction if the link is between two start
    // points or between to end points. Otherwise, they're of the same direction.
    bool roadsOpposingDirections = fromContactPointKey.contactPoint_ == toContactPointKey.contactPoint_;

    const auto& laneLinks = connection.laneLinks();
    for (int i = 0; i < static_cast<int>(laneLinks.size()); i++)
    {
        const Junction::LaneLink& laneLink = laneLinks[i];
        LaneID fromLaneId = laneLink.from();
        LaneID toLaneId = laneLink.to();

        bool curSuccess = true;

        if (fromLaneId != LaneID(0) && toLaneId == LaneID(0))
        {
            LaneLinkToCenterLaneError* error = new LaneLinkToCenterLaneError();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->fromLaneId_ = fromLaneId;
            errors.emplace_back(error);
            curSuccess = false;
        }
        else if (fromLaneId == LaneID(0) && toLaneId != LaneID(0))
        {
            LaneLinkToCenterLaneError* error = new LaneLinkToCenterLaneError();
            error->aSectionContactPointKey_ = toSectionContactPointKey;
            error->bSectionContactPointKey_ = fromSectionContactPointKey;
            error->fromLaneId_ = toLaneId;
            errors.emplace_back(error);
            curSuccess = false;
        }

        if (toLaneId < toLaneIDsMin || toLaneId > toLaneIDsMax)
        {
            LaneLinkTargetOutOfRange* error = new LaneLinkTargetOutOfRange();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->aLaneId_ = fromLaneId;
            error->bLaneId_ = toLaneId;
            errors.emplace_back(error);
            curSuccess = false;
        }

        if (fromLaneId < fromLaneIDsMin || fromLaneId > fromLaneIDsMax)
        {
            LaneLinkTargetOutOfRange* error = new LaneLinkTargetOutOfRange();
            error->aSectionContactPointKey_ = toSectionContactPointKey;
            error->bSectionContactPointKey_ = fromSectionContactPointKey;
            error->aLaneId_ = toLaneId;
            error->bLaneId_ = fromLaneId;
            errors.emplace_back(error);
            curSuccess = false;
        }

        if (!curSuccess)
        {
            success = false;
            continue;
        }

        if (fromLaneId == LaneID(0) || toLaneId == LaneID(0))
        {
            continue;
        }

        // If the two roads have the same direction (ie, one is the successor of
        // the other, and the other the predecessor of the first one), then the
        // two lanes must be on the same side of their respective reference line.
        //
        // If the two roads have opposing directions (ie, they meet head to head
        // or tail to tail), then the two lanes must be on opposite sides of
        // their respective reference lines.
        if (fromLaneId.sameSide(toLaneId) == roadsOpposingDirections)
        {
            LaneLinkOpposingDirections* error = new LaneLinkOpposingDirections();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->aLaneId_ = fromLaneId;
            error->bLaneId_ = toLaneId;
            errors.emplace_back(error);
            success = false;
            continue;
        }

        const LaneSection::Lane& bLane = toLaneSection.laneById(toLaneId);
        if (bLane.hasLink(backLinkType))
        {
            LaneID backLinkId = bLane.link(backLinkType);
            if (backLinkId != fromLaneId)
            {
                LaneLinkMisMatch* error = new LaneLinkMisMatch();
                error->aSectionContactPointKey_ = fromSectionContactPointKey;
                error->bSectionContactPointKey_ = toSectionContactPointKey;
                error->aLaneId_ = fromLaneId;
                error->bLaneId_ = toLaneId;
                error->cLaneId_ = backLinkId;
                errors.emplace_back(error);
                success = false;
            }
        }
        else
        {
            LaneBackLinkNotSpecified* error = new LaneBackLinkNotSpecified();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->aLaneId_ = fromLaneId;
            error->bLaneId_ = toLaneId;
            errors.emplace_back(error);
            success = false;
        }
    }

    return success;
}

bool validateConnectingConnectingLaneLinks(const XodrMap& map, RoadContactPointKey fromContactPointKey,
                                           RoadContactPointKey toContactPointKey,
                                           const Junction::Connection& linkConnection,
                                           std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneSectionContactPointKey fromSectionContactPointKey =
        fromRoadToLaneSectionContactPointKey(map, fromContactPointKey);
    LaneSectionContactPointKey toSectionContactPointKey = fromRoadToLaneSectionContactPointKey(map, toContactPointKey);

    const LaneSection& fromLaneSection = laneSectionByKey(map, fromSectionContactPointKey.laneSectionKey());
    const LaneSection& toLaneSection = laneSectionByKey(map, toSectionContactPointKey.laneSectionKey());

    LaneID fromLaneIDsMin(-fromLaneSection.numRightLanes());
    LaneID fromLaneIDsMax(fromLaneSection.numLeftLanes());

    LaneID toLaneIDsMin(-toLaneSection.numRightLanes());
    LaneID toLaneIDsMax(toLaneSection.numLeftLanes());

    for (const Junction::LaneLink& laneLink : linkConnection.laneLinks())
    {
        LaneID fromLaneId = laneLink.from();
        LaneID toLaneId = laneLink.to();

        if (fromLaneId != LaneID(0) && toLaneId == LaneID(0))
        {
            LaneLinkToCenterLaneError* error = new LaneLinkToCenterLaneError();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->fromLaneId_ = fromLaneId;
            errors.emplace_back(error);
            success = false;
        }
        else if (fromLaneId == LaneID(0) && toLaneId != LaneID(0))
        {
            LaneLinkToCenterLaneError* error = new LaneLinkToCenterLaneError();
            error->aSectionContactPointKey_ = toSectionContactPointKey;
            error->bSectionContactPointKey_ = fromSectionContactPointKey;
            error->fromLaneId_ = toLaneId;
            errors.emplace_back(error);
            success = false;
        }

        if (fromLaneId < fromLaneIDsMin || fromLaneId > fromLaneIDsMax)
        {
            LaneLinkTargetOutOfRange* error = new LaneLinkTargetOutOfRange();
            error->aSectionContactPointKey_ = toSectionContactPointKey;
            error->bSectionContactPointKey_ = fromSectionContactPointKey;
            error->aLaneId_ = toLaneId;
            error->bLaneId_ = fromLaneId;
            errors.emplace_back(error);
            success = false;
        }

        if (toLaneId < toLaneIDsMin || toLaneId > toLaneIDsMax)
        {
            LaneLinkTargetOutOfRange* error = new LaneLinkTargetOutOfRange();
            error->aSectionContactPointKey_ = fromSectionContactPointKey;
            error->bSectionContactPointKey_ = toSectionContactPointKey;
            error->aLaneId_ = fromLaneId;
            error->bLaneId_ = toLaneId;
            errors.emplace_back(error);
            success = false;
        }
    }

    return success;
}

bool validateRoadInternalLaneLinks(const XodrMap& map, int roadIdx,
                                   std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    const Road& road = map.roads()[roadIdx];

    const auto& laneSections = road.laneSections();
    for (int i = 0; i < static_cast<int>(laneSections.size()) - 1; i++)
    {
        LaneSectionContactPointKey key1(roadIdx, i, ContactPoint::END);
        LaneSectionContactPointKey key2(roadIdx, i + 1, ContactPoint::START);
        success &= validateLaneLinks(laneSections[i], laneSections[i + 1], key1, key2, errors);
        success &= validateLaneLinks(laneSections[i + 1], laneSections[i], key2, key1, errors);
    }

    return success;
}

bool validateLaneLinks(const LaneSection& fromSection, const LaneSection& toSection,
                       const LaneSectionContactPointKey& fromContactPointKey,
                       const LaneSectionContactPointKey& toContactPointKey,
                       std::vector<std::unique_ptr<LinkValidationError>>& errors)
{
    bool success = true;

    LaneID toLaneIDsMin(-toSection.numRightLanes());
    LaneID toLaneIDsMax(toSection.numLeftLanes());

    // The roads are of opposing direction if the link is between two start
    // points or between to end points. Otherwise, they're of the same direction.
    bool roadsOpposingDirections = fromContactPointKey.contactPoint_ == toContactPointKey.contactPoint_;

    RoadLinkType linkType = linkTypeForContactPoint(fromContactPointKey.contactPoint_);
    RoadLinkType backLinkType = linkTypeForContactPoint(toContactPointKey.contactPoint_);

    const auto& fromLanes = fromSection.lanes();
    for (int i = 0; i < static_cast<int>(fromLanes.size()); i++)
    {
        const LaneSection::Lane& fromLane = fromLanes[i];
        if (fromLane.hasLink(linkType))
        {
            LaneID fromLaneId = fromSection.laneIndexToId(i);
            LaneID toLaneId = fromLane.link(linkType);

            if (validateLaneLinkInRange(fromContactPointKey, toContactPointKey, fromLaneId, toLaneId,
                                        roadsOpposingDirections, toLaneIDsMin, toLaneIDsMax, errors))
            {
                const LaneSection::Lane& bLane = toSection.laneById(toLaneId);
                if (bLane.hasLink(backLinkType))
                {
                    LaneID backLinkId = bLane.link(backLinkType);
                    if (backLinkId != fromLaneId)
                    {
                        LaneLinkMisMatch* error = new LaneLinkMisMatch();
                        error->aSectionContactPointKey_ = fromContactPointKey;
                        error->bSectionContactPointKey_ = toContactPointKey;
                        error->aLaneId_ = fromLaneId;
                        error->bLaneId_ = toLaneId;
                        error->cLaneId_ = backLinkId;
                        errors.emplace_back(error);
                        success = false;
                    }
                }
                else
                {
                    LaneBackLinkNotSpecified* error = new LaneBackLinkNotSpecified();
                    error->aSectionContactPointKey_ = fromContactPointKey;
                    error->bSectionContactPointKey_ = toContactPointKey;
                    error->aLaneId_ = fromLaneId;
                    error->bLaneId_ = toLaneId;
                    errors.emplace_back(error);
                    success = false;
                }
            }
            else
            {
                success = false;
            }
        }
    }

    return success;
}

std::string LaneBackLinkNotSpecified::description(const XodrMap& map) const
{
    std::stringstream desc;
    desc << "Lane links aren't symmetric between A = " << aSectionContactPointKey_.toString(map)
         << " and B = " << bSectionContactPointKey_.toString(map) << ": Lane " << aLaneId_
         << " in section A links to lane " << bLaneId_ << " in section B, but lane " << bLaneId_
         << " in section B doesn't link back to a lane in section A.";
    return desc.str();
}

std::string LaneLinkMisMatch::description(const XodrMap& map) const
{
    std::stringstream desc;
    desc << "Lane links aren't symmetric between A = " << aSectionContactPointKey_.toString(map)
         << " and B = " << bSectionContactPointKey_.toString(map) << ": Lane " << aLaneId_
         << " in section A links to lane " << bLaneId_ << " in section B, but lane " << bLaneId_
         << " in section B links to lane " << cLaneId_ << " in section A.";
    return desc.str();
}

std::string LaneLinkToCenterLaneError::description(const XodrMap&) const
{
    std::stringstream err;
    err << "Non-center lane " << fromLaneId_ << " shouldn't link to a center lane.";
    return err.str();
}

std::string LaneLinkTargetOutOfRange::description(const XodrMap& map) const
{
    std::stringstream desc;
    desc << "Lane link out of range between A = " << aSectionContactPointKey_.toString(map)
         << " and B = " << bSectionContactPointKey_.toString(map) << ": Lane " << aLaneId_
         << " in section A tried to link to lane " << bLaneId_
         << " in section B, but this lane doesn't exist in section B.";
    return desc.str();
}

std::string LaneLinkOpposingDirections::description(const XodrMap& map) const
{
    std::stringstream desc;
    desc << "Lane " << aLaneId_ << " in " << aSectionContactPointKey_.toString(map) << " links to lane " << bLaneId_
         << " in " << bSectionContactPointKey_.toString(map) << ", which is of opposing driving direction.";
    return desc.str();
}

}}  // namespace aid::xodr
