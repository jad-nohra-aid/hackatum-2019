#pragma once

#include "xodr_reader.h"

#include <map>
#include <string>

namespace aid { namespace xodr {

/**
 * @brief An XodrObjectReference is a reference from one object to another
 * object in an xodr file (for example, the reference to the successor in a Road).
 *
 * In the xodr file, references are specified using object ID's.
 * An XodrObjectReference provides this ID, but also provides an index
 * into the relevant array's in the XodrMap, which can be used to quickly access
 * the target object.
 *
 * To resolve these indices, a separate 'resolve' pass is needed, which should
 * be run after the whole XodrMap is parsed. See @ref resolve.
 */
class XodrObjectReference
{
  public:
    XodrObjectReference() = default;

    /**
     * @brief Constructs an object reference with the given id and index.
     *
     * @brief id            The id of the target object.
     * @brief index         The index of the target object.
     */
    XodrObjectReference(const std::string& id, int index) : id_(id), index_(index) {}

    /**
     * @brief Parses the given text (usually an attribute value) into an XodrObjectReference.
     *
     * The text is simply interpreted as an id.
     *
     * @returns             The XodrObjectReference.
     */
    static XodrParseResult<XodrObjectReference> parse(const std::string& txt);

    /**
     * @brief Returns whether the ID of this reference is equal to the given value.
     */
    bool operator==(const std::string& b) const;

    /**
     * @brief Returns whether the ID of this reference is different from the given value.
     */
    bool operator!=(const std::string& b) const;

    /**
     * @returns The id of the target object.
     */
    const std::string& id() const { return id_; }

    /**
     * @brief Returns true if this XodrObjectReference refers to a valid object,
     * false if it's null.
     *
     * An XodrObjectReference is null if the ID has the value of the 'nullValue'
     * passed to the resolve() function.
     *
     * This function should only be used after the resolve() has been called.
     *
     * @returns True if it refers to a valid object, false otherwise.
     */
    bool hasValue() const;

    /**
     * Returns the index of the target object.
     *
     * This function should only be used after the resolve() has been called,
     * and when hasValue() is true.
     *
     * @returns The index of the target object.
     */
    int index() const;

    /**
     * @brief Resolves the index of this XodrObjectReference.
     *
     * The given idToIndex mapping maps object ID's to their indices, and should
     * have an entry for each object of the target type in the XodrMap.
     *
     * If the id specified in this XodrObjectReference isn't found in the map,
     * then an exception is thrown.
     *
     * @param idToIndex     The mapping from object id's to object indices.
     * @param objTypeName   The type name of the target object. This name used
     *                      in the error message of the exception which is
     *                      thrown when the reference can't be resolved.
     */
    void resolve(const std::map<std::string, int>& idToIndex, const std::string& objTypeName);

    /**
     * @brief An overload of resolve() which supports a null value.
     *
     * @param idToIndex     The mapping from object id's to object indices.
     * @param nullValue     The ID value which indicates that this reference is null.
     * @param objTypeName   The type name of the target object. This name used
     *                      in the error message of the exception which is
     *                      thrown when the reference can't be resolved.
     */
    void resolve(const std::map<std::string, int>& idToIndex, const std::string& nullValue,
                 const std::string& objTypeName);

  private:
    static const int INVALID_VALUE = -2;
    static const int NULL_VALUE = -1;

    std::string id_;
    int index_ = INVALID_VALUE;
};

}}  // namespace aid::xodr