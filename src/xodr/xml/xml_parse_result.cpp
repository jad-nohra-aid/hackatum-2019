#include "xml/xml_parse_result.h"

#include <sstream>

namespace aid { namespace xodr {

std::string XmlParseError::description() const
{
    std::stringstream message;
    switch (category_)
    {
        case Category::MISSING_ATTRIBUTE:
            message << "XML element <" << name_ << "> should have attribute named " << value_;
            break;
        case Category::UNEXPECTED_ATTRIBUTE:
            message << "XML element <" << name_ << "> has unexpected attribute named " << value_;
            break;
        case Category::INVALID_ATTRIBUTE_VALUE:
            message << "Invalid value " << value_ << " for attribute " << name_;
            break;
        case Category::MISSING_CHILD_ELEMENT:
            message << "XML element <" << name_ << "> should have at least one child element <" << value_ << ">";
            break;
        case Category::UNEXPECTED_CHILD_ELEMENT:
            message << "XML element <" << name_ << "> has unexpected child element <" << value_ << ">";
            break;
        case Category::DUPLICATE_CHILD_ELEMENT:
            message << "XML element <" << name_ << "> should not have more than one child element <" << value_ << ">";
            break;
    }
    return message.str();
}

bool XmlParseError::isFatal() const
{
	return category_ != Category::UNEXPECTED_ATTRIBUTE &&
		category_ != Category::UNEXPECTED_CHILD_ELEMENT;
}

}}  // namespace aid::xodr