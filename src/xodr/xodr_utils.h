#pragma once

#include "xodr_map.h"
#include "xodr_map_keys.h"

namespace aid { namespace xodr {

/**
 * @brief Loops over all (unordered) pairs of connected roads in the given XodrMap.
 *
 * Roads are connected if they are each other's successor/predecessor, or if
 * they are connected by a junction connection.
 *
 * The callback function must be of the type
 *
 *   void f(RoadContactPointKey aKey, RoadContactPointKey bKey);
 *
 * @param map               The XodrMap.
 * @param f                 The function object which is called for each road link.
 */
template <class F>
void forEachRoadLink(const XodrMap& map, F&& f);

/**
 * @brief Loops over all (unordered) pairs of connected roads in the given XodrMap.
 *
 * Roads are connected if they are each other's successor/predecessor, or if
 * they are connected by a junction connection.
 *
 * This overload takes two callbacks. The first one, roadRoadF, is called for
 * normal road links. This function must have the following type:
 *
 *   void f(RoadContactPointKey aKey, RoadContactPointKey bKey);
 *
 * The second one, junctionRoadF, is called for roads which are connected in a
 * junction. This function must have the following type:
 *
 *   void f(RoadContactPointKey aKey, RoadContactPointKey bKey,
 *       const Junction::Connection& connection);
 *
 * @param map               The XodrMap.
 * @param roadRoadF         The callback which is called for each normal road link.
 * @param junctionRoadF     The callback which is called for each junction road link.
 */
template <class RoadRoadF, class JunctionRoadF>
void forEachRoadLink(const XodrMap& map, RoadRoadF&& roadRoadF, JunctionRoadF&& junctionRoadF);

}}  // namespace aid::xodr

#include "xodr_utils_impl.h"
