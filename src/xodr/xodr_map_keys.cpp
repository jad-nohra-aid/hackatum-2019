#include "xodr_map_keys.h"

#include <sstream>

#include "xodr_map.h"
#include "lane_section.h"

namespace aid { namespace xodr {

bool RoadContactPointKey::operator==(const RoadContactPointKey& b) const
{
    return roadIdx_ == b.roadIdx_ && contactPoint_ == b.contactPoint_;
}

bool RoadContactPointKey::operator<(const RoadContactPointKey& b) const
{
    if (roadIdx_ != b.roadIdx_)
    {
        return roadIdx_ < b.roadIdx_;
    }
    else
    {
        return contactPoint_ < b.contactPoint_;
    }
}

std::string RoadContactPointKey::toString(const XodrMap& map) const
{
    const Road& road = map.roads()[roadIdx_];

    std::stringstream ret;
    ret << "[road: " << road.id() << ", contact point: " << contactPointName(contactPoint_) << "]";
    return ret.str();
}

const LaneSection& laneSectionByKey(const XodrMap& map, LaneSectionKey key)
{
    const Road& road = map.roads()[key.roadIdx_];
    return road.laneSections()[key.laneSectionIdx_];
}

std::string LaneSectionContactPointKey::toString(const XodrMap& map) const
{
    const Road& road = map.roads()[roadIdx_];

    std::stringstream ret;
    ret << "[road: '" << road.id() << "', lane section: " << laneSectionIdx_
        << ", contact point: " << contactPointName(contactPoint_) << "]";
    return ret.str();
}

}}  // namespace aid::xodr
