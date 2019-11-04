#include "units.h"

namespace aid { namespace xodr { namespace xml_parsers {

template <>
DistanceUnit parseXmlAttrib(const std::string& value)
{
    if (value == "m")
    {
        return DistanceUnit::METER;
    }
    else if (value == "km")
    {
        return DistanceUnit::KILOMETER;
    }
    else if (value == "ft")
    {
        return DistanceUnit::FEET;
    }
    else if (value == "mile")
    {
        return DistanceUnit::LAND_MILE;
    }
    else
    {
        std::stringstream err;
        err << "Not a distance unit: '" << value << "'.";
        throw std::invalid_argument(err.str());
    }
}

template <>
SpeedUnit parseXmlAttrib(const std::string& value)
{
    if (value == "m/s")
    {
        return SpeedUnit::METERS_PER_SECOND;
    }
    else if (value == "mps")
    {
        return SpeedUnit::MILES_PER_HOUR;
    }
    else if (value == "km/h")
    {
        return SpeedUnit::KILOMETERS_PER_HOUR;
    }
    else
    {
        std::stringstream err;
        err << "Not a speed unit: '" << value << "'.";
        throw std::invalid_argument(err.str());
    }
}

template <>
MassUnit parseXmlAttrib(const std::string& value)
{
    if (value == "kg")
    {
        return MassUnit::KILOGRAM;
    }
    else if (value == "t")
    {
        return MassUnit::METRIC_TONS;
    }
    else
    {
        std::stringstream err;
        err << "Not a mass unit: '" << value << "'.";
        throw std::invalid_argument(err.str());
    }
}

}}}  // namespace aid::xodr::xml_parsers