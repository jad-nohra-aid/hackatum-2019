#pragma once

#include <functional>
#include <vector>
#include <boost/optional.hpp>

#include "xml_parse_result.h"
#include "xml_reader.h"

namespace aid { namespace xodr {

namespace xml_parsers {
/**
 * @brief Parsers an object of the given type using the given XmlReader.
 *
 * When called, the current node of the XmlReader must be the start tag of
 * the element to parse, on return the current node will be the end tag of
 * this element.
 *
 * This is a templated function, so it's possible to create specializations
 * for user types.
 *
 * The default implementation calls T::parseXml. This function is assumed to
 * have the same semantics as this function.
 *
 * @returns The
 */
template <class XmlReaderT, class T>
T parseXmlElem(XmlReaderT& xml)
{
    return T::Value::parseXml(xml);
}
}  // namespace xml_parsers

/**
 * @brief An XmlChildElementParsers is a container for parsers for the child
 * elements of a certain element type. It contains all the information needed to
 * parse the child elements of this xml element type.
 *
 * Before an XmlChildElementParsers object can be used it has to be initialized
 * by having parsers for the individual child element types added to it. This is
 * done using the various add***Parser functions. Each parser consists of an
 * element name and the information needed to parse such an element when it's
 * encountered. After all parsers have been added, the finalize() function
 * should be called to make the XmlChildElementParsers ready for use.
 *
 * Child elements parsers can be of a certain multiplicity, where the
 * multiplicity specifies how often a type of child element may occur
 * (see @ref Multiplicity). If the multiplicity requirement isn't met, then
 * the parse function will throw an exception.
 *
 * To use XmlChildElementParsers, simply call the parse method with an XmlReader
 * and a target object.
 *
 * Usually, you will use a static XmlChildElementParsers which is initialized
 * once and reused each time you want to parse an xml element of the same type.
 *
 * @tparam The result type of the target object. This should be a type similar to
 * @ref XmlParseResult with typedefs for T::Value and T::Error to represent the
 * value type for the parsed object and error type respectively.
 */
template <class XmlReaderT, class T>
class XmlChildElementParsers
{
  public:
    /**
     * @brief Parses the child elements of the XmlReader's current element
     * and stores the result in the given target object.
     *
     * @param xml           The XmlReader which provides the xml to parse.
     *                      On entry, this XmlReader's current node must be the
     *                      start tag of the element whose children we're
     *                      interested in. On return, the current node will be
     *                      the end tag of this same element.
     * @param result        The target object in which the result should be stored.
     */
    void parse(XmlReaderT& xml, T& result) const;

    /**
     * @brief The Multiplicity for a certain type of child element.
     */
    enum class Multiplicity
    {
        /**
         * @brief The type of child element may occur zero or one times.
         */
        ZERO_OR_ONE,

        /**
         * @brief The type of child element has to occur exactly once.
         */
        ONE,

        /**
         * @brief The type of child element is optional, and may be repeated an
         * unlimited number of times.
         */
        ZERO_OR_MORE,

        /**
         * @brief The type of child element has to occur at least once, but
         * may be repeated an unlimited number of times.
         */
        ONE_OR_MORE,
    };

    /**
     * @brief Adds a parser for child elements with the given name.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param multiplicity  The multiplicity requirement.
     * @param parseF        The parse function. This function will be invoked
     *                      by the XmlReader::parse function each time it
     *                      encounters a child element with the given name.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class ParseF, class... ParseFailArgs>
    void addParser(const std::string& name, Multiplicity multiplicity, ParseF&& func, ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds a parser which parses the child element with the given name
     * using the xml_parsers::parseXmlElem() function and stores the result in
     * the given field.
     *
     * The parseXmlElem function is templated, so you can add new
     * specializations for your own types. The default implementation forwards
     * to T::parseXml.
     *
     * A parser added with this function will have multiplicity Multiplicity::ONE.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param fieldPtr      The field to store the result in.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class FieldT, class... ParseFailArgs>
    void addFieldParser(const std::string& name, typename FieldT::Value T::Value::*fieldPtr,
                        ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds an optional field parser.
     *
     * This function is similar to @ref addFieldParser, with the difference that
     * the child element is optional (so the multiplicity is
     * Multiplicity::ZERO_OR_ONE). The XmlReader::parse function will assign
     * defaultValue to the target field if no child element with the given name
     * is found.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param fieldPtr      The field to store the result in.
     * @param defaultValue  The default value to assign to the target field in
     *                      case no child element with the given name is found.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class FieldT, class... ParseFailArgs>
    void addOptionalFieldParser(const std::string& name, typename FieldT::Value T::Value::*fieldPtr,
                                typename FieldT::Value defaultValue, ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds an optional field parser for a boost::optional field.
     *
     * This function is similar to the above addOptionalFieldParser, but with
     * the difference that it stores the parsed object in a field of type
     * boost::optional<FieldT::Value>. If the object isn't specified, the field
     * will be set to boost::none.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param fieldPtr      The field to store the result in.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class FieldT, class... ParseFailArgs>
    void addOptionalFieldParser(const std::string& name, boost::optional<typename FieldT::Value> T::Value::*fieldPtr,
                                ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds a parser which parses the child element with the given name
     * using the xml_parsers::parseXmlElem() function and stores in the target
     * object using the given setter function.
     *
     * The parseXmlElem function is templated, so you can add new
     * specializations for your own types. The default implementation forwards
     * to T::parseXml.
     *
     * A parser added with this function will have multiplicity Multiplicity::ONE.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param setter        Pointer to the member function to set the value in
     *                      the target object.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class ValueT, class... ParseFailArgs>
    void addSetterParser(const std::string& name, void (T::Value::*setter)(typename ValueT::Value value),
                         ParseFailArgs... parseFailArgs);

    /**
     * @brief An optional setter parser.
     *
     * This function is similar to @ref addSetterParser, with the difference
     * that the child element is optional (so the multiplicity is
     * Multiplicity::ZERO_OR_ONE). The parse function will call the setter with
     * defaultValue if no child element with the given name is found.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param setter        Pointer to the member function to set the value in
     *                      the target object.
     * @param defaultValue  The default value to use when no child element with
     *                      the given name is found.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class ValueT, class... ParseFailArgs>
    void addOptionalSetterParser(const std::string& name, void (T::Value::*setter)(typename ValueT::Value value),
                                 typename ValueT::Value defaultValue, ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds a parser which parses each child element with the given name
     * using the xml_parsers::parseXmlElem() function and appends the resulting
     * value to a vector in the target object.
     *
     * @param name          The name of the child elements to be parsed with
     *                      this parser.
     * @param vectorPtr     Pointer to the vector member of the target type.
     * @param multiplicity  The multiplicity (generally Multiplicity::ZERO_OR_MORE
     *                      or Multiplicity::ONE_OR_MORE).
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class ElemT, class... ParseFailArgs>
    void addVectorElementParser(const std::string& name, std::vector<typename ElemT::Value> T::Value::*vectorPtr,
                                Multiplicity multiplicity, ParseFailArgs... parseFailArgs);

    /**
     * @brief Finalizes the initialization phase of this XmlAttributeParsers<T>.
     *
     * This function must be called after the last parser has been added, and
     * before the @ref parse function is used.
     */
    void finalize();

    /**
     * @brief A utility function which allows you to parse an xml element where
     * we're only interested in child elements of a single type.
     *
     * This is functionally equivalent to the following:
     *
     *   XmlChildElementParsers<T> parsers;
     *   parsers.addParser(elemName, Multiplicity::ONE_OR_MORE, func,
     *                     parseFailArgs);
     *   parsers.finalize();
     *   parsers.parse(xml, obj);
     */
    template <class Func, class... ParseFailArgs>
    static void parseOneOrMore(XmlReaderT& xml, T& ret, const std::string& elemName, Func&& func,
                               ParseFailArgs... parseFailArgs);

  private:
    /**
     * @brief A parse function for a child element.
     *
     * This function is called with an XmlReader whose current node is the start
     * tag of the child element to parse. It's the responsibility of the parse
     * function to advance the XmlReader to the end element tag of this
     * child (so a valid parse function would be one which simply calls
     * skipToEndElement()).
     *
     * The second parameter is the target object. This is the object where the
     * result should be stored in.
     */
    using ParseFunc = std::function<void(XmlReaderT&, T&)>;

    /**
     * @brief A function which sets a default value in a target object.
     *
     * A SetDefaultFunc is only relevant for parsers of optional child elements
     * (that is, those with multiplicity ZERO_OR_ONE or ZERO_OR_MORE). If it's
     * specified for a certain parser, then it will be invoked by the parse
     * function if no child element with the parser's name is found.
     */
    using SetDefaultFunc = std::function<void(typename T::Value&)>;

    /**
     * @brief A function which marks a parse result as failure.
     */
    using SetErrorFunc = std::function<void(XmlParseError error, T& result)>;

    struct Parser
    {
        std::string name_;
        bool required_;
        bool allowMany_;

        ParseFunc parseFunc_;
        SetDefaultFunc setDefaultFunc_;
        SetErrorFunc setErrorFunc_;
    };

    /**
     * A bit-mask which specifies whether a parser is optional. The bit at
     * index i corresponds to the parser with index i in the parsers_ list.
     *
     * This value is computed by the finalize() function.
     */
    uint32_t optionalMask_ = (uint32_t)-1;

    /**
     * A list of all parsers.
     *
     * This list is sorted by name in the finalize function, to allow for binary
     * searches on element names.
     */
    std::vector<Parser> parsers_;
};

}}  // namespace aid::xodr

#include "xml_child_element_parsers_impl.h"
