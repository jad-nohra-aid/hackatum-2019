#include "xodr_map.h"
#include "validation/road_link_validation.h"
#include "validation/junction_validation.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

XodrParseResult<XodrMap> XodrMap::fromFile(const std::string& fileName)
{
    XodrReader reader = XodrReader::fromFile(fileName);
    reader.readStartElement("OpenDRIVE");
    return XodrMap::parseXml(reader);
}

XodrParseResult<XodrMap> XodrMap::fromText(const std::string& text)
{
    XodrReader reader = XodrReader::fromText(text);
    reader.readStartElement("OpenDRIVE");
    return XodrMap::parseXml(reader);
}

class XodrMap::HeaderChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<XodrMap>>
{
  public:
    HeaderChildElemParsers()
    {
        addParser("geoReference", Multiplicity::ZERO_OR_ONE, [](XodrReader& xml, XodrParseResult<XodrMap>& map) {
            map.value().geoReference_.emplace(xml.getText());
            xml.readEndElement();
        });
        finalize();
    }
};

class XodrMap::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<XodrMap>>
{
  public:
    ChildElemParsers()
    {
        addVectorElementParser<XodrParseResult<Road>>("road", &XodrMap::roads_, Multiplicity::ONE_OR_MORE,
                                                      XodrInvalidations::ALL);
        addVectorElementParser<XodrParseResult<Junction>>("junction", &XodrMap::junctions_, Multiplicity::ZERO_OR_MORE,
                                                          XodrInvalidations::ALL);
        finalize();
    }

  private:
};

XodrParseResult<XodrMap> XodrMap::parseXml(XodrReader& xml)
{
    XodrParseResult<XodrMap> ret;

    xml.readStartElement("header");
    static HeaderChildElemParsers headerChildElemParsers;
    headerChildElemParsers.parse(xml, ret);
    static const ChildElemParsers childElementParsers;
    childElementParsers.parse(xml, ret);
    ret.value().resolveReferences(ret.errors());
    ret.value().totalNumLanes_ = xml.peekNextGlobalLaneIndex();
    return ret;
}

void XodrMap::resolveReferences(std::vector<XodrParseError>& errors)
{
    assert(idToIndexMaps_.roadIdToIndex_.empty());
    assert(idToIndexMaps_.junctionIdToIndex_.empty());

    for (int i = 0; i < static_cast<int>(roads_.size()); i++)
    {
        auto insertRes = idToIndexMaps_.roadIdToIndex_.insert(std::make_pair(roads_[i].id(), i));
        if (!insertRes.second)
        {
            std::stringstream err;
            err << "Multiple roads with id '" << roads_[i].id() << "' found.";
            // Multiple roads with the same id make the map useless
            errors.emplace_back(err.str(), XodrInvalidations::ALL);
            return;
        }
    }

    for (int i = 0; i < static_cast<int>(junctions_.size()); i++)
    {
        auto insertRes = idToIndexMaps_.junctionIdToIndex_.insert(std::make_pair(junctions_[i].id(), i));
        if (!insertRes.second)
        {
            std::stringstream err;
            err << "Multiple junctions with id '" << roads_[i].id() << "' found.";
            // Multiple junctions with the same id make the map useless
            errors.emplace_back(err.str(), XodrInvalidations::ALL);
            return;
        }
    }

    for (Road& road : roads_)
    {
        try
        {
            road.resolveReferences(idToIndexMaps_);
        }
        catch (const std::runtime_error& e)
        {
            std::stringstream err;
            err << "Road with id '" << road.id() << "' has invalid connection. " << e.what();
            errors.emplace_back(err.str(), XodrInvalidations::CONNECTIVITY);
        }
    }

    for (Junction& junction : junctions_)
    {
        try
        {
            junction.resolveReferences(idToIndexMaps_);
        }
        catch (const std::runtime_error& e)
        {
            std::stringstream err;
            err << "Junction with id'" << junction.id() << "' has invalid connection. " << e.what();
            errors.emplace_back(err.str(), XodrInvalidations::CONNECTIVITY);
        }
    }
}

bool XodrMap::hasGeoReference() const
{
    return static_cast<bool>(geoReference_);
}

const std::string& XodrMap::geoReference() const
{
    assert(hasGeoReference());
    return *geoReference_;
}

const Road* XodrMap::roadById(const std::string& id) const
{
    auto it = idToIndexMaps_.roadIdToIndex_.find(id);
    if (it == idToIndexMaps_.roadIdToIndex_.end())
    {
        return nullptr;
    }

    return &roads_[it->second];
}

Road* XodrMap::test_roadById(const std::string& id)
{
    auto it = idToIndexMaps_.roadIdToIndex_.find(id);
    if (it == idToIndexMaps_.roadIdToIndex_.end())
    {
        return nullptr;
    }

    return &roads_[it->second];
}

Junction* XodrMap::test_junctionById(const std::string& id)
{
    auto it = idToIndexMaps_.junctionIdToIndex_.find(id);
    if (it == idToIndexMaps_.junctionIdToIndex_.end())
    {
        return nullptr;
    }

    return &junctions_[it->second];
}

int XodrMap::roadIndexById(const std::string& id) const
{
    auto it = idToIndexMaps_.roadIdToIndex_.find(id);
    if (it == idToIndexMaps_.roadIdToIndex_.end())
    {
        return -1;
    }

    return it->second;
}

const Junction* XodrMap::junctionById(const std::string& id) const
{
    auto it = idToIndexMaps_.junctionIdToIndex_.find(id);
    if (it == idToIndexMaps_.junctionIdToIndex_.end())
    {
        return nullptr;
    }

    return &junctions_[it->second];
}

int XodrMap::junctionIndexById(const std::string& id) const
{
    auto it = idToIndexMaps_.junctionIdToIndex_.find(id);
    if (it == idToIndexMaps_.junctionIdToIndex_.end())
    {
        return -1;
    }

    return it->second;
}

bool XodrMap::hasRoadObjects() const
{
    for (const Road& road : roads_)
    {
        if (!road.roadObjects().empty())
        {
            return true;
        }
    }

    return false;
}

void XodrMap::validate() const
{
    for (const Road& road : roads_)
    {
        road.validate();
    }

    validateJunctionMembership(*this);

    std::vector<std::unique_ptr<LinkValidationError>> errors;
    validateLinks(*this, errors);
    if (!errors.empty())
    {
        throw std::runtime_error("Link validation failed. Run the link validator for exact error messages.");
    }
}

}}  // namespace aid::xodr
