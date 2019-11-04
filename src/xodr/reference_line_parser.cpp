#include "reference_line.h"

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

namespace xml_parsers {

template <>
ReferenceLine::PRange parseXmlAttrib<ReferenceLine::PRange>(const std::string& value)
{
    if (value == "arcLength")
    {
        return ReferenceLine::PRange::ARC_LENGTH;
    }
    else if (value == "normalized")
    {
        return ReferenceLine::PRange::NORMALIZED;
    }
    else
    {
        throw std::invalid_argument(value);
    }
}
}  // namespace xml_parsers

XodrParseResult<ReferenceLine> ReferenceLine::fromText(const std::string& text)
{
    XodrReader xml = XodrReader::fromText(text);
    xml.readStartElement("planView");
    return ReferenceLine::parseXml(xml);
}

class ReferenceLine::GeometryAttribs::AttribParsers : public XmlAttributeParsers<XodrParseResult<GeometryAttribs>>
{
  public:
    AttribParsers()
    {
        addSetterParser<double>("s", &GeometryAttribs::setS, XodrInvalidations::GEOMETRY);
        addSetterParser<double>("x", &GeometryAttribs::setX, XodrInvalidations::GEOMETRY);
        addSetterParser<double>("y", &GeometryAttribs::setY, XodrInvalidations::GEOMETRY);
        addSetterParser<double>("hdg", &GeometryAttribs::setHeading, XodrInvalidations::GEOMETRY);
        addFieldParser<double>("length", &GeometryAttribs::length_, XodrInvalidations::GEOMETRY);
        finalize();
    }
};

XodrParseResult<ReferenceLine> ReferenceLine::parseXml(XodrReader& xml)
{
    XodrParseResult<ReferenceLine> ret;

    XmlChildElementParsers<XodrReader, XodrParseResult<ReferenceLine>>::parseOneOrMore(
        xml, ret, "geometry",
        [](XodrReader& xml, XodrParseResult<ReferenceLine>& refLine) {
            XodrParseResult<GeometryAttribs> geomAttribs;

            static GeometryAttribs::AttribParsers geomAttribParser;

            geomAttribParser.parse(xml, geomAttribs);

            if (geomAttribs.hasValidGeometry())
            {
                // no point checking this length_ parameter if it could have a parse error
                if (geomAttribs.value().length_ <= 0)
                {
                    geomAttribs.errors().emplace_back("Reference line must have strictly positive length",
                                                      XodrInvalidations::GEOMETRY);
                }
                if (geomAttribs.value().startVertex_.sCoord_ < 0)
                {
                    geomAttribs.errors().emplace_back("Reference line s-offset must not be negative",
                                                      XodrInvalidations::GEOMETRY);
                }
            }
            refLine.appendErrors(geomAttribs);

            xml.readStartElement();
            const std::string& elemName = xml.getCurElementName();
            if (elemName == "line")
            {
                XodrParseResult<Line> res = Line::parseXml(geomAttribs.value(), xml);
                refLine.value().geometries_.emplace_back(new Line(std::move(res.value())));
                refLine.appendErrors(res);
            }
            else if (elemName == "spiral")
            {
                XodrParseResult<Spiral> res = Spiral::parseXml(geomAttribs.value(), xml);
                refLine.value().geometries_.emplace_back(new Spiral(std::move(res.value())));
                refLine.appendErrors(res);
            }
            else if (elemName == "arc")
            {
                XodrParseResult<Arc> res = Arc::parseXml(geomAttribs.value(), xml);
                refLine.value().geometries_.emplace_back(new Arc(std::move(res.value())));
                refLine.appendErrors(res);
            }
            else if (elemName == "poly3")
            {
                XodrParseResult<Poly3Geom> res = Poly3Geom::parseXml(geomAttribs.value(), xml);
                refLine.value().geometries_.emplace_back(new Poly3Geom(std::move(res.value())));
                refLine.appendErrors(res);
            }
            else if (elemName == "paramPoly3")
            {
                XodrParseResult<ParamPoly3> res = ParamPoly3::parseXml(geomAttribs.value(), xml);
                refLine.value().geometries_.emplace_back(new ParamPoly3(std::move(res.value())));
                refLine.appendErrors(res);
            }
            else
            {
                std::stringstream err;
                err << "'" << elemName
                    << "' is not a valid type of geometry. Expected one of 'line', 'spiral', 'arc', 'poly3' or "
                       "'paramPoly3'.";
                refLine.errors().emplace_back(err.str(), XodrInvalidations::GEOMETRY);
            }
            xml.readEndElement();
        },
        XodrInvalidations::ALL);
    if (!ret.value().geometries_.empty())
    {
        ret.value().endVertex_ = ret.value().geometries_.back()->endVertex();
    }

    return ret;
}

XodrParseResult<ReferenceLine::Line> ReferenceLine::Line::parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml)
{
    Line line;
    line.setGeometryAttribs(geomAttribs);
    xml.readEndElement();
    return line;
}

class ReferenceLine::Spiral::AttribParsers : public XmlAttributeParsers<XodrParseResult<Spiral>>
{
  public:
    AttribParsers()
    {
        addFieldParser("curvStart", &Spiral::startCurvature_, XodrInvalidations::GEOMETRY);
        addFieldParser("curvEnd", &Spiral::endCurvature_, XodrInvalidations::GEOMETRY);
        finalize();
    }
};

XodrParseResult<ReferenceLine::Spiral> ReferenceLine::Spiral::parseXml(const GeometryAttribs& geomAttribs,
                                                                       XodrReader& xml)
{
    XodrParseResult<Spiral> ret;
    ret.value().setGeometryAttribs(geomAttribs);

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    if (ret.hasValidGeometry() && ret.value().curvatureRateOfChange() == 0)
    {
        ret.errors().emplace_back("The 'curvStart' and 'curvEnd' attributes of a <spiral> shouldn't be equal.",
                                  XodrInvalidations::GEOMETRY);
    }

    xml.readEndElement();
    return ret;
}

class ReferenceLine::Arc::AttribParsers : public XmlAttributeParsers<XodrParseResult<Arc>>
{
  public:
    AttribParsers()
    {
        addParser("curvature",
                  [](const std::string& value, XodrParseResult<Arc>& arc) {
                      double curvature = xml_parsers::parseXmlAttrib<double>(value);
                      if (curvature == 0)
                      {
                          throw std::runtime_error("The curvature attribute of an <arc> element should be non-zero.");
                      }

                      arc.value().curvature_ = curvature;
                  },
                  XodrInvalidations::GEOMETRY);

        finalize();
    }
};

XodrParseResult<ReferenceLine::Arc> ReferenceLine::Arc::parseXml(const GeometryAttribs& geomAttribs, XodrReader& xml)
{
    XodrParseResult<Arc> arc;
    arc.value().setGeometryAttribs(geomAttribs);

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, arc);

    xml.readEndElement();
    return arc;
}

class ReferenceLine::Poly3Geom::AttribParsers : public XmlAttributeParsers<XodrParseResult<Poly3Geom>>
{
  public:
    AttribParsers()
    {
        addSetterParser("a", &Poly3Geom::setA, XodrInvalidations::GEOMETRY);
        addSetterParser("b", &Poly3Geom::setB, XodrInvalidations::GEOMETRY);
        addSetterParser("c", &Poly3Geom::setC, XodrInvalidations::GEOMETRY);
        addSetterParser("d", &Poly3Geom::setD, XodrInvalidations::GEOMETRY);
        finalize();
    }
};

XodrParseResult<ReferenceLine::Poly3Geom> ReferenceLine::Poly3Geom::parseXml(const GeometryAttribs& geomAttribs,
                                                                             XodrReader& xml)
{
    XodrParseResult<Poly3Geom> poly3;
    poly3.value().setGeometryAttribs(geomAttribs);

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, poly3);

    xml.readEndElement();
    return poly3;
}

class ReferenceLine::ParamPoly3::AttribParsers : public XmlAttributeParsers<XodrParseResult<ParamPoly3>>
{
  public:
    AttribParsers()
    {
        addSetterParser("aU", &ParamPoly3::setAU, XodrInvalidations::GEOMETRY);
        addSetterParser("bU", &ParamPoly3::setBU, XodrInvalidations::GEOMETRY);
        addSetterParser("cU", &ParamPoly3::setCU, XodrInvalidations::GEOMETRY);
        addSetterParser("dU", &ParamPoly3::setDU, XodrInvalidations::GEOMETRY);

        addSetterParser("aV", &ParamPoly3::setAV, XodrInvalidations::GEOMETRY);
        addSetterParser("bV", &ParamPoly3::setBV, XodrInvalidations::GEOMETRY);
        addSetterParser("cV", &ParamPoly3::setCV, XodrInvalidations::GEOMETRY);
        addSetterParser("dV", &ParamPoly3::setDV, XodrInvalidations::GEOMETRY);

        addOptionalFieldParser("pRange", &ParamPoly3::pRange_, PRange::NORMALIZED, XodrInvalidations::GEOMETRY);

        finalize();
    }
};

XodrParseResult<ReferenceLine::ParamPoly3> ReferenceLine::ParamPoly3::parseXml(const GeometryAttribs& geomAttribs,
                                                                               XodrReader& xml)
{
    XodrParseResult<ParamPoly3> paramPoly3;
    paramPoly3.value().setGeometryAttribs(geomAttribs);

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, paramPoly3);

    xml.readEndElement();
    return paramPoly3;
}

}}  // namespace aid::xodr
