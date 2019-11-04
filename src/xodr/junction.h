#pragma once

#include "xodr_reader.h"
#include "xodr_object_reference.h"
#include "road_link.h"
#include "lane_id.h"

namespace aid { namespace xodr {

/**
 * @brief A Junction describes the part of a roadmap where roads branch off into
 * more than one predecessor or successor road.
 */
class Junction
{
  public:
    class Connection;
    class LaneLink;

    /**
     * @brief Creates an empty junction.
     */
    Junction() = default;

    /**
     * @brief Parsers a Junction using the given XodrReader.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml       The XodrReader.
     * @returns         The resulting Junction.
     */
    static XodrParseResult<Junction> parseXml(XodrReader& xml);

    /**
     * @brief A connection within a junction.
     */
    class Connection
    {
      public:
        /**
         * @brief Parses a Connection using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting Connection.
         */
        static XodrParseResult<Connection> parseXml(XodrReader& xml);

        /**
         * @brief Gets the id of this connection within the junction.
         */
        const std::string& id() const { return id_; }

        /**
         * @brief Gets the id of the incoming road.
         */
        const XodrObjectReference& incomingRoad() const { return incomingRoad_; }

        /**
         * @brief Gets the id of the connecting road.
         */
        const XodrObjectReference& connectingRoad() const { return connectingRoad_; }

        /**
         * @brief Gets the contact point on the connecting road.
         */
        ContactPoint contactPoint() const { return contactPoint_; }

        /**
         * @brief Gets the lane specific linking information.
         */
        const std::vector<LaneLink>& laneLinks() const { return laneLinks_; }

        /**
         * @brief Searches this connection for a lane link whose 'from' lane
         * equals the given 'fromLane', and returns the 'to' lane of that link.
         *
         * If no link with the given from lane is found, LaneID::null() is returned.
         *
         * @param fromLane  The lane id of the from lane.
         * @return          The lane id of the target lane, or LaneID::null() if
         *                  no link from the given lane is found.
         */
        LaneIDOpt findLaneLinkTarget(LaneID fromLane) const;

        /**
         * @brief Resolves the XodrObjectReference references in this connection.
         *
         * See XodrObjectReference::resolve() for more details.
         *
         * @param idToIndexMaps   The mappings from identifiers to indices.
         */
        void resolveReferences(const IdToIndexMaps& idToIndexMaps);

        /**
         * @brief Sets the target of the given 'from' lane to the given 'to' lane.
         *
         * If a link whose 'from' lane id equals 'fromLaneId' already exists in
         * this connection, then its 'to' lane id will be replaced with the
         * given 'to' lane id, or if the new 'to' id is LaneIDOpt::null(), then
         * the whole link will be removed.
         *
         * If no link with the given 'from' lane id exists yet, then a new link
         * will be added (at least if toLaneId != LaneIDOpt::null()).
         *
         * @param fromLaneId  The 'from' lane id of the lane link.
         * @param toLaneId    The 'to' lane id of the lane link.
         */
        void test_setLaneLinkTarget(LaneID fromLaneId, LaneIDOpt toLaneId);

      private:
        class AttribParsers;
        class ChildElemParsers;

        std::string id_;
        XodrObjectReference incomingRoad_;
        XodrObjectReference connectingRoad_;
        ContactPoint contactPoint_;
        std::vector<LaneLink> laneLinks_;
    };

    /**
     * A lane link specifies how the individual lanes of two adjacent roads are connected.
     */
    class LaneLink
    {
      public:
        LaneLink() = default;
        LaneLink(LaneID from, LaneID to) : from_(from), to_(to) {}

        /**
         * @brief Parses a LaneLink using the given XodrReader.
         *
         * This function has @ref xml_parsers::parseXmlElem semantics.
         *
         * @param xml       The XodrReader.
         * @returns         The resulting LaneLink.
         */
        static XodrParseResult<LaneLink> parseXml(XodrReader& xml);

        /**
         * @brief The id of the 'from' lane of this LaneLink.
         *
         * The 'from' lane is part of the incoming road of the Connection.
         */
        LaneID from() const { return from_; }

        /**
         * @brief The id of the 'to' lane of this LaneLink.
         *
         * The 'to' lane is part of the connecting road of the Connection.
         */
        LaneID to() const { return to_; }

      public:
        void test_setTo(LaneID to) { to_ = to; }

      private:
        class AttribParsers;

        LaneID from_;
        LaneID to_;
    };

    /**
     * @brief Gets the name of this junction.
     */
    const std::string& name() const { return name_; }

    /**
     * @brief Gets the id of this junction.
     */
    const std::string& id() const { return id_; }

    /**
     * @brief Gets the connections of this junction.
     */
    const std::vector<Connection>& connections() const { return connections_; }

    /**
     * @brief Resolves the XodrObjectReference references in this junction.
     *
     * See XodrObjectReference::resolve() for more details.
     *
     * @param idToIndexMaps   The mappings from identifiers to indices.
     */
    void resolveReferences(const IdToIndexMaps& idToIndexMaps);

    /**
     * @brief Returns whether this junction contains a connection which connects
     * the given incoming road to the given connecting road, at the given
     * contact point on the connecting road.
     *
     * @param incomingRoadIdx The index of the incoming road.
     * @param connectingRoadIdx The index of the outgoing road.
     * @param contactPoint    The contact point.
     * @returns               True if this junction contains a connection with
     *                        the given values, false otherwise.
     */
    bool hasConnection(int incomingRoadIdx, int connectingRoadIdx, ContactPoint contactPoint) const;

    /**
     * @brief Returns the connection with the given incoming road, connecting
     * road and contact point, or nullptr if no such connection exists in this junction.
     *
     * @param incomingRoadIdx   The index of the incoming road.
     * @param connectingRoadIDx The index of the connecting road.
     * @param contactPoint      The contact point.
     * @returns                 A pointer to the connection, or nullptr if no
     *                          such connection exists in this Junction.
     */
    const Connection* findConnection(int incomingRoadIdx, int connectingRoadIdx, ContactPoint contactPoint) const;

    /**
     * @brief Returns whether this junction has a connection with the given
     * connecting road, and whose outgoing contact point is the given contact point.
     *
     * @param connectingRoadIdx The index of the connecting road.
     * @param contactPoint      The outgoing contact point.
     * @return True if this junction contains such a connection, false otherwise.
     */
    bool hasOutgoingConnection(int connectingRoadIdx, ContactPoint contactPoint) const;

  public:
    Connection* test_connectionById(const std::string& id);

  private:
    class AttribParsers;
    class ChildElemParsers;

    std::string name_;
    std::string id_;
    std::vector<Connection> connections_;
};

}}  // namespace aid::xodr
