#include "road.h"

#include "xml/xml_child_element_parsers.h"
#include "xml/xml_attribute_parsers.h"

namespace aid { namespace xodr {

class Road::AttribParsers : public XmlAttributeParsers<XodrParseResult<Road>>
{
  public:
    AttribParsers()
    {
        addFieldParser("name", &Road::name_);
        addFieldParser("length", &Road::length_, XodrInvalidations::GEOMETRY);
        addFieldParser("id", &Road::id_, XodrInvalidations::ALL);
        addFieldParser("junction", &Road::junctionRef_, XodrInvalidations::CONNECTIVITY);
        finalize();
    }
};

class Road::LaneChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Road>>
{
  public:
    LaneChildElemParsers()
    {
        addParser("laneSection", Multiplicity::ONE_OR_MORE, [](XodrReader& xml, XodrParseResult<Road>& road) {
            XodrParseResult<LaneSection> laneSection = LaneSection::parseXml(xml);
            if (road.value().laneSections_.empty())
            {
                if (laneSection.value().startS() != 0)
                {
                    std::stringstream err;
                    err << "The first <laneSection> of the road with id '" << road.value().id()
                        << "' does not start at s-coordinate 0.";
                    road.errors().emplace_back(err.str(), XodrInvalidations::GEOMETRY);
                }
            }
            else
            {
                LaneSection& prevLaneSection = road.value().laneSections_.back();
                if (prevLaneSection.startS() >= laneSection.value().startS())
                {
                    std::stringstream err;
                    err << "The <laneSection>s of the road with id '" << road.value().id()
                        << "' do not appear in ascending order of starting s-coordinates.";
                    road.errors().emplace_back(err.str(), XodrInvalidations::GEOMETRY);
                }

                prevLaneSection.endS_ = laneSection.value().startS_;
            }

            road.value().laneSections_.push_back(std::move(laneSection.value()));
        });

        finalize();
    }
};

class Road::ObjectsChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Road>>
{
  public:
    ObjectsChildElemParsers()
    {
        addVectorElementParser<XodrParseResult<RoadObject>>("object", &Road::roadObjects_, Multiplicity::ZERO_OR_MORE);

        addParser("objectReference", Multiplicity::ZERO_OR_MORE, [](XodrReader&, XodrParseResult<Road>& result) {
            result.errors().emplace_back("WARNING: <objectReference> element not implemented yet.");
        });

        addParser("tunnel", Multiplicity::ZERO_OR_MORE, [](XodrReader&, XodrParseResult<Road>& result) {
            result.errors().emplace_back("WARNING: <tunnel> element not implemented yet.");
        });

        addParser("bridge", Multiplicity::ZERO_OR_MORE, [](XodrReader&, XodrParseResult<Road>& result) {
            result.errors().emplace_back("WARNING: <bridge> element not implemented yet.");
        });

        finalize();
    }
};

class Road::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<Road>>
{
  public:
    ChildElemParsers()
    {
        addFieldParser<XodrParseResult<ReferenceLine>>("planView", &Road::referenceLine_, XodrInvalidations::ALL);
        addOptionalFieldParser<XodrParseResult<ElevationProfile>>("elevationProfile", &Road::elevationProfile_);

        addParser("lanes", Multiplicity::ONE,
                  [](XodrReader& xml, XodrParseResult<Road>& road) {
                      static const LaneChildElemParsers childElemParsers;
                      childElemParsers.parse(xml, road);
                  },
                  XodrInvalidations::GEOMETRY);

        addOptionalFieldParser<XodrParseResult<RoadLinks>>("link", &Road::links_, RoadLinks(),
                                                           XodrInvalidations::CONNECTIVITY);

        addParser("objects", Multiplicity::ZERO_OR_ONE, [](XodrReader& xml, XodrParseResult<Road>& road) {
            static const ObjectsChildElemParsers childElemParsers;
            childElemParsers.parse(xml, road);
        });

        finalize();
    }
};

XodrParseResult<Road> Road::parseXml(XodrReader& xml)
{
    XodrParseResult<Road> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    if (!ret.hasValidGeometry())
    {
        // No LENGTH attribute means the next steps are not possible
        return ret;
    }

    if (ret.value().laneSections_.back().startS() >= ret.value().referenceLine_.endS())
    {
        std::stringstream err;
        err << "A laneSection of the road with id '" << ret.value().id() << "' has invalid endS.";
        ret.errors().emplace_back(err.str(), XodrInvalidations::GEOMETRY);
    }

    ret.value().laneSections_.back().endS_ = ret.value().referenceLine_.endS();
    return ret;
}

void Road::resolveReferences(const IdToIndexMaps& idToIndexMaps)
{
    junctionRef_.resolve(idToIndexMaps.junctionIdToIndex_, "-1", "junction");
    links_.resolveReferences(idToIndexMaps);
}

int Road::globalLaneIndicesBegin() const
{
    return laneSections_.front().lanes().front().globalIndex();
}

int Road::globalLaneIndicesEnd() const
{
    return laneSections_.back().lanes().back().globalIndex() + 1;
}

}}  // namespace aid::xodr
