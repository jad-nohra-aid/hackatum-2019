#pragma once

#include "xml/xml_attribute_parsers.h"
#include "xml/xml_reader.h"

#include <cassert>
#include <assert.h>
#include <map>

#include <boost/variant.hpp>

namespace aid { namespace xodr {

/**
 * Bitmasks of possible XODR violations that we may want to distinguish to deal with partially correct XODR files.
 */
namespace XodrInvalidations {
/** @brief See XodrParseResult::hasValidGeometry to see the meaning of this constant. */
static constexpr unsigned GEOMETRY = (1 << 0);

/** @brief See XodrParseResult::hasValidConnectivity to see the meaning of this constant. */
static constexpr unsigned CONNECTIVITY = (1 << 1);

/** @brief Represents an xodr invalidation that breaks all functionality. */
static constexpr unsigned ALL = GEOMETRY | CONNECTIVITY;
}  // namespace XodrInvalidations

/**
 * @brief An error found while parsing an XODR file.
 *
 * This structure is only used for errors found while parsing the file.
 * No structural analysis is done to find these errors.
 */
class XodrParseError
{
  public:
    /** @brief Variant for the types of errors encountered while parsing XODR.
     *
     * For now, we only care about distinguishing structural XML errors and
     * string error messages. This could be extended to add more type safety.
     */
    using ErrorData = boost::variant<XmlParseError, std::string>;

    XodrParseError() = default;

    /**
     * @brief Constructs an XODR parse error that does not invalidate any
     * properties about the parsed structure.
     *
     * @param data  The data (XmlParseError or message string) to attach to
     * this error object.
     */
    XodrParseError(ErrorData data) : data_(std::move(data)), invalidations_(0) {}

    /**
     * @brief Constructs an XODR parse error.
     *
     * @param data          The data (XmlParseError or message string) to
     *                      attach to this error object.
     * @param invalidations The bitmask of @ref XodrInvalidations given
     *                      by this parse error.
     */
    XodrParseError(ErrorData data, unsigned invalidations) : data_(std::move(data)), invalidations_(invalidations) {}

    std::string description() const;

    /** @brief See XodrParseResult::hasValidGeometry to see the meaning of this check. */
    bool invalidatesRoadGeometry() const { return invalidations_ & XodrInvalidations::GEOMETRY; };

    /** @brief See XodrParseResult::hasValidGeometry to see the meaning of this check. */
    bool invalidatesConnectivity() const { return invalidations_ & XodrInvalidations::CONNECTIVITY; };

    /**
     * @return True if this is a fatal error, false otherwise.
     */
    bool isFatal() const;

  private:
    ErrorData data_;
    unsigned invalidations_;
};

template <class T>
class XodrParseResult : public XmlParseResult<T, XodrParseError>
{
  public:
    using Value = T;
    using Error = XodrParseError;

    XodrParseResult() : XmlParseResult<T, XodrParseError>() {}
    XodrParseResult(T value) : XmlParseResult<T, XodrParseError>(std::move(value)) {}  // NOLINT

    XodrParseResult(T value, XodrParseError warning)
        : XmlParseResult<T, XodrParseError>(std::move(value), std::move(warning))
    {
    }

    XodrParseResult(XodrParseError error) : XmlParseResult<T, XodrParseError>(std::move(error)) {}

    /**
     * @brief Moves the value of the result out of this XodrParseResult, while
     * asserting that the geometry and connectivity of the result are OK.
     *
     * If this assertion is not satisfied, throws runtime_error with the
     * description of the first failure.
     *
     * If this assertion should not be made, one can simply move the value out of
     * this XodrParseResult with std::move(result.value());
     * @return
     */
    T&& extract_value()
    {
        for (const auto& error : XmlParseResult<T, XodrParseError>::errors())
        {
            if (error.invalidatesRoadGeometry() || error.invalidatesConnectivity())
            {
                throw std::runtime_error(error.description());
            }
        }
        return std::move(XmlParseResult<T, XodrParseError>::value());
    }

    XodrParseResult(const XodrParseResult<T>&) = delete;
    XodrParseResult& operator=(const XodrParseResult<T>&) = delete;

    XodrParseResult(XodrParseResult<T>&&) = default;
    XodrParseResult& operator=(XodrParseResult<T>&&) = default;

    /**
     * @brief Checks if all specified roads and lanes in this XodrParseResult
     * have sensible geometry.
     *
     * Sensible geometry means the logic to create a tessellation of the geometry works.
     *
     * @return True if and only if the geometry is good enough to make a tessellation.
     */
    bool hasValidGeometry() const
    {
        return !std::any_of(XmlParseResult<T, XodrParseError>::errors().begin(),
                            XmlParseResult<T, XodrParseError>::errors().end(),
                            [](const XodrParseError& e) { return e.invalidatesRoadGeometry(); });
    }

    /**
     * @brief Checks whether the parsing of connections was succesful.
     *
     * Effectively checks if any of the connections refer to IDs of
     * roads that are not specified and no roads, junctions and have missing IDs.
     *
     * @return True if and only if all specified connected roads exist and no
     * roads have missing connections.
     */
    bool hasValidConnectivity() const
    {
        return !std::any_of(XmlParseResult<T, XodrParseError>::errors().begin(),
                            XmlParseResult<T, XodrParseError>::errors().end(),
                            [](const XodrParseError& e) { return e.invalidatesConnectivity(); });
    }

    /**
     * @brief convenience wrapper over errors() to get the set of errors as string messages.
     */
    std::vector<std::string> errorMessages() const
    {
        std::vector<std::string> ret;
        for (const auto& error : XmlParseResult<T, XodrParseError>::errors())
        {
            ret.push_back(error.description());
        }
        return ret;
    }
};

/**
 * @brief The reader class for xodr files.
 *
 * This provides the xml reading functionality (it derives from XmlReader), but
 * also contains the additional state which is used while parsing xodr files.
 */
class XodrReader : public XmlReader
{
  public:
    /**
     * @brief Creates an XodrReader which parses the given file.
     *
     * @param fileName      The file name of the xodr file.
     * @returns             The XodrReader.
     */
    static XodrReader fromFile(const std::string& fileName);

    /**
     * @brief Creates an XodrReader which parsers the xodr contained in the
     * given string.
     *
     * @param text          A string containing the xodr to parse.
     * @returns             The XodrReader.
     */
    static XodrReader fromText(const std::string& text);

    /**
     * @brief Gets a new global lane index.
     *
     * The function will return 0 the first time it's called, then the return
     * value will increase by 1 each subsequent time it's called.
     */
    int newGlobalLaneIndex() { return nextGlobalLaneIndex_++; }

    /**
     * @brief Peeks the next global lane index.
     *
     * This function will return the same value as @ref newGlobalLaneIndex,
     * but won't increment the counter.
     */
    int peekNextGlobalLaneIndex() const { return nextGlobalLaneIndex_; }

  private:
    XodrReader() = default;

    int nextGlobalLaneIndex_ = 0;
};

struct IdToIndexMaps
{
    std::map<std::string, int> roadIdToIndex_;
    std::map<std::string, int> junctionIdToIndex_;
};

}}  // namespace aid::xodr