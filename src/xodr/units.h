#pragma once

#include "xml/xml_attribute_parsers.h"
#include "xodr_reader.h"

namespace aid { namespace xodr {

enum class DistanceUnit
{
    NOT_SPECIFIED,
    METER,
    KILOMETER,
    FEET,
    LAND_MILE
};

enum class SpeedUnit
{
    NOT_SPECIFIED,
    METERS_PER_SECOND,
    MILES_PER_HOUR,
    KILOMETERS_PER_HOUR
};

enum class MassUnit
{
    NOT_SPECIFIED,
    KILOGRAM,
    METRIC_TONS
};

namespace xml_parsers {

template <>
DistanceUnit parseXmlAttrib(const std::string& value);

template <>
SpeedUnit parseXmlAttrib(const std::string& value);

template <>
MassUnit parseXmlAttrib(const std::string& value);

}  // namespace xml_parsers

}}  // namespace aid::xodr