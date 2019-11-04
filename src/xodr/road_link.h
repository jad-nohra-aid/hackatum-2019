#pragma once

#include "xodr_reader.h"
#include "xodr_object_reference.h"
#include "xml/xml_attribute_parsers.h"

namespace aid { namespace xodr {

enum class RoadLinkType
{
    PREDECESSOR,
    SUCCESSOR
};

/**
 * @brief Returns the name (in all lowercase) of the given roadLinkType.
 *
 * @param roadLinkType      The road link type.
 * @returns                 The name of the road link type.
 */
std::string roadLinkTypeName(RoadLinkType roadLinkType);

/**
 * @brief The contact point on the target element of this RoadLink.
 */
enum class ContactPoint
{
    /**
     * @brief A placeholder value which indicates that the contact point
     * isn't specified.
     *
     * RoadLink::contactPoint() will never actually return this value, it's
     * only used internally.
     */
    NOT_SPECIFIED,

    /**
     * @brief The contact point is at the start of the target element.
     */
    START,

    /**
     * @brief The contact point is at the end of the target element.
     */
    END,
};

/**
 * @brief Returns the name (in all lowercase) of the given contactPoint.
 *
 * Note that it's an error to call this function with ContactPoint::NOT_SPECIFIED.
 *
 * @param contactPoint      The contact point.
 * @returns                 The name of the contact point.
 */
std::string contactPointName(ContactPoint contactPoint);

/**
 * @brief Returns the type of link which originates from the given type of
 * contact point.
 *
 * @param contactPoint      The contact point.
 * @returs                  The road link type.
 */
RoadLinkType linkTypeForContactPoint(ContactPoint contactPoint);

/**
 * @brief Returns the contact point from which a link of the given link type originates.
 *
 * @param linkType          The link type.
 * @return                  The contact point.
 */
ContactPoint contactPointForLinkType(RoadLinkType linkType);

/**
 * @brief Returns the opposite contact point of the given contact point.
 *
 * @param contactPoint      The contact point.
 * @return                  The opposite contact point of @p contactPoint.
 */
ContactPoint oppositeContactPoint(ContactPoint contactPoint);

namespace xml_parsers {

template <>
ContactPoint parseXmlAttrib(const std::string& value);
}

/**
 * @brief A RoadLink contains the information needed to link a road to its
 * successor or predecessor road or junction.
 */
class RoadLink
{
  public:
    /**
     * @brief Constructs a RoadLink with its element type set to NOT_SPECIFIED.
     *
     * Use this as a placeholder value  to indicate that a road link isn't specified.
     */
    RoadLink();

    /**
     * @brief Parsers a RoadLink xodr element. Road link elements are the
     * predecessor and successor child elements of a road/link element.
     *
     * This function has xml_parsers::parseXmlElem() semantics.
     *
     * @param xml           An XodrReader. The current element must be either a
     *                      predecessor or successor start tag.
     * @returns             The resulting RoadLink.
     */
    static XodrParseResult<RoadLink> parseXml(XodrReader& xml);

    /**
     * @brief Creates a RoadLink of type ROAD, which links to the given road,
     * at the given contact point.
     *
     * @param roadRef       A reference to the target road.
     * @param contactPoint  The contact point on the target road.
     * @returns             The RoadLink.
     */
    static RoadLink roadLink(XodrObjectReference roadRef, ContactPoint contactPoint);

    /**
     * @brief Creates a RoadLink to the given junction.
     *
     * @param junctioNRef   A reference to the target junction.
     * @returns             The RoadLink.
     */
    static RoadLink junctionLink(XodrObjectReference junctionRef);

    /**
     * @brief The type of the target of this RoadLink.
     */
    enum class ElementType
    {
        /**
         * @brief No target element is specified.
         *
         * This value is used to indicate that a RoadLink doesn't link to
         * anything, and so none of the other functions should be used if
         * elementType() returns NOT_SPECIFIED.
         */
        NOT_SPECIFIED,

        /**
         * @brief The target of the RoadLink is another road.
         */
        ROAD,

        /**
         * @brief The target of the road link is a junction.
         */
        JUNCTION
    };

    /**
     * @brief Gets the target element type of this RoadLink.
     *
     * @returns             The ElementType.
     */
    ElementType elementType() const { return elementType_; }

    /**
     * @brief Gets the ContactPoint on the target road this RoadLink.
     *
     * This value is only valid when the target element is of type 'road'. It's
     * an error to call it in other cases.
     *
     * @returns             The contact point.
     */
    ContactPoint contactPoint() const;

    /**
     * @brief Gets the id of the target element.
     *
     * This function should only be called when elementType() returns ROAD or
     * JUNCTION. It's an error to call it when elementType() returns NOT_SPECIFIED.
     */
    const XodrObjectReference& elementRef() const;

    /**
     * @brief Resolves the XodrObjectReference references in this road link.
     *
     * See XodrObjectReference::resolve() for more details.
     *
     * @param idToIndexMaps   The mappings from identifiers to indices.
     */
    void resolveReferences(const IdToIndexMaps& idToIndexMaps);

  private:
    class AttribParsers;

    ElementType elementType_;
    ContactPoint contactPoint_;
    XodrObjectReference elementRef_;
};

/**
 * @brief A NeighborLink contains the information needed to link a road to a
 * neighboring road.
 */
class NeighborLink
{
  public:
    /**
     * @brief Constructs a NeighborLink whose 'isSpecified' flag is set to false.
     *
     * Use this as a placeholder to indicate that a neighbor link is missing.
     */
    NeighborLink() = default;

    /**
     * @brief Parsers a NeighborLink xodr element. NeighborLink elements are the
     * <neighbor/> child elements of a road/link element.
     *
     * This function has xml_parsers::parseXmlElem() semantics.
     *
     * @param xml           An XodrReader.
     * @returns             The resulting NeighborLink.
     */
    static XodrParseResult<NeighborLink> parseXml(XodrReader& xml);

    /**
     * @brief The side the neighboring road is on.
     */
    enum class Side
    {
        /**
         * @brief The left side.
         */
        LEFT,

        /**
         * @brief The right side.
         */
        RIGHT,
    };

    /**
     * @brief Whether the two neighboring road have the same or opposing directions.
     */
    enum class Direction
    {
        /**
         * @brief The two neighboring roads have the same direction.
         */
        SAME,

        /**
         * @brief The two neighboring roads have opposing directions.
         */
        OPPOSITE
    };

    /**
     * @brief Returns whether this NeighborLink's 'isSpecified' flag is set.
     */
    bool isSpecified() const { return isSpecified_; }

    /**
     * @brief Gets the side the neighboring road is on.
     *
     * @pre `isSpecified() == true`
     */
    Side side() const;

    /**
     * @brief Gets the direction of the neighboring road, relative to the road
     * which owns this NeighborLink.
     *
     * @pre `isSpecified() == true`
     */
    Direction direction() const;

    /**
     * @brief Gets the reference to the neighboring road.
     */
    const XodrObjectReference& elementRef() const;

    /**
     * @brief Resolves the XodrObjectReference references in this NeighborLink.
     *
     * See XodrObjectReference::resolve() for more details.
     *
     * @param idToIndexMaps   The mappings from identifiers to indices.
     */
    void resolveReferences(const IdToIndexMaps& idToIndexMaps);

  private:
    class AttribParsers;

    bool isSpecified_ = false;
    Side side_;
    Direction direction_;
    XodrObjectReference elementRef_;
};

/**
 * A container for the successor and predecessor RoadLink's, as well as the
 * neighboring road links of a road.
 *
 * The main use of this class is that it provides the functionality to parse the
 * <link> child element of a <road> element in an xodr file.
 */
class RoadLinks
{
  public:
    /**
     * Creates a RoadLinks instance with all its members set to 'not specified'.
     */
    RoadLinks() = default;

    /**
     * @brief Parses a RoadLinks from a <link> xodr element.
     *
     * If any if the elements are missing the xml, then the relevant RoadLink
     * or NeighborLink will be set to 'not specified'.
     *
     * This function has @ref xml_parsers::parseXmlElem semantics.
     *
     * @param xml           The XodrReader.
     * @returns             The resulting RoadLinks.
     */
    static XodrParseResult<RoadLinks> parseXml(XodrReader& xml);

    /**
     * @brief Gets the the predecessor RoadLink of this RoadLinks instance.
     */
    const RoadLink& predecessor() const { return predecessor_; }

    /**
     * @brief Gets the successor RoadLink of this RoadLinks instance.
     */
    const RoadLink& successor() const { return successor_; }

    /**
     * @brief Gets the left neighbor of this RoadLinks instance.
     */
    const NeighborLink& leftNeighbor() const { return leftNeighbor_; }

    /**
     * @brief Gets the right neighbor of this RoadLinks instance.
     */
    const NeighborLink& rightNeighbor() const { return rightNeighbor_; }

    /**
     * @brief Resolves the XodrObjectReference references in this RoadLinks instance.
     *
     * See XodrObjectReference::resolve() for more details.
     *
     * @param idToIndexMaps   The mappings from identifiers to indices.
     */
    void resolveReferences(const IdToIndexMaps& idToIndexMaps);

  public:
    /**
     * @brief Sets the predecessor of this RoadLinks.
     *
     * This function should only be used from unit tests.
     *
     * @param predecessor   The predecessor RoadLink.
     */
    void test_setPredecessor(const RoadLink& predecessor) { predecessor_ = predecessor; }

    /**
     * @brief Sets the successor of this RoadLinks.
     *
     * This function should only be used from unit tests.
     *
     * @param successor     The successor RoadLink.
     */
    void test_setSuccessor(const RoadLink& successor) { successor_ = successor; }

  private:
    class ChildElemParsers;

    RoadLink predecessor_;
    RoadLink successor_;
    NeighborLink leftNeighbor_;
    NeighborLink rightNeighbor_;
};

}}  // namespace aid::xodr
