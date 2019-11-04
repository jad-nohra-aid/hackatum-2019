#include "road_object_outline.h"

#include "xodr_reader.h"
#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

class RoadObjectOutline::ChildElemParsers
    : public XmlChildElementParsers<XodrReader, XodrParseResult<RoadObjectOutline>>
{
  public:
    ChildElemParsers()
    {
        addParser("cornerRoad", Multiplicity::ZERO_OR_MORE,
                  [](XodrReader& xml, XodrParseResult<RoadObjectOutline>& result) {
                      XodrParseResult<CornerRoad> res = CornerRoad::parseXml(xml);
                      result.value().corners_.push_back(res.value());
                      result.appendErrors(res);
                  });

        addParser("cornerLocal", Multiplicity::ZERO_OR_MORE,
                  [](XodrReader& xml, XodrParseResult<RoadObjectOutline>& result) {
                      XodrParseResult<CornerLocal> res = CornerLocal::parseXml(xml);
                      result.value().corners_.push_back(res.value());
                      result.appendErrors(res);
                  });

        finalize();
    }
};

XodrParseResult<RoadObjectOutline> RoadObjectOutline::parseXml(XodrReader& xml)
{
    XodrParseResult<RoadObjectOutline> ret;

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

class RoadObjectOutline::CornerRoad::AttribParsers
    : public XmlAttributeParsers<XodrParseResult<RoadObjectOutline::CornerRoad>>
{
  public:
    AttribParsers()
    {
        addFieldParser("s", &CornerRoad::s_);
        addFieldParser("t", &CornerRoad::t_);
        addFieldParser("dz", &CornerRoad::dz_);
        addFieldParser("height", &CornerRoad::height_);

        finalize();
    }
};

XodrParseResult<RoadObjectOutline::CornerRoad> RoadObjectOutline::CornerRoad::parseXml(XodrReader& xml)
{
    XodrParseResult<CornerRoad> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class RoadObjectOutline::CornerLocal::AttribParsers
    : public XmlAttributeParsers<XodrParseResult<RoadObjectOutline::CornerLocal>>
{
  public:
    AttribParsers()
    {
        addFieldParser("u", &CornerLocal::u_);
        addFieldParser("v", &CornerLocal::v_);
        addFieldParser("z", &CornerLocal::z_);
        addFieldParser("height", &CornerLocal::height_);

        finalize();
    }
};

XodrParseResult<RoadObjectOutline::CornerLocal> RoadObjectOutline::CornerLocal::parseXml(XodrReader& xml)
{
    XodrParseResult<CornerLocal> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

}}  // namespace aid::xodr