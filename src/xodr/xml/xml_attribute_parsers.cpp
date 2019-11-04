#include "xml/xml_attribute_parsers.h"

namespace aid { namespace xodr { namespace xml_parsers {
template <>
int parseXmlAttrib<int>(const std::string& value)
{
    return std::stoi(value);
}

template <>
double parseXmlAttrib<double>(const std::string& value)
{
    return std::stod(value);
}

template <>
std::string parseXmlAttrib<std::string>(const std::string& value)
{
    return value;
}

template <>
bool parseXmlAttrib<bool>(const std::string& value)
{
    if (value == "true")
    {
        return true;
    }
    else if (value == "false")
    {
        return false;
    }
    else if (value == "1")
    {
        return true;
    }
    else if (value == "0")
    {
        return false;
    }
    else
    {
        throw std::invalid_argument(value);
    }
}
}}}  // namespace aid::xodr::xml_parsers