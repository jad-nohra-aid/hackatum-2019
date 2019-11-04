#pragma once

namespace aid { namespace xodr {

class XodrMap;

/**
 * @brief Validates junction membership of roads.
 *
 * Specifically, it validates that a road belongs to a certain junction if and
 * only if there's a connection in that junction which uses that road as its
 * connecting road.
 *
 * An exception is thrown if validation doesn't pass.
 *
 * @param map               The XodrMap to validate.
 */
void validateJunctionMembership(const XodrMap& map);

}}  // namespace aid::xodr