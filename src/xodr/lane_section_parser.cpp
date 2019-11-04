#include "lane_section.h"

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {
namespace xml_parsers {

template <>
LaneType parseXmlAttrib(const std::string& value)
{
    if (value == "none")
    {
        return LaneType::NONE;
    }
    else if (value == "driving")
    {
        return LaneType::DRIVING;
    }
    else if (value == "stop")
    {
        return LaneType::STOP;
    }
    else if (value == "shoulder")
    {
        return LaneType::SHOULDER;
    }
    else if (value == "biking")
    {
        return LaneType::BIKING;
    }
    else if (value == "sidewalk")
    {
        return LaneType::SIDEWALK;
    }
    else if (value == "border")
    {
        return LaneType::BORDER;
    }
    else if (value == "restricted")
    {
        return LaneType::RESTRICTED;
    }
    else if (value == "parking")
    {
        return LaneType::PARKING;
    }
    else if (value == "bidirectional")
    {
        return LaneType::BIDIRECTIONAL;
    }
    else if (value == "median")
    {
        return LaneType::MEDIAN;
    }
    else if (value == "special1")
    {
        return LaneType::SPECIAL1;
    }
    else if (value == "special2")
    {
        return LaneType::SPECIAL2;
    }
    else if (value == "special3")
    {
        return LaneType::SPECIAL3;
    }
    else if (value == "roadWorks")
    {
        return LaneType::ROADWORKS;
    }
    else if (value == "tram")
    {
        return LaneType::TRAM;
    }
    else if (value == "rail")
    {
        return LaneType::RAIL;
    }
    else if (value == "entry")
    {
        return LaneType::ENTRY;
    }
    else if (value == "exit")
    {
        return LaneType::EXIT;
    }
    else if (value == "offRamp")
    {
        return LaneType::OFF_RAMP;
    }
    else if (value == "onRamp")
    {
        return LaneType::ON_RAMP;
    }
    else if (value == "connectingRamp")
    {
        return LaneType::CONNECTING_RAMP;
    }
    else if (value == "bus")
    {
        return LaneType::BUS;
    }
    else if (value == "taxi")
    {
        return LaneType::TAXI;
    }
    else if (value == "hov")
    {
        return LaneType::HOV;
    }
    else
    {
        std::stringstream err;
        err << "Not a valid lane type: " << value;
        throw std::invalid_argument(err.str());
    }
}
}  // namespace xml_parsers

class LaneSection::AttribParsers : public XmlAttributeParsers<XodrParseResult<LaneSection>>
{
  public:
    AttribParsers()
    {
        addFieldParser("s", &LaneSection::startS_, XodrInvalidations::GEOMETRY);
        addOptionalFieldParser("singleSided", &LaneSection::singleSided_, false, XodrInvalidations::GEOMETRY);
        finalize();
    }
};

class LaneSection::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<LaneSection>>
{
  public:
    ChildElemParsers()
    {
        addParser("left", Multiplicity::ZERO_OR_ONE, &LaneSection::parseLeftLanes, XodrInvalidations::ALL);

        addParser("center", Multiplicity::ONE,
                  [](XodrReader& xml, XodrParseResult<LaneSection>& laneSection) {
                      if (!laneSection.value().lanes_.empty() && laneSection.value().lanes_.back().id() != LaneID(1))
                      {
                          laneSection.errors().emplace_back("Lanes should occur with consecutive and descending IDs.",
                                                            XodrInvalidations::ALL);
                          return;
                      }

                      xml.skipToEndElement();
                  },
                  XodrInvalidations::ALL);

        addParser("right", Multiplicity::ZERO_OR_ONE, &LaneSection::parseRightLanes);

        finalize();
    }
};

XodrParseResult<LaneSection> LaneSection::parseXml(XodrReader& xml)
{
    XodrParseResult<LaneSection> ret;

    ret.value().numLeftLanes_ = 0;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    return ret;
}

void LaneSection::parseLeftLanes(XodrReader& xml, XodrParseResult<LaneSection>& laneSection)
{
    XmlChildElementParsers<XodrReader, XodrParseResult<LaneSection>>::parseOneOrMore(
        xml, laneSection, "lane", [](XodrReader& xml, XodrParseResult<LaneSection>& laneSection) {
            XodrParseResult<Lane> lane = Lane::parseXml(xml);
            if (lane.hasValidGeometry())
            {
                if (lane.value().id() <= LaneID(0))
                {
                    lane.errors().emplace_back(
                        XodrParseError("Left lanes must have a positive ID.", XodrInvalidations::ALL));
                }

                if (!laneSection.value().lanes_.empty())
                {
                    const Lane& prevLane = laneSection.value().lanes_.back();
                    if (static_cast<int>(prevLane.id()) - 1 != static_cast<int>(lane.value().id()))
                    {
                        lane.errors().emplace_back("Lanes should occur with consecutive and descending IDs.",
                                                   XodrInvalidations::ALL);
                    }
                }
            }

            laneSection.appendErrors(lane);
            laneSection.value().lanes_.push_back(std::move(lane.value()));
            laneSection.value().numLeftLanes_++;
        });
}

void LaneSection::parseRightLanes(XodrReader& xml, XodrParseResult<LaneSection>& laneSection)
{
    XmlChildElementParsers<XodrReader, XodrParseResult<LaneSection>>::parseOneOrMore(
        xml, laneSection, "lane", [](XodrReader& xml, XodrParseResult<LaneSection>& laneSection) {
            XodrParseResult<Lane> lane = Lane::parseXml(xml);
            if (lane.hasValidGeometry())
            {
                if (lane.value().id() >= LaneID(0))
                {
                    lane.errors().emplace_back("Right lanes must have a negative ID.", XodrInvalidations::ALL);
                }

                if (laneSection.value().lanes_.empty() || laneSection.value().lanes_.back().id() == LaneID(1))
                {
                    if (lane.value().id() != LaneID(-1))
                    {
                        lane.errors().emplace_back("Lanes should occur with consecutive and descending IDs.",
                                                   XodrInvalidations::ALL);
                    }
                }
                else
                {
                    if (static_cast<int>(laneSection.value().lanes_.back().id()) - 1 !=
                        static_cast<int>(lane.value().id()))
                    {
                        lane.errors().emplace_back("Lanes should occur with consecutive and descending IDs.",
                                                   XodrInvalidations::ALL);
                    }
                }
            }

            laneSection.appendErrors(lane);
            laneSection.value().lanes_.push_back(std::move(lane.value()));
        });
}

class LaneSection::Lane::AttribParsers : public XmlAttributeParsers<XodrParseResult<Lane>>
{
  public:
    AttribParsers()
    {
        addFieldParser("id", &Lane::id_, XodrInvalidations::ALL);
        addFieldParser("type", &Lane::type_);
        addOptionalFieldParser("level", &Lane::level_, false);
        finalize();
    }
};

class LaneSection::Lane::LinkChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Lane>>
{
  public:
    LinkChildElemParsers()
    {
        addParser("predecessor", Multiplicity::ZERO_OR_ONE,
                  [](XodrReader& xml, XodrParseResult<Lane>& lane) {
                      XmlAttributeParsers<XodrParseResult<Lane>>::parseField(xml, lane, "id", &Lane::predecessor_);
                      xml.skipToEndElement();
                  },
                  XodrInvalidations::CONNECTIVITY);

        addParser("successor", Multiplicity::ZERO_OR_ONE,
                  [](XodrReader& xml, XodrParseResult<Lane>& lane) {
                      XmlAttributeParsers<XodrParseResult<Lane>>::parseField(xml, lane, "id", &Lane::successor_);
                      xml.skipToEndElement();
                  },
                  XodrInvalidations::CONNECTIVITY);

        finalize();
    }
};

class LaneSection::Lane::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Lane>>
{
  public:
    ChildElemParsers()
    {
        addParser("link", Multiplicity::ZERO_OR_ONE, [](XodrReader& xml, XodrParseResult<Lane>& lane) {
            static const LinkChildElemParsers linkChildElemParsers;
            linkChildElemParsers.parse(xml, lane);
        });

        addVectorElementParser<XodrParseResult<WidthPoly3>>("width", &Lane::widthPoly3s_, Multiplicity::ONE_OR_MORE,
                                                            XodrInvalidations::GEOMETRY);

        addVectorElementParser<XodrParseResult<LaneMaterial>>("material", &Lane::materials_,
                                                              Multiplicity::ZERO_OR_MORE);
        addVectorElementParser<XodrParseResult<LaneVisibility>>("visibility", &Lane::visibilities_,
                                                                Multiplicity::ZERO_OR_MORE);
        addVectorElementParser<XodrParseResult<LaneSpeedLimit>>("speed", &Lane::speedLimits_,
                                                                Multiplicity::ZERO_OR_MORE);
        addVectorElementParser<XodrParseResult<LaneAccess>>("access", &Lane::accesses_, Multiplicity::ZERO_OR_MORE);
        addVectorElementParser<XodrParseResult<LaneHeight>>("height", &Lane::heights_, Multiplicity::ZERO_OR_MORE);
        addVectorElementParser<XodrParseResult<LaneRule>>("rule", &Lane::rules_, Multiplicity::ZERO_OR_MORE);

        finalize();
    }
};

XodrParseResult<LaneSection::Lane> LaneSection::Lane::parseXml(XodrReader& xml)
{
    XodrParseResult<Lane> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    ret.value().globalIndex_ = xml.newGlobalLaneIndex();

    return ret;
}

class LaneSection::WidthPoly3::AttribParsers : public XmlAttributeParsers<XodrParseResult<WidthPoly3>>
{
  public:
    AttribParsers()
    {
        addFieldParser("sOffset", &WidthPoly3::sOffset_, XodrInvalidations::GEOMETRY);
        addSetterParser("a", &WidthPoly3::setA, XodrInvalidations::GEOMETRY);
        addSetterParser("b", &WidthPoly3::setB, XodrInvalidations::GEOMETRY);
        addSetterParser("c", &WidthPoly3::setC, XodrInvalidations::GEOMETRY);
        addSetterParser("d", &WidthPoly3::setD, XodrInvalidations::GEOMETRY);
        finalize();
    }
};

XodrParseResult<LaneSection::WidthPoly3> LaneSection::WidthPoly3::parseXml(XodrReader& xml)
{
    XodrParseResult<WidthPoly3> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    xml.skipToEndElement();

    return ret;
}

}}  // namespace aid::xodr
