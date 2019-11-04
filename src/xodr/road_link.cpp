#include "road_link.h"

namespace aid { namespace xodr {

std::string roadLinkTypeName(RoadLinkType roadLinkType)
{
    switch (roadLinkType)
    {
        default:
            assert(!"Invalid RoadLinkType");

        case RoadLinkType::PREDECESSOR:
            return "predecessor";

        case RoadLinkType::SUCCESSOR:
            return "successor";
    }
}

std::string contactPointName(ContactPoint contactPoint)
{
    switch (contactPoint)
    {
        default:
            assert(!"Invalid ContactPoint");

        case ContactPoint::START:
            return "start";

        case ContactPoint::END:
            return "end";
    }
}

RoadLinkType linkTypeForContactPoint(ContactPoint contactPoint)
{
    switch (contactPoint)
    {
        default:
            assert(!"Invalid ContactPoint");

        case ContactPoint::START:
            return RoadLinkType::PREDECESSOR;

        case ContactPoint::END:
            return RoadLinkType::SUCCESSOR;
    }
}

ContactPoint contactPointForLinkType(RoadLinkType linkType)
{
    switch (linkType)
    {
        default:
            assert(!"Invalid link type.");

        case RoadLinkType::PREDECESSOR:
            return ContactPoint::START;

        case RoadLinkType::SUCCESSOR:
            return ContactPoint::END;
    }
}

ContactPoint oppositeContactPoint(ContactPoint contactPoint)
{
    switch (contactPoint)
    {
        default:
            assert(!"Invalid ContactPoint");

        case ContactPoint::START:
            return ContactPoint::END;

        case ContactPoint::END:
            return ContactPoint::START;
    }
}

RoadLink::RoadLink() : elementType_(ElementType::NOT_SPECIFIED) {}

RoadLink RoadLink::roadLink(XodrObjectReference roadRef, ContactPoint contactPoint)
{
    RoadLink ret;
    ret.elementType_ = ElementType::ROAD;
    ret.contactPoint_ = contactPoint;
    ret.elementRef_ = roadRef;
    return ret;
}

RoadLink RoadLink::junctionLink(XodrObjectReference junctionRef)
{
    RoadLink ret;
    ret.elementType_ = ElementType::JUNCTION;
    ret.elementRef_ = junctionRef;
    return ret;
}

ContactPoint RoadLink::contactPoint() const
{
    assert(elementType_ == ElementType::ROAD);
    return contactPoint_;
}

const XodrObjectReference& RoadLink::elementRef() const
{
    assert(elementType_ != ElementType::NOT_SPECIFIED);
    return elementRef_;
}

NeighborLink::Side NeighborLink::side() const
{
    assert(isSpecified());
    return side_;
}

NeighborLink::Direction NeighborLink::direction() const
{
    assert(isSpecified());
    return direction_;
}

const XodrObjectReference& NeighborLink::elementRef() const
{
    assert(isSpecified());
    return elementRef_;
}

}}  // namespace aid::xodr
