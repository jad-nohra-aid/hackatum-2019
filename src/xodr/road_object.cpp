#include "road_object.h"

#include <limits>
#include <map>
#include <sstream>

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_child_element_parsers.h"

namespace aid { namespace xodr {

namespace xml_parsers {
template <>
RoadObject::Type parseXmlAttrib(const std::string& value)
{
    static const std::map<std::string, RoadObject::Type> mapping = {
        {"none", RoadObject::Type::NONE},
        {"obstacle", RoadObject::Type::OBSTACLE},
        {"car", RoadObject::Type::CAR},
        {"truck", RoadObject::Type::TRUCK},
        {"van", RoadObject::Type::VAN},
        {"trailer", RoadObject::Type::TRAILER},
        {"bike", RoadObject::Type::BIKE},
        {"motorbike", RoadObject::Type::MOTORBIKE},
        {"tram", RoadObject::Type::TRAM},
        {"train", RoadObject::Type::TRAIN},
        {"pedestrian", RoadObject::Type::PEDESTRIAN},
        {"pole", RoadObject::Type::POLE},
        {"tree", RoadObject::Type::TREE},
        {"vegetation", RoadObject::Type::VEGETATION},
        {"barrier", RoadObject::Type::BARRIER},
        {"building", RoadObject::Type::BUILDING},
        {"parkingSpace", RoadObject::Type::PARKING_SPACE},
        {"wind", RoadObject::Type::WIND},
        {"patch", RoadObject::Type::PATCH},
    };

    auto it = mapping.find(value);
    if (it == mapping.end())
    {
        std::stringstream err;
        err << "'" << value << "' isn't a valid road object type.";
        throw std::runtime_error(err.str());
    }

    return it->second;
}

template <>
RoadObject::Orientation parseXmlAttrib(const std::string& value)
{
    static std::map<std::string, RoadObject::Orientation> mapping = {
        {"+", RoadObject::Orientation::POSITIVE},
        {"-", RoadObject::Orientation::NEGATIVE},
        {"none", RoadObject::Orientation::NONE},
    };

    auto it = mapping.find(value);
    if (it == mapping.end())
    {
        std::stringstream err;
        err << "'" << value << "' isn't a valid road object orientation.";
        throw std::runtime_error(err.str());
    }

    return it->second;
}
}  // namespace xml_parsers

class RoadObject::AttribParsers : public XmlAttributeParsers<XodrParseResult<RoadObject>>
{
  public:
    AttribParsers()
    {
        addFieldParser("type", &RoadObject::type_);
        addFieldParser("name", &RoadObject::name_);
        addFieldParser("id", &RoadObject::id_);
        addFieldParser("s", &RoadObject::s_);
        addFieldParser("t", &RoadObject::t_);
        addFieldParser("zOffset", &RoadObject::zOffset_);
        addFieldParser("validLength", &RoadObject::validLength_);
        addFieldParser("orientation", &RoadObject::orientation_);
        addOptionalFieldParser("length", &RoadObject::length_, std::numeric_limits<double>::quiet_NaN());
        addOptionalFieldParser("width", &RoadObject::width_, std::numeric_limits<double>::quiet_NaN());
        addOptionalFieldParser("radius", &RoadObject::radius_, std::numeric_limits<double>::quiet_NaN());
        addOptionalFieldParser("height", &RoadObject::height_, std::numeric_limits<double>::quiet_NaN());
        addFieldParser("hdg", &RoadObject::heading_);
        addFieldParser("pitch", &RoadObject::pitch_);
        addFieldParser("roll", &RoadObject::roll_);

        finalize();
    }
};

class RoadObject::ChildElemParsers : public XmlChildElementParsers<XodrReader, XodrParseResult<RoadObject>>
{
  public:
    ChildElemParsers()
    {
        addParser("repeat", Multiplicity::ZERO_OR_MORE, [](XodrReader& xml, XodrParseResult<RoadObject>& result) {
            result.errors().emplace_back("WARNING: <repeat> element not implemented yet.");
            xml.skipToEndElement();
        });
        addParser("outline", Multiplicity::ZERO_OR_ONE, [](XodrReader& xml, XodrParseResult<RoadObject>& result) {
            auto ret = RoadObjectOutline::parseXml(xml);
            result.value().outline_.reset(new RoadObjectOutline(ret.value()));
            result.appendErrors(ret);
        });

        addParser("validity", Multiplicity::ZERO_OR_MORE, [](XodrReader& xml, XodrParseResult<RoadObject>& result) {
            result.errors().emplace_back("WARNING: <validity> element not implemented yet.");
            xml.skipToEndElement();
        });

        addParser("parkingSpace", Multiplicity::ZERO_OR_MORE, [](XodrReader& xml, XodrParseResult<RoadObject>& result) {
            result.errors().emplace_back("WARNING: <parkingSpace> element not implemented yet.");
            xml.skipToEndElement();
        });

        finalize();
    }
};

XodrParseResult<RoadObject> RoadObject::parseXml(XodrReader& xml)
{
    XodrParseResult<RoadObject> ret;

    static const AttribParsers attribParsers;
    attribParsers.parse(xml, ret);

    static const ChildElemParsers childElemParsers;
    childElemParsers.parse(xml, ret);

    ret.value().validateGeometry(ret.errors());

    return ret;
}

void RoadObject::validateGeometry(std::vector<XodrParseError>& errors)
{
    if (!std::isnan(length_))
    {
        if (std::isnan(width_))
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' has missing 'width' attribute. A 'length' attribute is "
                << "specified, so a 'width' attribute must be specified too.";
            errors.emplace_back(err.str());
        }

        if (!std::isnan(radius_))
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' has both 'length' and 'radius' attributes. "
                << "Either a pair of 'length' and 'width' attributes or a 'radius' "
                << "attribute should be specified, but not both.";
            errors.emplace_back(err.str());
        }

        if (std::isnan(height_))
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' does not have a 'height' attribute. "
                << "The 'height' attribute is required if the 'length' and 'width' "
                << "attributes are specified.";
            errors.emplace_back(err.str());
        }

        if (outline_ != nullptr)
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' has both 'length' and 'outline' attributes. "
                << "Either a pair of 'length' and 'width' attributes or an 'outline' "
                << "attribute should be specified, but not both.";
            errors.emplace_back(err.str());
        }
    }
    else if (!std::isnan(width_))
    {
        std::stringstream err;
        err << "Road object with ID '" << id_ << "' has missing 'length' attribute. A 'width' attribute is "
            << "specified, so a 'length' attribute must be specified too.";
        errors.emplace_back(err.str());
    }
    else if (!std::isnan(radius_))
    {
        if (std::isnan(height_))
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' has missing 'height' attribute. A 'radius' attribute is "
                << "specified, so a 'height' attribute must be specified too.";
            errors.emplace_back(err.str());
        }

        if (outline_ != nullptr)
        {
            std::stringstream err;
            err << "Road object with ID '" << id_ << "' has both 'radius' and 'outline' attributes. "
                << "Either a 'radius' attributes or an 'outline' "
                << "attribute should be specified, but not both.";
            errors.emplace_back(err.str());
        }
    }
    else if (outline_ == nullptr)
    {
        std::stringstream err;
        err << "Road object with ID '" << id_ << "' does not have any size specification. "
            << "Either a pair of 'length' and 'width' attributes, a 'radius' "
            << "attribute or an 'outline' child element expected.";
        errors.emplace_back(err.str());
    }
}

std::string RoadObject::typeName(Type type)
{
    switch (type)
    {
        default:
            assert("Invalid road object type.");

        case Type::NONE:
            return "none";
        case Type::OBSTACLE:
            return "obstacle";
        case Type::CAR:
            return "car";
        case Type::TRUCK:
            return "truck";
        case Type::VAN:
            return "van";
        case Type::BUS:
            return "bus";
        case Type::TRAILER:
            return "trailer";
        case Type::BIKE:
            return "bike";
        case Type::MOTORBIKE:
            return "motorbike";
        case Type::TRAM:
            return "tram";
        case Type::TRAIN:
            return "train";
        case Type::PEDESTRIAN:
            return "pedestrian";
        case Type::POLE:
            return "pole";
        case Type::TREE:
            return "tree";
        case Type::VEGETATION:
            return "vegetation";
        case Type::BARRIER:
            return "barrier";
        case Type::BUILDING:
            return "building";
        case Type::PARKING_SPACE:
            return "parkingSpace";
        case Type::WIND:
            return "wind";
        case Type::PATCH:
            return "patch";
    }
}

bool RoadObject::hasBoxGeometry() const
{
    assert(std::isnan(length_) == std::isnan(width_));
    return !std::isnan(length_);
}

double RoadObject::length() const
{
    assert(hasBoxGeometry());
    return length_;
}

double RoadObject::width() const
{
    assert(hasBoxGeometry());
    return width_;
}

double RoadObject::radius() const
{
    assert(hasCylinderGeometry());
    return radius_;
}

double RoadObject::height() const
{
    assert(hasBoxGeometry() || hasCylinderGeometry());
    return height_;
}

const RoadObjectOutline& RoadObject::outline() const
{
    assert(hasOutlineGeometry());
    return *outline_;
}

}}  // namespace aid::xodr