#include "road.h"

namespace aid { namespace xodr {

const ElevationProfile& Road::elevationProfile() const
{
    assert(hasElevationProfile());
    return *elevationProfile_;
}

const RoadLink& Road::roadLink(RoadLinkType roadLinkType) const
{
    switch (roadLinkType)
    {
        default:
            assert(!"Invalid RoadLinkType.");

        case RoadLinkType::PREDECESSOR:
            return predecessor();

        case RoadLinkType::SUCCESSOR:
            return successor();
    }
}

int Road::laneSectionIndexForContactPoint(ContactPoint contactPoint) const
{
    switch (contactPoint)
    {
        default:
            assert(!"Invalid ContactPoint");

        case ContactPoint::START:
            return 0;

        case ContactPoint::END:
            return laneSections_.size() - 1;
    }
}

LaneSection& Road::laneSectionForContactPoint(ContactPoint contactPoint)
{
    return laneSections_[laneSectionIndexForContactPoint(contactPoint)];
}

const LaneSection& Road::laneSectionForContactPoint(ContactPoint contactPoint) const
{
    return laneSections_[laneSectionIndexForContactPoint(contactPoint)];
}

int Road::laneSectionIndexForExternalLinkType(RoadLinkType linkType) const
{
    switch (linkType)
    {
        default:
            assert(!"Invalid RoadLinkType");

        case RoadLinkType::PREDECESSOR:
            return 0;

        case RoadLinkType::SUCCESSOR:
            return laneSections_.size() - 1;
    }
}

LaneSection& Road::laneSectionForExternalLinkType(RoadLinkType linkType)
{
    switch (linkType)
    {
        default:
            assert(!"Invalid RoadLinkType");

        case RoadLinkType::PREDECESSOR:
            return laneSections_.front();

        case RoadLinkType::SUCCESSOR:
            return laneSections_.back();
    }
}

void Road::validate() const
{
    for (const LaneSection& laneSection : laneSections_)
    {
        laneSection.validate();
    }
}

}}  // namespace aid::xodr
