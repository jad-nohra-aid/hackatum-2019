#include <sstream>
#include <algorithm>

namespace aid { namespace xodr {

namespace xml_parsers {
template <class T>
T parseXmlAttrib(const std::string& value)
{
    auto parseResult = T::parse(value);
    if (!parseResult.errors().empty())
    {
        throw std::invalid_argument(value);
    }
    return parseResult.value();
}
}  // namespace xml_parsers

template <class T>
void XmlAttributeParsers<T>::parse(XmlReader& xml, T& result) const
{
    // If this assert triggers then you probably forgot to call finalize.
    assert(optionalMask_ != (uint32_t)-1);

    uint32_t visitedMask = 0;

    std::vector<XmlReader::Attrib> attribs = xml.getAttributes();

    for (const XmlReader::Attrib& attrib : attribs)
    {
        // Perform a binary search to find the parser for the attribute.

        // The current search range consists of the parsers with index i
        // satisfying min <= i < max.
        int min = 0;
        int max = static_cast<int>(parsers_.size());

        while (true)
        {
            // If the current search range is empty, then no parser exists for
            // the current attribute.
            if (min == max)
            {
                // Skip attributes which don't have a parser in this XmlAttributeParsers.
                result.errors().emplace_back(XmlParseError(XmlParseError::Category::UNEXPECTED_ATTRIBUTE,
                                                           xml.getCurElementName(), attrib.value_));
                break;
            }

            // Compute the index of the mid parser of the current search range.
            int mid = (min + max) / 2;
            int cmpRes = parsers_[mid].name_.compare(attrib.name_);
            if (cmpRes < 0)
            {
                // The mid parser's name compares lower than the attribute's
                // name, so we can narrow the search range to the part after the
                // mid element.
                min = mid + 1;
            }
            else if (cmpRes > 0)
            {
                // The mid parser's name compares higher than the attribute's
                // name, so we can narrow the search range to the part before
                // the mid element.
                max = mid;
            }
            else
            {
                // The mid parser's name matches the attribute's name, so this
                // is the parser we should use.

                uint32_t mask = 1 << mid;

                // Duplicated attributes aren't allowed in xml, so it would be
                // a code error (of the xml parser) if it successfully parsed
                // xml with duplicated attributes.
                assert(!(visitedMask & mask));

                visitedMask |= mask;
                try
                {
                    parsers_[mid].parseFunc_(attrib.value_.c_str(), result);
                }
                catch (const std::exception&)
                {
                    parsers_[mid].setErrorFunc_(XmlParseError(XmlParseError::Category::INVALID_ATTRIBUTE_VALUE,
                                                              parsers_[mid].name_, attrib.value_),
                                                result);
                }
                break;
            }
        }
    }

    uint32_t fullMask = (1 << static_cast<int>(parsers_.size())) - 1;
    if (visitedMask != fullMask)
    {
        // There are parsers which weren't invoked, so we either have to report
        // an error, or set the default value, depending on whether the parser
        // is marked as required.

        for (int i = 0; i < static_cast<int>(parsers_.size()); i++)
        {
            if (!(visitedMask & (1 << i)))
            {
                if (parsers_[i].required_)
                {
                    parsers_[i].setErrorFunc_(XmlParseError(XmlParseError::Category::MISSING_ATTRIBUTE,
                                                            xml.getCurElementName(), parsers_[i].name_),
                                              result);
                }
                else
                {
                    assert(parsers_[i].setDefaultFunc_);
                    parsers_[i].setDefaultFunc_(result.value());
                }
            }
        }
    }
}

template <class T>
template <class ParseF, class... ParseFailArgs>
void XmlAttributeParsers<T>::addParser(const std::string& name, ParseF&& parseF, ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = true;
    parser.parseFunc_ = parseF;
    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };
    parsers_.push_back(parser);
}

template <class T>
template <class FieldT, class... ParseFailArgs>
void XmlAttributeParsers<T>::addFieldParser(const std::string& name, FieldT T::Value::*fieldPtr,
                                            ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = true;

    parser.parseFunc_ = [fieldPtr](const std::string& value, T& result) {
        result.value().*fieldPtr = xml_parsers::parseXmlAttrib<FieldT>(value);
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };
    parsers_.push_back(parser);
}  // namespace xodr

template <class T>
template <class FieldT, class... ParseFailArgs>
void XmlAttributeParsers<T>::addOptionalFieldParser(const std::string& name, FieldT T::Value::*fieldPtr,
                                                    FieldT defaultValue, ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = false;

    parser.parseFunc_ = [fieldPtr](const std::string& value, T& result) {
        result.value().*fieldPtr = xml_parsers::parseXmlAttrib<FieldT>(value);
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parser.setDefaultFunc_ = [fieldPtr, defaultValue](typename T::Value& res) { res.*fieldPtr = defaultValue; };

    parsers_.push_back(parser);
}

template <class T>
template <class SetterParamT, class... ParseFailArgs>
void XmlAttributeParsers<T>::addSetterParser(const std::string& name, void (T::Value::*setter)(SetterParamT),
                                             ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    using ValueT = typename std::decay<SetterParamT>::type;

    Parser parser;
    parser.name_ = name;
    parser.required_ = true;

    parser.parseFunc_ = [setter, name, parseFailArgs...](const std::string& value, T& result) {
        (result.value().*setter)(xml_parsers::parseXmlAttrib<ValueT>(value));
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class T>
template <class SetterParamT, class... ParseFailArgs>
void XmlAttributeParsers<T>::addOptionalSetterParser(const std::string& name, void (T::Value::*setter)(SetterParamT),
                                                     SetterParamT defaultValue, ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    using ValueT = typename std::decay<SetterParamT>::type;

    Parser parser;
    parser.name_ = name;
    parser.required_ = false;

    parser.parseFunc_ = [setter, name, parseFailArgs...](const std::string& value, T& result) {
        (result.value().*setter)(xml_parsers::parseXmlAttrib<ValueT>(value));
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parser.setDefaultFunc_ = [setter, defaultValue](typename T::Value& obj) { (obj.*setter)(defaultValue); };

    parsers_.push_back(parser);
}

template <class T>
template <class FieldT>
void XmlAttributeParsers<T>::parseField(XmlReader& xml, T& result, const std::string& attribName,
                                        FieldT T::Value::*fieldPtr)
{
    result.value().*fieldPtr = xml_parsers::parseXmlAttrib<FieldT>(xml.getAttribute(attribName));
}

template <class T>
void XmlAttributeParsers<T>::finalize()
{
    std::sort(parsers_.begin(), parsers_.end(), [](const Parser& a, const Parser& b) { return a.name_ < b.name_; });

    optionalMask_ = 0;
    for (int i = 0; i < static_cast<int>(parsers_.size()); i++)
    {
        if (!parsers_[i].required_)
        {
            optionalMask_ |= 1 << i;
        }
    }
}

}}  // namespace aid::xodr
