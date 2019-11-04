#pragma once

#include <string>
#include <vector>

#include "tinyxml.h"

namespace aid { namespace xodr {

/**
 * @brief An XmlReader is a stream based xml parser.
 *
 * An xml file is treated as a sequence of nodes. The various functions in the
 * in the XmlReader allow you to go through these nodes in a sequential manner.
 */
class XmlReader
{
  public:
    ~XmlReader() = default;

    /**
     * @brief Creates an XmlReader which parses the given xml file.
     *
     * @param fileName      The file name of the xml file.
     * @returns             The XmlReader.
     */
    static XmlReader fromFile(const std::string& fileName);

    /**
     * @brief Creates an XmlReader which parsers the xml contained in the given string.
     *
     * @param text          A string containing the xml to parse.
     * @returns             The XmlReader.
     */
    static XmlReader fromText(const std::string& text);

    /**
     * @brief Reads the start tag of an element.
     *
     * An exception is thrown if the next node isn't a start element.
     */
    void readStartElement();

    /**
     * @brief Reads the start tag of an element, while checking that the element
     * has the expected name.
     *
     * An exception is thrown if the next node isn't a start tag, or if it's a
     * start tag of an element with a name other than expectedName.
     *
     * The XmlReader remains unchanged in case of an exception, so it's possible
     * to continue using it after the exception has been thrown.
     *
     * Note that an empty-element tag is treated as a start tag followed by an
     * end tag, so you could still use this function, together with
     * @ref readEndElement to read empty-element tags.
     *
     * @param expectedName  The expected name of the start element.
     */
    void readStartElement(const std::string& expectedName);

    /**
     * @brief Reads the end tag of an element.
     *
     * An exception is thrown if the next node isn't an end tag.
     */
    void readEndElement();

    /**
     * @brief Skips to the end tag of of the current element.
     *
     * This function skips over all remaining child nodes and makes the current
     * node the end tag of the current element.
     */
    void skipToEndElement();

    /**
     * @brief Tries to read the start tag of an element.
     *
     * If the next node is a start tag, then this node will become the current
     * node and true is returned. If the next node isn't a start element, false
     * is returned and the the XmlReader remains unchanged.
     *
     * @returns             True if the read was successful, false otherwise.
     */
    bool tryReadStartElement();

    /**
     * @brief Tries to read the start tag of an element, while checking that the
     * element has the expected name.
     *
     * If the next node is the start tag of an element with the given name, then
     * this node is read, and 'true' is returned. In all other cases, false is
     * returned and the XmlReader remains unchanged.
     *
     * @param expectedName  The expected name of the start element.
     * @returns             True if the read was successful, false otherwise.
     */
    bool tryReadStartElement(const std::string& expectedName);

    /**
     * @brief Tries to read the end tag of the current element.
     *
     * If the next node is the end tag, then this node is read, and 'true'
     * is returned. In all other cases, false is returned and the XmlReader
     * remains unchanged.
     *
     * @returns             True if the read was successful, false otherwise.
     */
    bool tryReadEndElement();

    /**
     * @brief Gets the name of the current element.
     *
     * This function should only be called when the current node is the start
     * tag of an element. It's the responsibility of the caller to make sure
     * this is the case.
     *
     * @returns             The name of the current element.
     */
    std::string getCurElementName() const;

    /**
     * @brief A attribute's name/value pair.
     */
    struct Attrib
    {
        /**
         * @brief The attribute name.
         */
        std::string name_;

        /**
         * @brief The attribute value.
         */
        std::string value_;
    };

    /**
     * @brief Gets all attributes associated with the current element.
     *
     * This function should only be called when the current node is the start
     * tag of an element. It's the responsibility of the caller to make sure
     * this is the case.
     *
     * @returns             A vector containing the attributes.
     */
    std::vector<Attrib> getAttributes() const;

    /**
     * @brief Gets the value of the attribute with the given name.
     *
     * An exception is thrown if the current element doesn't have an attribute
     * with this name.
     *
     * This function should only be called when the current node is the start
     * tag of an element. It's the responsibility of the caller to make sure
     * this is the case.
     *
     * @returns             The value of the attribute.
     */
    std::string getAttribute(const std::string& name) const;

    /**
     * @brief Gets the text contained in this element.
     *
     * This function expects that the current element only contains text in it's
     * body, and no child elements. An exception is thrown if this isn't the case.
     *
     * This function should only be called when the current node is the start
     * tag of an element. It's the responsibility of the caller to make sure
     * this is the case.
     *
     * @returns             The text contained in this element.
     */
    std::string getText() const;

    /**
     * @returns The line number of the current location in the xml document.
     */
    int getLineNumber() const;

    /**
     * @returns The column number of the current location in the xml document.
     */
    int getColumnNumber() const;

  protected:
    XmlReader() = default;

    /**
     * @brief Initializes this XmlReader to parse xml from a file.
     *
     * The purpose of this function is to allow derived classes to implement
     * functions similar to @ref fromFile.
     *
     * @param fileName      The file name of the xml file.
     */
    void initFromFile(const std::string& fileName);

    /**
     * @brief Initializes this XmlReader to parsers the xml contained in the
     * given string.
     *
     * The purpose of this function is to allow derived classes to implement
     * functions similar to @ref fromText.
     *
     * @param fileName      The file name of the xml file.
     */
    void initFromText(const std::string& text);

  private:
    TiXmlDocument doc_;
    TiXmlElement* curElement_;
    bool endOfElement_;
};

}}  // namespace aid::xodr
