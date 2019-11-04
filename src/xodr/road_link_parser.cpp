#include "road_link.h"

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

namespace xml_parsers {

template <>
RoadLink::ElementType parseXmlAttrib(const std::string& value)
{
    if (value == "road")
    {
        return RoadLink::ElementType::ROAD;
    }
    else if (value == "junction")
    {
        return RoadLink::ElementType ::JUNCTION;
    }
    else
    {
        throw std::invalid_argument("Invalid element type.");
    }
}

template <>
ContactPoint parseXmlAttrib(const std::string& value)
{
    if (value == "start")
    {
        return ContactPoint::START;
    }
    else if (value == "end")
    {
        return ContactPoint::END;
    }
    else
    {
        throw std::invalid_argument("Invalid contact point.");
    }
}
}  // namespace xml_parsers

class RoadLink::AttribParsers : public XmlAttributeParsers<XodrParseResult<RoadLink>>
{
  public:
    AttribParsers()
    {
        addFieldParser("elementType", &RoadLink::elementType_);
        addFieldParser("elementId", &RoadLink::elementRef_);
        addOptionalFieldParser("contactPoint", &RoadLink::contactPoint_, ContactPoint::NOT_SPECIFIED);
        finalize();
    }
};

XodrParseResult<RoadLink> RoadLink::parseXml(XodrReader& xml)
{
    XodrParseResult<RoadLink> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    if (ret.value().elementType_ == ElementType::ROAD && ret.value().contactPoint_ == ContactPoint::NOT_SPECIFIED)
    {
        throw std::runtime_error("The contactPoint attribute is required when elementType is 'road'.");
    }

    return ret;
}

void RoadLink::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    switch (elementType_)
    {
        default:
            assert(!"Invalid elementType_");

        case ElementType::NOT_SPECIFIED:
            break;

        case ElementType::ROAD:
            elementRef_.resolve(idToIndexMaps.roadIdToIndex_, "road");
            break;

        case ElementType::JUNCTION:
            elementRef_.resolve(idToIndexMaps.junctionIdToIndex_, "junction");
            break;
    }
}

namespace xml_parsers {

template <>
NeighborLink::Side parseXmlAttrib(const std::string& value)
{
    if (value == "left")
    {
        return NeighborLink::Side::LEFT;
    }
    else if (value == "right")
    {
        return NeighborLink::Side::RIGHT;
    }
    else
    {
        throw std::invalid_argument("Invalid side.");
    }
}

template <>
NeighborLink::Direction parseXmlAttrib(const std::string& value)
{
    if (value == "same")
    {
        return NeighborLink::Direction::SAME;
    }
    else if (value == "opposite")
    {
        return NeighborLink::Direction::OPPOSITE;
    }
    else
    {
        throw std::invalid_argument("Invalid direction.");
    }
}
}  // namespace xml_parsers

class NeighborLink::AttribParsers : public XmlAttributeParsers<XodrParseResult<NeighborLink>>
{
  public:
    AttribParsers()
    {
        addFieldParser("side", &NeighborLink::side_);
        addFieldParser("elementId", &NeighborLink::elementRef_);
        addFieldParser("direction", &NeighborLink::direction_);
        finalize();
    }
};

XodrParseResult<NeighborLink> NeighborLink::parseXml(XodrReader& xml)
{
    XodrParseResult<NeighborLink> ret;
    ret.value().isSpecified_ = true;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

void NeighborLink::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    if (isSpecified_)
    {
        elementRef_.resolve(idToIndexMaps.roadIdToIndex_, "road");
    }
}

class RoadLinks::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<RoadLinks>>
{
  public:
    ChildElemParsers()
    {
        addOptionalFieldParser<XodrParseResult<RoadLink>>("predecessor", &RoadLinks::predecessor_, RoadLink(),
                                                          XodrInvalidations::CONNECTIVITY);
        addOptionalFieldParser<XodrParseResult<RoadLink>>("successor", &RoadLinks::successor_, RoadLink(),
                                                          XodrInvalidations::CONNECTIVITY);
        addParser("neighbor", Multiplicity::ZERO_OR_MORE,
                  [](XodrReader& xml, XodrParseResult<RoadLinks>& pair) {
                      XodrParseResult<NeighborLink> link = NeighborLink::parseXml(xml);
                      if (link.errors().empty())
                      {
                          switch (link.value().side())
                          {
                              case NeighborLink::Side::LEFT:
                                  if (pair.value().leftNeighbor_.isSpecified())
                                  {
                                      pair.errors().emplace_back("At most a single left neighbor may be specified.",
                                                                 XodrInvalidations::CONNECTIVITY);
                                  }
                                  else
                                  {
                                      pair.value().leftNeighbor_ = link.value();
                                  }
                                  break;

                              case NeighborLink::Side::RIGHT:
                                  if (pair.value().rightNeighbor_.isSpecified())
                                  {
                                      pair.errors().emplace_back("At most a single right neighbor may be specified.",
                                                                 XodrInvalidations::CONNECTIVITY);
                                  }
                                  else
                                  {
                                      pair.value().rightNeighbor_ = link.value();
                                  }
                                  break;
                          }
                      }
                      pair.appendErrors(link);
                  },
                  XodrInvalidations::CONNECTIVITY);

        finalize();
    }
};

XodrParseResult<RoadLinks> RoadLinks::parseXml(XodrReader& xml)
{
    XodrParseResult<RoadLinks> ret;

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

void RoadLinks::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    predecessor_.resolveReferences(idToIndexMaps);
    successor_.resolveReferences(idToIndexMaps);
    leftNeighbor_.resolveReferences(idToIndexMaps);
    rightNeighbor_.resolveReferences(idToIndexMaps);
}

}}  // namespace aid::xodr
