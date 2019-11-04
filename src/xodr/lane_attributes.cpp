#include "lane_attributes.h"
#include "xml/xml_attribute_parsers.h"
#include "units.h"

namespace aid { namespace xodr {

class LaneMaterial::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneMaterial>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneMaterial::sOffset_);
        addFieldParser("surface", &LaneMaterial::surface_);
        addFieldParser("friction", &LaneMaterial::friction_);
        addFieldParser("roughness", &LaneMaterial::roughness_);
        finalize();
    }
};

XodrParseResult<LaneMaterial> LaneMaterial::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneMaterial> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class LaneVisibility::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneVisibility>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneVisibility::sOffset_);
        addFieldParser("forward", &LaneVisibility::forward_);
        addFieldParser("back", &LaneVisibility::back_);
        addFieldParser("left", &LaneVisibility::left_);
        addFieldParser("right", &LaneVisibility::right_);
        finalize();
    }
};

XodrParseResult<LaneVisibility> LaneVisibility::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneVisibility> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class LaneSpeedLimit::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneSpeedLimit>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneSpeedLimit::sOffset_);
        addFieldParser("max", &LaneSpeedLimit::maxSpeed_);
        addOptionalFieldParser("unit", &LaneSpeedLimit::unit_, SpeedUnit::NOT_SPECIFIED);
        finalize();
    }
};

XodrParseResult<LaneSpeedLimit> LaneSpeedLimit::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneSpeedLimit> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class LaneAccess::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneAccess>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneAccess::sOffset_);
        addFieldParser("restriction", &LaneAccess::restriction_);
        finalize();
    }
};

XodrParseResult<LaneAccess> LaneAccess::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneAccess> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class LaneHeight::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneHeight>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneHeight::sOffset_);
        addFieldParser("inner", &LaneHeight::inner_);
        addFieldParser("outer", &LaneHeight::outer_);
        finalize();
    }
};

XodrParseResult<LaneHeight> LaneHeight::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneHeight> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

class LaneRule::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneRule>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &LaneRule::sOffset_);
        addFieldParser("value", &LaneRule::value_);
        finalize();
    }
};

XodrParseResult<LaneRule> LaneRule::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneRule> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

}}  // namespace aid::xodr