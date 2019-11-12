#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <iostream>

namespace aid { namespace xodr {

/**
 * @brief An error that could appear while parsing an XML document.
 *
 * This is intended to be ignorant of semantics other than the set of attributes
 * and child elements and multiplicities a given XML element should have.
 */
class XmlParseError
{
  public:
    enum class Category
    {
        /** @brief an attribute was specified as non-optional but is missing. */
        MISSING_ATTRIBUTE,
        /** @brief an attribute was not expected by the XmlAttributeParser. */
        UNEXPECTED_ATTRIBUTE,
        /** @brief a child element was specified as non-optional but is missing. */
        MISSING_CHILD_ELEMENT,
        /** @brief an child element was not expected by the XmlChildElementParser. */
        UNEXPECTED_CHILD_ELEMENT,
        /** @brief more than one instance of a child element was found, but this was not allowed. */
        DUPLICATE_CHILD_ELEMENT,
        /** @brief The value of an attribute was such that it could not be parsed into the expected type. */
        INVALID_ATTRIBUTE_VALUE,
    };

    /**
     * @brief Constructs an XmlParseError.
     *
     * @param category      The @ref Category of this error.
     * @param name          The attribute name or (parent) element name.
     * @param value         The attribute value or (child) element name.
     */
    XmlParseError(Category category, std::string name, std::string value)
        : category_(category), name_(std::move(name)), value_(std::move(value))
    {
    }

    /**
     * @brief Gets a human-readable description of this error.
     * @return a human-readable description of this error.
     */

    std::string description() const;

	/**
     * @return True if this is a fatal error, false otherwise.
     */
    bool isFatal() const;

    Category category_;
    std::string name_;
    std::string value_;
};

/**
 * @brief The result of parsing some XML structure.
 *
 * @tparam T    The type of the parsed document.
 * @tparam E    The error type as defined by the user of the XML parser. It
 *              should at least have a no-argument constructor and a
 *              constructor that accepts an XmlParseError to handle structural
 *              errors. Additional constructors and data can be defined for
 *              application specific errors.
 */
template <typename T, typename E = XmlParseError>
class XmlParseResult
{
  public:
    using Value = T;
    using Error = E;

    /**
     * Creates an empty XmlParseResult.
     */
    XmlParseResult() = default;

    /**
     * Creates a valid XmlParseResult by moving @param value.
     */
    XmlParseResult(Value value)  // NOLINT: Implicit conversion is OK to handle the simple success case
        : value_(std::move(value)), errors_{}
    {
    }

    /**
     * Creates a valid XmlParseResult with a warning message attached.
     *
     * @param value     The value to be returned.
     * @param warning   The attached warning message.
     */
    XmlParseResult(Value value, E warning) : value_(std::move(value)), errors_{warning} {}

    /**
     * @brief Creates an XmlParseResult representing an error.
     *
     * @param error
     */
    XmlParseResult(E error) : value_{}, errors_{error} {}

    XmlParseResult(const XmlParseResult<T, E>&) = delete;
    XmlParseResult& operator=(const XmlParseResult<T, E>&) = delete;

    XmlParseResult(XmlParseResult<T, E>&&) = default;
    XmlParseResult& operator=(XmlParseResult<T, E>&&) = default;

    template <class ChildT>
    void appendErrors(ChildT& childParseResult)
    {
        std::move(childParseResult.errors().begin(), childParseResult.errors().end(), std::back_inserter(errors()));
    }

    /**
     * @brief Gets a mutable reference to the value stored in this result object.
     *
     * There are two use cases for this method:
     *  - Modifying the value stored in the result
     *  - Moving the value out of the result
     *
     * @return A mutable reference to the value stored in this result object.
     */
    Value& value() { return value_; }

    /**
     * @brief Gets a const reference to the value stored in this result object.
     * @return A const reference to the value stored in this result object.
     */
    const Value& value() const { return value_; }

    std::vector<E>& errors() { return errors_; }

    const std::vector<E>& errors() const { return errors_; }

    /**
     * @return True if this XmlParseResult contains at least 1 fatal error,
     * false otherwise.
     */
    bool hasFatalErrors() const
	{
    	for(const E& err : errors_)
		{
    		if(err.isFatal())
			{
    			return true;
			}
		}

    	return false;
	}

  protected:
    Value value_ = Value();

    std::vector<E> errors_;
};
}}  // namespace aid::xodr