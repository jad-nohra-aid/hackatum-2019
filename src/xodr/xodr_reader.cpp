#include "xodr_reader.h"

namespace aid { namespace xodr {

namespace {
class XodrParseErrorDescriptionVisitor : public boost::static_visitor<std::string>
{
  public:
    std::string operator()(const XmlParseError& xmlParseError) const { return xmlParseError.description(); }

    std::string operator()(const std::string& message) const { return message; }
};
}  // namespace

std::string XodrParseError::description() const
{
    return boost::apply_visitor(XodrParseErrorDescriptionVisitor(), data_);
}

bool XodrParseError::isFatal() const
{
    struct Visitor : public boost::static_visitor<bool>
    {
        bool operator()(const XmlParseError& err) const { return err.isFatal(); }

        bool operator()(const std::string& err) const { return true; }
    };

    return boost::apply_visitor(Visitor(), data_);
}

XodrReader XodrReader::fromFile(const std::string& fileName)
{
    XodrReader ret;
    ret.initFromFile(fileName);
    return ret;
}

XodrReader XodrReader::fromText(const std::string& text)
{
    XodrReader ret;
    ret.initFromText(text);
    return ret;
}
}}  // namespace aid::xodr