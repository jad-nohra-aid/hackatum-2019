#pragma once

namespace aid { namespace xodr {

template <class F>
void forEachRoadLink(const XodrMap& map, F&& f)
{
    forEachRoadLink(map, f, [&](RoadContactPointKey aKey, RoadContactPointKey bKey, const xodr::Junction::Connection&) {
        f(aKey, bKey);
    });
}

template <class RoadRoadF, class JunctionRoadF>
void forEachRoadLink(const XodrMap& map, RoadRoadF&& roadRoadF, JunctionRoadF&& junctionRoadF)
{
    // Most connections between adjacent roads A, B are specified twice in an
    // OpenDRIVE file. Once from road A to road B, and once from road B to A.
    // The exceptions are connections between connecting roads and outgoing
    // roads, which are only specified from the connecting road to the outgoing
    // road, but not in the other direction.
    //
    // To make sure that each connection is converted exactly once, we iterate
    // over each road A, and then convert the following connections:
    //
    //  1. If road A is part of a junction, then we convert the connection to
    //     any adjacent road B to which there is connectivity information in A.
    //
    //     This takes care of connections from connecting roads to normal roads
    //     (where the normal road can be either the incoming or outgoing road).
    //
    //     It also takes care of connections between two connecting roads (in
    //     different junctions). In this case, since connectivity information is
    //     available in road A, road A must be the incoming road to road B and
    //     road B the outgoing road to road A, which means there's no
    //     connectivity information from road B back to road A. For this reason,
    //     the connection will be converted only once, as required.
    //
    //  2. If road A is not part of a junction, then convert the connection to
    //     any adjacent road B, if road B is not part of a junction and if the
    //     index of road A is lower than the index of road B.
    //
    //     If road B is part of a junction, then because of case 1, the
    //     connection from road B to road A is converted instead.
    //
    //     If road B is not part of a junction, then comparing the indices will
    //     make sure the pair is handled only once (connectivity information
    //     is available in both directions).

    const auto& roads = map.roads();
    for (int roadIdx = 0; roadIdx < static_cast<int>(roads.size()); roadIdx++)
    {
        const Road& road = roads[roadIdx];

        for (const RoadLinkType roadLinkType : {RoadLinkType::PREDECESSOR, RoadLinkType::SUCCESSOR})
        {
            const RoadLink& roadLink = road.roadLink(roadLinkType);
            switch (roadLink.elementType())
            {
                default:
                    assert(!"Invalid element type");

                case RoadLink::ElementType::ROAD:
                {
                    int otherRoadIdx = roadLink.elementRef().index();

                    // See the comment block at the beginning of this function for the
                    // reason for this check.
                    if (road.junctionRef().hasValue() || roadIdx < otherRoadIdx)
                    {
                        RoadContactPointKey fromKey(roadIdx, contactPointForLinkType(roadLinkType));
                        RoadContactPointKey toKey(otherRoadIdx, roadLink.contactPoint());
                        roadRoadF(fromKey, toKey);
                    }
                }
                break;

                case RoadLink::ElementType::JUNCTION:
                {
                    // See the comment block at the beginning of this function for the
                    // reason for this check.
                    if (road.junctionRef().hasValue())
                    {
                        int junctionIdx = roadLink.elementRef().index();
                        const Junction& junction = map.junctions()[junctionIdx];

                        for (const Junction::Connection& connection : junction.connections())
                        {
                            if (connection.incomingRoad().index() == roadIdx)
                            {
                                RoadContactPointKey fromKey(roadIdx, contactPointForLinkType(roadLinkType));
                                RoadContactPointKey toKey(connection.connectingRoad().index(), roadLink.contactPoint());
                                junctionRoadF(fromKey, toKey, connection);
                            }
                        }
                    }
                }
                break;

                case RoadLink::ElementType::NOT_SPECIFIED:
                    break;
            }
        }
    }
}

}}  // namespace aid::xodr