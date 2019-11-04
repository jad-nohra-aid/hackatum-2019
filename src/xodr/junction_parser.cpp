#include "junction.h"

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

class Junction::AttribParsers : public XmlAttributeParsers<XodrParseResult<Junction>>
{
  public:
    AttribParsers()
    {
        addFieldParser("name", &Junction::name_);
        addFieldParser("id", &Junction::id_, XodrInvalidations::ALL);
        finalize();
    }
};

class Junction::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Junction>>
{
  public:
    ChildElemParsers()
    {
        addVectorElementParser<XodrParseResult<Junction::Connection>>("connection", &Junction::connections_,
                                                                      Multiplicity::ONE_OR_MORE);
        finalize();
    }
};

XodrParseResult<Junction> Junction::parseXml(XodrReader& xml)
{
    XodrParseResult<Junction> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

void Junction::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    for (Connection& connection : connections_)
    {
        connection.resolveReferences(idToIndexMaps);
    }
}

class Junction::Connection::AttribParsers : public XmlAttributeParsers<XodrParseResult<Connection>>
{
  public:
    AttribParsers()
    {
        addFieldParser("id", &Connection::id_, XodrInvalidations::ALL);
        addFieldParser("incomingRoad", &Connection::incomingRoad_, XodrInvalidations::CONNECTIVITY);
        addFieldParser("connectingRoad", &Connection::connectingRoad_, XodrInvalidations::CONNECTIVITY);
        addFieldParser("contactPoint", &Connection::contactPoint_, XodrInvalidations::CONNECTIVITY);
        finalize();
    }
};

class Junction::Connection::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Connection>>
{
  public:
    ChildElemParsers()
    {
        addVectorElementParser<XodrParseResult<Junction::LaneLink>>("laneLink", &Connection::laneLinks_,
                                                                    Multiplicity::ZERO_OR_MORE);
        finalize();
    }
};

XodrParseResult<Junction::Connection> Junction::Connection::parseXml(XodrReader& xml)
{
    XodrParseResult<Connection> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

void Junction::Connection::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    incomingRoad_.resolve(idToIndexMaps.roadIdToIndex_, "road");
    connectingRoad_.resolve(idToIndexMaps.roadIdToIndex_, "road");
}

class Junction::LaneLink::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneLink>>
{
  public:
    AttribParsers()
    {
        addFieldParser("from", &LaneLink::from_, XodrInvalidations::CONNECTIVITY);
        addFieldParser("to", &LaneLink::to_, XodrInvalidations::CONNECTIVITY);
        finalize();
    }
};

XodrParseResult<Junction::LaneLink> Junction::LaneLink::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneLink> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

}}  // namespace aid::xodr
