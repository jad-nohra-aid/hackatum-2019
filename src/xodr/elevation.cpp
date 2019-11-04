#include "elevation.h"
#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

class ElevationProfile::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<ElevationProfile>>
{
  public:
    ChildElemParsers()
    {
        addVectorElementParser<XodrParseResult<ElevationProfile::Elevation>>(
            "elevation", &ElevationProfile::elevations_, Multiplicity::ONE_OR_MORE);
        finalize();
    }
};

XodrParseResult<ElevationProfile> ElevationProfile::parseXml(XodrReader& xml)
{
    XodrParseResult<ElevationProfile> ret;

    static ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

ElevationProfile::Elevation::Elevation(double sCoord, const Poly3& poly3) : sCoord_(sCoord), poly3_(poly3) {}

class ElevationProfile::Elevation::AttribParsers
    : public XmlAttributeParsers<XodrParseResult<ElevationProfile::Elevation>>
{
  public:
    AttribParsers()
    {
        addFieldParser("s", &Elevation::sCoord_);
        addSetterParser("a", &Elevation::setA);
        addSetterParser("b", &Elevation::setB);
        addSetterParser("c", &Elevation::setC);
        addSetterParser("d", &Elevation::setD);

        finalize();
    }
};

XodrParseResult<ElevationProfile::Elevation> ElevationProfile::Elevation::parseXml(XodrReader& xml)
{
    XodrParseResult<ElevationProfile::Elevation> ret;

    static AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

}}  // namespace aid::xodr
