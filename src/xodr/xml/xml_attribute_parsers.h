#pragma once

#include <string>
#include <functional>

#include "xml_parse_result.h"
#include "xml_reader.h"

namespace aid { namespace xodr {
namespace xml_parsers {
/**
 * @brief The function which is used to parse attribute values into objects
 * of the required type.
 *
 * To add parsing support for your own types, simply create a new template
 * specialization.
 */
template <class T>
T parseXmlAttrib(const std::string& value);

template <>
int parseXmlAttrib<int>(const std::string& value);

template <>
double parseXmlAttrib<double>(const std::string& value);

template <>
std::string parseXmlAttrib<std::string>(const std::string& value);

template <>
bool parseXmlAttrib<bool>(const std::string& value);

}  // namespace xml_parsers

/**
 * @brief An XmlAttributeParsers is a container for parsers of the attributes
 * of a certain element type. It contains all the information needed to parse
 * the attribute of this xml element type.
 *
 * Before an XmlAttributeParsers object can be used it has to be initialized by
 * having parsers for the individual attributes added to it. This is done using
 * the various add***Parser functions. Each parser consists of an attribute name
 * and the information needed to parse such an attribute when it's encountered.
 * After all parsers have been added, the finalize() function should be called
 * to make the XmlAttributeParsers ready for use.
 *
 * Then to use it, simply call the parse method with an XmlReader and a target object.
 *
 * By default, an attribute is non optional, which means that an exception is
 * thrown if the attribute isn't present in the xml element the @ref parse
 * function is trying to parse.
 *
 * If an attribute parser is optional, then the attribute may be ommitted from
 * the xml element, without resulting in a parsing failure.
 *
 * Usually, you will use a static XmlAttributeParsers which is initialized once
 * and reused each time you want to parse an xml element of the same type.
 *
 * @tparam The type of the target object.
 */
template <class T>
class XmlAttributeParsers
{
  public:
    /**
     * @brief Parses the attributes of the given XmlReader's current element
     * and stores the result in the given result.
     */
    void parse(XmlReader& xml, T& result) const;

    /**
     * @brief Adds a parser for attributes with the given name, which uses the
     * user provided function to parse the value of the attribute.
     *
     * @param name          The attribute name.
     * @param               A parser functor. This functor must have the
     *                      signature void(const std::string& value, T& obj);
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class ParseF, class... ParseFailArgs>
    void addParser(const std::string& name, ParseF&& parseF, ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds a parser for attributes with the given name, which parses the
     * value using the xml_parsers::parseXmlAttrib function and stores the
     * result in the given field.
     *
     * The parseXmlAttrib function is templated, so you can add new
     * specializations for your own types. The default implementation forwards
     * to T::parse.
     *
     * @param name          The attribute name.
     * @param fieldPtr      Pointer to the field where the result should be stored.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class FieldT, class... ParseFailArgs>
    void addFieldParser(const std::string& name, FieldT T::Value::*fieldPtr, ParseFailArgs... parseFailArgs);

    /**
     * @brief An optional field parser.
     *
     * This function is similar to @ref addFieldParser, but allows the attribute
     * to be ommitted from the xml element. If it's ommitted then the field will
     * be set to the given @p defaultValue instead.
     *
     * @param name          The attribute name.
     * @param fieldPtr      Pointer to the field where the result should be stored.
     * @param defaultValue  The default value. This is the value the field will
     *                      be set to when the attribute isn't specified.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class FieldT, class... ParseFailArgs>
    void addOptionalFieldParser(const std::string& name, FieldT T::Value::*fieldPtr, FieldT defaultValue,
                                ParseFailArgs... parseFailArgs);

    /**
     * @brief Adds a parser for attributes with the given name which parses the
     * value using the xml_parsers::parseXmlAttribu function and stores the
     * result using the given setter function.
     *
     * The parseXmlAttrib is a templated function, so you can add new
     * specializations for your own types. The default implementation forwards
     * to T::parse.
     *
     * @param name          The attribute name.
     * @param setter        Pointer to the member function to set the value in
     *                      the target object.
     * @param parseFailArgs Any arguments that should be passed to the
     *                      constructor of T::Error() on parser failure.
     *                      The first argument will always be the XmlParseError
     *                      object that triggered the error.
     */
    template <class SetterParamT, class... ParseFailArgs>
    void addSetterParser(const std::string& name, void (T::Value::*setter)(SetterParamT value),
                         ParseFailArgs... parseFailArgs);

    /**
     * @brief An optional setter parser.
     *
     * This function is similar to @ref addSetterParser, but allows the
     * attribute to be ommitted from the xml element. If it's ommitted then the
     * setter will be called with the given @p defaultValue instead.
     */
    template <class SetterParamT, class... ParseFailArgs>
    void addOptionalSetterParser(const std::string& name, void (T::Value::*setter)(SetterParamT value),
                                 SetterParamT defaultValue, ParseFailArgs... parseFailArgs);

    /**
     * @brief Finalizes the initialization phase of this XmlAttributeParsers<T>.
     *
     * This function must be called after the last parser has been added, and
     * before the @ref parse function is used.
     */
    void finalize();

    /**
     * @brief A utility function which allows you to parse an xml element with a
     * single attribute (that is, a single attribute of interest, since the
     * parser ignores attributes it doesn't recognize).
     *
     * This is functionally equivalent to the following:
     *
     *   XmlAttributeParsers<T> parsers;
     *   parsers.addFieldParser(attribName, fieldPtr);
     *   parsers.finalize();
     *   parsers.parse(xml, obj);
     */
    template <class FieldT>
    static void parseField(XmlReader& xml, T& result, const std::string& attribName, FieldT T::Value::*fieldPtr);

  private:
    using ParseFunc = std::function<void(const std::string& value, T&)>;
    using SetDefaultFunc = std::function<void(typename T::Value&)>;
    using SetErrorFunc = std::function<void(XmlParseError error, T&)>;

    /**
     * Information for a single attribute parser.
     */
    struct Parser
    {
        /**
         * The attribute name.
         */
        std::string name_;

        /**
         * Whether the attribute is required or optional.
         */
        bool required_;

        /**
         * The parser function.
         *
         * This function is called when an attribute with this parser's name
         * is encountered. It's passed the attribute value and the target object
         * and is in turn responsible for parsing the value and storing the
         * result in the target object.
         */
        ParseFunc parseFunc_;

        /**
         * A function which should is applied to the object if the attribute
         * was missing.
         *
         * This function is only relevant for optional attribute parsers, since
         * setErrorFunc_ will be called if the attribute was not optional.
         */
        SetDefaultFunc setDefaultFunc_;

        /**
         * A function which should be applied to the object if the attribute
         * was missing or an exception was thrown while parsing.
         */
        SetErrorFunc setErrorFunc_;
    };

    /**
     * A bit-mask which specifies whether a parser is optional. The bit at
     * index i corresponds to the parser with index i in the parsers_ list.
     *
     * This value is computed by the finalize() function.
     */
    uint32_t optionalMask_;

    /**
     * A list of all parsers.
     *
     * This list is sorted by name in the finalize function, to allow for binary
     * searches on element names.
     */
    std::vector<Parser> parsers_;
};

}}  // namespace aid::xodr

#include "xml_attribute_parsers_impl.h"