#include "junction_validation.h"

#include <sstream>

#include "xodr_map.h"

namespace aid { namespace xodr {

static bool junctionContainsRoad(const Junction& junction, int roadIdx);

void validateJunctionMembership(const XodrMap& map)
{
    const auto& roads = map.roads();
    const auto& junctions = map.junctions();

    for (int i = 0; i < static_cast<int>(roads.size()); i++)
    {
        const Road& road = roads[i];
        if (road.junctionRef().hasValue())
        {
            const Junction& junction = junctions[road.junctionRef().index()];
            if (!junctionContainsRoad(junction, i))
            {
                std::stringstream err;
                err << "The road " << road.id() << " is part of junction " << junction.id()
                    << ", but this junction doesn't contain a connection with road " << road.id()
                    << " as connecting road.";
                throw std::runtime_error(err.str());
            }
        }
    }

    for (int i = 0; i < static_cast<int>(junctions.size()); i++)
    {
        const Junction& junction = junctions[i];
        for (const Junction::Connection& conn : junction.connections())
        {
            const Road& connectingRoad = roads[conn.connectingRoad().index()];
            if (connectingRoad.junctionRef().index() != i)
            {
                std::stringstream err;
                err << "Junction " << junction.id() << " uses " << connectingRoad.id()
                    << " as a connecting road, but this road doesn't belong to junction " << junction.id() << ".";
                throw std::runtime_error(err.str());
            }
        }
    }
}

static bool junctionContainsRoad(const Junction& junction, int roadIdx)
{
    for (const Junction::Connection& conn : junction.connections())
    {
        if (conn.connectingRoad().index() == roadIdx)
        {
            return true;
        }
    }

    return false;
}

}}  // namespace aid::xodr
