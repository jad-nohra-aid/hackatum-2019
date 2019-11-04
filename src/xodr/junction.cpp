#include "junction.h"

#include <climits>

namespace aid { namespace xodr {

bool Junction::hasConnection(int incomingRoadIdx, int connectingRoadIdx, ContactPoint contactPoint) const
{
    assert(contactPoint != ContactPoint::NOT_SPECIFIED);

    for (const Connection& conn : connections_)
    {
        if (conn.incomingRoad().index() == incomingRoadIdx && conn.connectingRoad().index() == connectingRoadIdx &&
            conn.contactPoint() == contactPoint)
        {
            return true;
        }
    }

    return false;
}

const Junction::Connection* Junction::findConnection(int incomingRoadIdx, int connectingRoadIdx,
                                                     ContactPoint contactPoint) const
{
    assert(contactPoint != ContactPoint::NOT_SPECIFIED);

    for (const Connection& conn : connections_)
    {
        if (conn.incomingRoad().index() == incomingRoadIdx && conn.connectingRoad().index() == connectingRoadIdx &&
            conn.contactPoint() == contactPoint)
        {
            return &conn;
        }
    }

    return nullptr;
}

bool Junction::hasOutgoingConnection(int connectingRoadIdx, ContactPoint contactPoint) const
{
    assert(contactPoint != ContactPoint::NOT_SPECIFIED);

    ContactPoint incomingContactPoint = oppositeContactPoint(contactPoint);

    for (const Connection& conn : connections_)
    {
        if (conn.connectingRoad().index() == connectingRoadIdx && conn.contactPoint() == incomingContactPoint)
        {
            return true;
        }
    }

    return false;
}

Junction::Connection* Junction::test_connectionById(const std::string& id)
{
    for (Connection& conn : connections_)
    {
        if (conn.id() == id)
        {
            return &conn;
        }
    }

    return nullptr;
}

LaneIDOpt Junction::Connection::findLaneLinkTarget(LaneID fromLane) const
{
    for (const LaneLink& laneLink : laneLinks_)
    {
        if (laneLink.from() == fromLane)
        {
            return laneLink.to();
        }
    }

    return LaneIDOpt::null();
}

void Junction::Connection::test_setLaneLinkTarget(LaneID fromLaneId, LaneIDOpt toLaneId)
{
    for (int i = 0; i < static_cast<int>(laneLinks_.size()); i++)
    {
        LaneLink& laneLink = laneLinks_[i];
        if (laneLink.from() == fromLaneId)
        {
            if (toLaneId)
            {
                laneLink.test_setTo(*toLaneId);
            }
            else
            {
                laneLinks_.erase(laneLinks_.begin() + i);
            }
            return;
        }
    }

    if (toLaneId)
    {
        laneLinks_.push_back(LaneLink(fromLaneId, *toLaneId));
    }
}

}}  // namespace aid::xodr