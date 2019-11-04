#pragma once

#include <boost/optional.hpp>

#include "xodr_reader.h"
#include "road.h"
#include "junction.h"

namespace aid { namespace xodr {

class LaneSection;

/**
 * @brief The root object of an xodr road map.
 */
class XodrMap
{
  public:
    XodrMap() = default;
    XodrMap(const XodrMap&) = delete;
    XodrMap& operator=(const XodrMap&) = delete;

    XodrMap(XodrMap&&) = default;
    XodrMap& operator=(XodrMap&&) = default;
    /**
     * @brief Loads an XodrMap from the given xodr file.
     *
     * @param fileName      The name of the xodr file.
     * @returns             The XodrMap.
     */
    static XodrParseResult<XodrMap> fromFile(const std::string& fileName);

    /**
     * @brief Loads an XodrMap from the given xodr text.
     *
     * @param text          The xodr text.
     * @returns             The XodrMap.
     */
    static XodrParseResult<XodrMap> fromText(const std::string& text);

    /**
     * @brief Parses an XodrMap from an <OpenDRIVE> xodr element using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml           The XodrReader.
     * @returns             The resulting XodrMap.
     */
    static XodrParseResult<XodrMap> parseXml(XodrReader& xml);

    /**
     * @brief Gets whether this XodrMap has a geo-reference.
     *
     * If this is the case, then the geoReference() can be used to get the
     * geo-reference string.
     *
     * @return True if this XodrMap has a geo-reference, false otherwise.
     */
    bool hasGeoReference() const;

    /**
     * @brief The geo-reference string used by this XodrMap.
     *
     * This function should only be called if hasGeoReference() returns true.
     *
     * The geo-reference describes the projection from geographic coordinates to
     * map coordinates, using a proj.4 string.
     *
     * @return The geo-reference string.
     */
    const std::string& geoReference() const;

    /**
     * @brief Gets all roads in this XodrMap.
     *
     * @returns             A const reference to the vector containing the roads.
     */
    const std::vector<Road>& roads() const { return roads_; }

    /**
     * @brief Gets the road with the given road id, or nullptr if no road with
     * that id exists.
     *
     * @param id            The id of the road.
     * @returns             A pointer to the road, or nullptr if no road with
     *                      the given id exists. The XodrMap will retain
     *                      ownership of the road.
     */
    const Road* roadById(const std::string& id) const;

    /**
     * @brief Gets the road index of the road with the given id, or -1 if no
     * road with that id exists.
     *
     * @param id            The id of the road.
     * @returns             The index of the road with the given id, or -1 if no
     *                      road with that id exists.
     */
    int roadIndexById(const std::string& id) const;

    /**
     * @brief Gets all the junctions in this XodrMap.
     *
     * @returns             A const reference to the vector containing the junctions.
     */
    const std::vector<Junction>& junctions() const { return junctions_; }

    /**
     * @brief Gets the junction with the given junction id, or nullptr if no
     * junction with that id exists.
     *
     * @param id            The id of the junction.
     * @returns             A pointer to the junction, or nullptr if no junction
     *                      with the given id exists. The XodrMap will retain
     *                      ownership of the road object.
     */
    const Junction* junctionById(const std::string& id) const;

    /**
     * @brief Gets the junction index of the junction with the given id, or -1
     * if no junction with that id exists.
     *
     * @param id            The id of the junction.
     * @returns             The index of the junction with the given id, or -1
     *                      if no junction with that id exists. The XodrMap will
     *                      retain ownership of the junction object.
     */
    int junctionIndexById(const std::string& id) const;

    /**
     * @brief Gets the total number of lanes in this XodrMap.
     *
     * Use this as the size of array which associates an element which each lane
     * and can be indexed using @ref LaneSection::Lane::globalIndex().
     *
     * @returns             The total number of lanes.
     */
    int totalNumLanes() const { return totalNumLanes_; }

    /**
     * @brief Returns whether this XodrMap has any road objects.
     *
     * The road objects are children of roads, so this function iterates over
     * all roads, and returns true if there's at least one road with at least
     * one road objects.
     *
     * @return True if there's at least one road with at least one road object,
     * false otherwise.
     */
    bool hasRoadObjects() const;

    /**
     * @brief Validates this XodrMap.
     *
     * This function calls all the validation functions.
     *
     * An exception is thrown if validation doesn't pass.
     */
    void validate() const;

  public:
    /**
     * @brief Gets the road with the given road id, or nullptr if no road with
     * that id exists.
     *
     * This function is similar to roadById, but returns a writable pointer.
     *
     * This function should only be used from unit tests.
     *
     * @param id            The id of the road.
     * @returns             A pointer to the road, or nullptr if no road with
     *                      the given id exists. The XodrMap will retain
     *                      ownership of the road object.
     */
    Road* test_roadById(const std::string& id);

    /**
     * @brief Gets the junction with the given junction id, or nullptr if no
     * junction with that id exists.
     *
     * This function is similar to junctionById, but returns a writable pointer.
     *
     * This function should only be used from unit tests.
     *
     * @param id            The id of the junction.
     * @returns             A pointer to the junction, or nullptr if no junction
     *                      with the given id exists. The XodrMap will retain
     *                      ownership of the junction object.
     */
    Junction* test_junctionById(const std::string& id);

  private:
    void resolveReferences(std::vector<XodrParseError>& errors);

    class HeaderChildElemParsers;
    class ChildElemParsers;

    boost::optional<std::string> geoReference_;

    std::vector<Road> roads_;
    std::vector<Junction> junctions_;

    IdToIndexMaps idToIndexMaps_;

    int totalNumLanes_;
};

}}  // namespace aid::xodr
