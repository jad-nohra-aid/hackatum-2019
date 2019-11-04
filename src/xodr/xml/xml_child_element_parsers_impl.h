#include <sstream>
#include <algorithm>

namespace aid { namespace xodr {

template <class XmlReaderT, class T>
void XmlChildElementParsers<XmlReaderT, T>::parse(XmlReaderT& xml, T& result) const
{
    // If this assert triggers then you probably forgot to call finalize.
    assert(optionalMask_ != (uint32_t)-1);

    uint32_t visitedMask = 0;

    std::string parentName = xml.getCurElementName();
    while (!xml.tryReadEndElement())
    {
        xml.readStartElement();

        // Perform a binary search to find the parser for the element with name xml.getCurElementName().

        // The current search range consists of the parsers with index i
        // satisfying min <= i < max.
        int min = 0;
        int max = static_cast<int>(parsers_.size());

        while (true)
        {
            // If the current search range is empty, then no parser exists for
            // the current element.
            if (min == max)
            {
                // Skip elements which don't have a parser.
                result.errors().emplace_back(XmlParseError(XmlParseError::Category::UNEXPECTED_CHILD_ELEMENT,
                                                           parentName, xml.getCurElementName()));

                xml.skipToEndElement();
                break;
            }

            // Compute the index of the mid parser of the current search range.
            int mid = (min + max) / 2;
            int cmpRes = parsers_[mid].name_.compare(xml.getCurElementName());
            if (cmpRes < 0)
            {
                // The mid parser's name compares lower than the element's name,
                // so we can narrow the search range to the part after the mid element.
                min = mid + 1;
            }
            else if (cmpRes > 0)
            {
                // The mid parser's name compares higher than the element's
                // name, so we can narrow the search range to the part before
                // the mid element.
                max = mid;
            }
            else
            {
                // The mid parser's name matches the element's name, so this is
                // the parser we should use.

                uint32_t mask = 1 << mid;
                if (visitedMask & mask)
                {
                    // We already parsed a similar element, so it depends on
                    // allowMany_ whether we allow this, or whether it's an error.

                    if (!parsers_[mid].allowMany_)
                    {
                        parsers_[mid].setErrorFunc_(XmlParseError(XmlParseError::Category::DUPLICATE_CHILD_ELEMENT,
                                                                  parentName, parsers_[mid].name_),
                                                    result);
                    }
                }
                else
                {
                    visitedMask |= mask;
                }

                parsers_[mid].parseFunc_(xml, result);
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
                    parsers_[i].setErrorFunc_(
                        XmlParseError(XmlParseError::Category::MISSING_CHILD_ELEMENT, parentName, parsers_[i].name_),
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

template <class XmlReaderT, class T>
template <class ParseF, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addParser(const std::string& name, Multiplicity multiplicity,
                                                      ParseF&& parse, ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = multiplicity == Multiplicity::ONE || multiplicity == Multiplicity::ONE_OR_MORE;
    parser.allowMany_ = multiplicity == Multiplicity::ZERO_OR_MORE || multiplicity == Multiplicity::ONE_OR_MORE;
    parser.parseFunc_ = std::move(parse);
    parser.setDefaultFunc_ = [](typename T::Value&) {};
    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };
    parsers_.push_back(std::move(parser));
}

template <class XmlReaderT, class T>
template <class FieldT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addFieldParser(const std::string& name,
                                                           typename FieldT::Value T::Value::*fieldPtr,
                                                           ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = true;
    parser.allowMany_ = false;

    parser.parseFunc_ = [fieldPtr](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, FieldT>(xml);
        result.value().*fieldPtr = std::move(childParseRes.value());
        result.appendErrors(childParseRes);
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
template <class FieldT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addOptionalFieldParser(const std::string& name,
                                                                   typename FieldT::Value T::Value::*fieldPtr,
                                                                   typename FieldT::Value defaultValue,
                                                                   ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = false;
    parser.allowMany_ = false;

    parser.parseFunc_ = [fieldPtr](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, FieldT>(xml);
        result.value().*fieldPtr = std::move(childParseRes.value());
        result.appendErrors(childParseRes);
    };

    parser.setDefaultFunc_ = [fieldPtr, defaultValue](typename T::Value& obj) { obj.*fieldPtr = defaultValue; };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
template <class FieldT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addOptionalFieldParser(
    const std::string& name, boost::optional<typename FieldT::Value> T::Value::*fieldPtr,
    ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = false;
    parser.allowMany_ = false;

    parser.parseFunc_ = [fieldPtr](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, FieldT>(xml);
        result.value().*fieldPtr = std::move(childParseRes.value());
        result.appendErrors(childParseRes);
    };

    parser.setDefaultFunc_ = [fieldPtr](typename T::Value& obj) { obj.*fieldPtr = boost::none; };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
template <class ElemT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addVectorElementParser(
    const std::string& name, std::vector<typename ElemT::Value> T::Value::*vectorPtr, Multiplicity multiplicity,
    ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = multiplicity == Multiplicity::ONE || multiplicity == Multiplicity::ONE_OR_MORE;
    parser.allowMany_ = multiplicity == Multiplicity::ZERO_OR_MORE || multiplicity == Multiplicity::ONE_OR_MORE;

    parser.parseFunc_ = [vectorPtr](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, ElemT>(xml);
        (result.value().*vectorPtr).push_back(std::move(childParseRes.value()));
        result.appendErrors(childParseRes);
    };

    parser.setDefaultFunc_ = [](typename T::Value&) {};

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
template <class ValueT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addSetterParser(const std::string& name,
                                                            void (T::Value::*setter)(typename ValueT::Value),
                                                            ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = true;
    parser.allowMany_ = false;
    parser.parseFunc_ = [setter](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, ValueT>(xml);
        (result.value().*setter)(std::move(childParseRes.value()));
        result.appendErrors(childParseRes);
    };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
template <class ValueT, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::addOptionalSetterParser(const std::string& name,
                                                                    void (T::Value::*setter)(typename ValueT::Value),
                                                                    typename ValueT::Value defaultValue,
                                                                    ParseFailArgs... parseFailArgs)
{
    assert(parsers_.size() <= 31);

    Parser parser;
    parser.name_ = name;
    parser.required_ = false;
    parser.allowMany_ = false;
    parser.parseFunc_ = [setter](XmlReaderT& xml, T& result) {
        auto childParseRes = xml_parsers::parseXmlElem<XmlReaderT, ValueT>(xml);
        (result.value().*setter)(std::move(childParseRes.value()));
        result.appendErrors(childParseRes);
    };
    parser.setDefaultFunc_ = [setter, defaultValue](typename T::Value& obj) { (obj.*setter)(defaultValue); };

    parser.setErrorFunc_ = [parseFailArgs...](XmlParseError parseError, T& result) {
        result.errors().emplace_back(std::move(parseError), parseFailArgs...);
    };

    parsers_.push_back(parser);
}

template <class XmlReaderT, class T>
void XmlChildElementParsers<XmlReaderT, T>::finalize()
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

template <class XmlReaderT, class T>
template <class Func, class... ParseFailArgs>
void XmlChildElementParsers<XmlReaderT, T>::parseOneOrMore(XmlReaderT& xml, T& result, const std::string& elemName,
                                                           Func&& func, ParseFailArgs... parseFailArgs)
{
    bool atLeastOne = false;
    std::string parent = xml.getCurElementName();
    while (!xml.tryReadEndElement())
    {
        xml.readStartElement();

        if (xml.getCurElementName() == elemName)
        {
            func(xml, result);
            atLeastOne = true;
        }
        else
        {
            xml.skipToEndElement();
        }
    }

    if (!atLeastOne)
    {
        result.errors().emplace_back(XmlParseError(XmlParseError::Category::MISSING_CHILD_ELEMENT, parent, elemName),
                                     parseFailArgs...);
    }
}

}}  // namespace aid::xodr
