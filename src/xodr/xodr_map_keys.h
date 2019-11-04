#pragma once

#include "road_link.h"

namespace aid { namespace xodr {

class XodrMap;
class LaneSection;

/**
 * @brief A struct which contains the information needed to identify a contact point
 * within an @ref XodrMap.
 */
struct RoadContactPointKey
{
    RoadContactPointKey() = default;

    /**
     * @brief Constructs a RoadContactPointKey from a road index and contact point.
     *
     * @param roadIdx       The road index.
     * @param contactPoint  The contact point.
     */
    RoadContactPointKey(int roadIdx, ContactPoint contactPoint) : roadIdx_(roadIdx), contactPoint_(contactPoint) {}

    /**
     * @brief Compares two RoadContactPointKey's for equality.
     */
    bool operator==(const RoadContactPointKey& b) const;

    /**
     * @brief Compares whether this RoadContactPointKey is less than the given RoadContactPointKey.
     *
     * RoadContactPointKey are ordered first by their road index, then by their
     * contactPoint_ if the road indices are equal.
     */
    bool operator<(const RoadContactPointKey& b) const;

    /**
     * @brief Gets a human readable description of this RoadContactPointKey.
     *
     * @param map           The XodrMap to which this RoadContactPointKey applies.
     * @return              The human readable description.
     */
    std::string toString(const XodrMap& map) const;

    /**
     * @brief The index of the road which contains the contact point.
     */
    int roadIdx_;

    /**
     * @brief The type of the contact point.
     */
    ContactPoint contactPoint_;
};

/**
 * @brief A struct which contains the information needed to identify a lane
 * section within an @ref XodrMap.
 */
struct LaneSectionKey
{
    LaneSectionKey() = default;

    /**
     * @brief Constructs a LaneSectionKey from a road index and a lane section index.
     *
     * @param roadIdx           The road index.
     * @param laneSectionIdx    The lane section index.
     */
    LaneSectionKey(int roadIdx, int laneSectionIdx) : roadIdx_(roadIdx), laneSectionIdx_(laneSectionIdx) {}

    /**
     * @brief Compares two LaneSectionKey's for equality.
     */
    bool operator==(const LaneSectionKey& b) const
    {
        return roadIdx_ == b.roadIdx_ && laneSectionIdx_ == b.laneSectionIdx_;
    }

    /**
     * @brief The index of the road which contains the lane section.
     */
    int roadIdx_;

    /**
     * @brief The index of the lane section within its road.
     */
    int laneSectionIdx_;
};

/**
 * @brief Gets the lane section with the given key from the given map.
 *
 * @param map               The map which contains the lane section.
 * @param key               The key of the lane section within the map.
 * @return                  The lane section.
 */
const LaneSection& laneSectionByKey(const XodrMap& map, LaneSectionKey key);

/**
 * @brief A struct which contains the information needed to identify a lane
 * section contact point within an @ref XodrMap.
 */
struct LaneSectionContactPointKey
{
    LaneSectionContactPointKey() = default;

    /**
     * @brief Constructs a LaneSectionContactPointKey from its components.
     *
     * @param roadIdx           The road index.
     * @param laneSectionIdx    The lane section index.
     * @param contactPoint      The contact point.
     */
    LaneSectionContactPointKey(int roadIdx, int laneSectionIdx, ContactPoint contactPoint)
    {
        roadIdx_ = roadIdx;
        laneSectionIdx_ = laneSectionIdx;
        contactPoint_ = contactPoint;
    }

    /**
     * @brief Constructs a LaneSectionContactPointKey from a lane section key
     * and a contact point.
     *
     * @param laneSectionKey    The lane section key.
     * @param contactPoint      The contact point.
     */
    LaneSectionContactPointKey(LaneSectionKey laneSectionKey, ContactPoint contactPoint)
    {
        roadIdx_ = laneSectionKey.roadIdx_;
        laneSectionIdx_ = laneSectionKey.laneSectionIdx_;
        contactPoint_ = contactPoint;
    }

    /**
     * @brief Compares two LaneSectionContactPointKey instances for equality.
     */
    bool operator==(const LaneSectionContactPointKey& b) const
    {
        return roadIdx_ == b.roadIdx_ && laneSectionIdx_ == b.laneSectionIdx_ && contactPoint_ == b.contactPoint_;
    }

    /**
     * @brief Gets the key of this key's lane section.
     */
    LaneSectionKey laneSectionKey() const { return LaneSectionKey(roadIdx_, laneSectionIdx_); }

    /**
     * @brief Gets a human readable description of this LaneSectionContactPointKey.
     *
     * @param map           The XodrMap to which this LaneSectionContactPointKey applies.
     * @return              The human readable description.
     */
    std::string toString(const XodrMap& map) const;

    /**
     * @brief The index of the road which contains the lane section contact point.
     */

    int roadIdx_;

    /**
     * @brief The index of the lane section within its road, which contains the
     * lane section contact point.
     */
    int laneSectionIdx_;

    /**
     * @brief The type of the contact point.
     */
    ContactPoint contactPoint_;
};

/**
 * @brief A struct which contains the information needed to locate a lane within
 * an @ref XodrMap.
 */
struct LaneKey
{
    LaneKey() = default;

    /**
     * @brief Constructs a LaneKey from a road, lane section and lane index.
     *
     * @param roadIdx           The road index.
     * @param laneSectionIdx    The lane section index.
     * @param laneIdx           The lane index.
     */
    LaneKey(int roadIdx, int laneSectionIdx, int laneIdx)
        : roadIdx_(roadIdx), laneSectionIdx_(laneSectionIdx), laneIdx_(laneIdx)
    {
    }

    /**
     * @brief Constructs a LaneKey from a LaneSectionKey and a lane index.
     *
     * @param sectionKey        The lane section key.
     * @param laneIdx           The lane index.
     */
    LaneKey(LaneSectionKey sectionKey, int laneIdx)
        : roadIdx_(sectionKey.roadIdx_), laneSectionIdx_(sectionKey.laneSectionIdx_), laneIdx_(laneIdx)
    {
    }

    /**
     * @brief Compares two LaneKey instances for equality.
     */
    bool operator==(const LaneKey& b) const
    {
        return roadIdx_ == b.roadIdx_ && laneSectionIdx_ == b.laneSectionIdx_ && laneIdx_ == b.laneIdx_;
    }

    /**
     * @brief The index of the road which contains the lane.
     */
    int roadIdx_;

    /**
     * @brief The index, within the road, of the lane section which contains the lane.
     */
    int laneSectionIdx_;

    /**
     * @brief The index of the lane within the lane section.
     */
    int laneIdx_;
};

/**
 * @brief A struct which contains the information needed to locate a junction
 * connection within an @ref XodrMap.
 */
struct JunctionConnectionKey
{
    JunctionConnectionKey() = default;

    /**
     * @brief Constructs a JunctionConnectionKey from its components.
     *
     * @param junctionIdx       The junction index.
     * @param connectionIdx     The connection index.
     */
    JunctionConnectionKey(int junctionIdx, int connectionIdx)
    {
        junctionIdx_ = junctionIdx;
        connectionIdx_ = connectionIdx;
    }

    /**
     * @brief The index of the junction which contains the connection.
     */
    int junctionIdx_;

    /**
     * @brief The index of the connection within its junction.
     */
    int connectionIdx_;
};

/**
 * @brief A struct which contains the information needed to locate a junction
 * lane link within an @ref XodrMap.
 */
struct JunctionLaneLinkKey
{
    JunctionLaneLinkKey() = default;

    /**
     * @brief Constructs a JunctionLaneLinkKey from its components.
     *
     * @param junctionIdx       The junction index.
     * @param connectionIdx     The connection index.
     * @param laneLinkIdx       The lane link index.
     */
    JunctionLaneLinkKey(int junctionIdx, int connectionIdx, int laneLinkIdx)
    {
        junctionIdx_ = junctionIdx;
        connectionIdx_ = connectionIdx;
        laneLinkIdx_ = laneLinkIdx;
    }

    /**
     * @brief Constructs a JunctionLaneLinkKey from a connection key and a lane
     * link index.
     *
     * @param connectionKey     The connection key.
     * @param laneLinkIdx       The lane link index.
     */
    JunctionLaneLinkKey(JunctionConnectionKey connectionKey, int laneLinkIdx)
    {
        junctionIdx_ = connectionKey.junctionIdx_;
        connectionIdx_ = connectionKey.connectionIdx_;
        laneLinkIdx_ = laneLinkIdx;
    }

    /**
     * @brief The index of the junction which contains the lane link.
     */
    int junctionIdx_;

    /**
     * @brief The index of the connection, within its junction, which contains
     * the lane link.
     */
    int connectionIdx_;

    /**
     * @brief The index of the lane link within its connection.
     */
    int laneLinkIdx_;
};

}}  // namespace aid::xodr