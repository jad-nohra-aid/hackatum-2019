#pragma once

#include "xodr_reader.h"

#include <climits>
#include <cassert>

namespace aid { namespace xodr {

class LaneID;
class LaneIDOpt;

/**
 * @brief A lane identifier.
 *
 * A lane identifier is an integer, but since we use both lane id's and lane
 * indices throughout this module, it's convenient to have a strongly typed ID
 * class, so that the compiler will catch cases where id's are used in places
 * where an index is expected, and vice versa.
 */
class LaneID
{
    friend class LaneIDOpt;

  private:
    /**
     * @brief The internal value a LaneIDOpt uses to indicate that it's not specified.
     *
     * A LaneID should never have this value (because any LaneID should be
     * convertible to a LaneIDOpt). The only reason why we need it here is for
     * error checking in the parsing functions.
     */
    static constexpr int NULL_TAG = std::numeric_limits<int>::max();

    /**
     * @brief The interval value which is used in debug mode to indicates that
     * a LaneID is uninitialized.
     */
    static constexpr int INVALID_TAG = std::numeric_limits<int>::max() - 1;

  public:
    /**
     * @brief Constructs an uninitialized LaneID.
     *
     * The value of an uninitialized LaneID is undefined, so a value needs to
     * be assigned to it before this LaneID becomes usable.
     *
     * It's an error to use an uninitialized LaneID for anything other than the
     * destination of an assignment.
     */
    LaneID()
    {
#ifndef NDEBUG
        id_ = INVALID_TAG;
#endif
    }

    /**
     * @brief Constructs a LaneID with the given value.
     */
    explicit LaneID(int id) : id_(id) {}

    /**
     * @brief The copy constructor
     */
    LaneID(const LaneID& src)
    {
        assert(src.isValid());
        id_ = src.id_;
    }

    /**
     * @brief Conversion operator to get the value of this ID as an int.
     */
    explicit operator int() const
    {
        assert(isValid());
        return id_;
    }

    /**
     * @brief Parses a LaneID from the given string.
     */
    static XodrParseResult<LaneID> parse(const std::string& s)
    {
        LaneID ret;

        ret.id_ = std::stoi(s);
        if (ret.id_ >= INVALID_TAG)
        {
            throw std::out_of_range("Lane identifier out of range");
        }

        return ret;
    }

    /**
     * @brief Compares the two LaneID's for equality.
     */
    bool operator==(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ == b.id_;
    }

    /**
     * @brief Compares the two LaneID's for inequality.
     */
    bool operator!=(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ != b.id_;
    }

    /**
     * @brief Checks whether the value of this LaneID is less than the given value.
     */
    bool operator<(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ < b.id_;
    }

    /**
     * @brief Checks whether the value of this LaneID is less than or equal to
     * the given value.
     */
    bool operator<=(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ <= b.id_;
    }

    /**
     * @brief Checks whether the value of this LaneID is greater than the given value.
     */
    bool operator>(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ > b.id_;
    }

    /**
     * @brief Checks whether the value of this LaneID is greater than or equal
     * to the given value.
     */
    bool operator>=(const LaneID& b) const
    {
        assert(isValid() && b.isValid());
        return id_ >= b.id_;
    }

    /**
     * @brief Checks whether the two LaneID's refer to lanes on the same side of
     * the reference line.
     */
    bool sameSide(LaneID b) const
    {
        assert(isValid() && b.isValid());
        assert(id_ != 0 && b.id_ != 0);
        return (id_ ^ b.id_) >= 0;
    }

  private:
#ifndef NDEBUG
    bool isValid() const { return id_ != INVALID_TAG; }
#endif

    int id_;
};

static inline std::ostream& operator<<(std::ostream& s, const LaneID& laneId)
{
    return s << static_cast<int>(laneId);
}

/**
 * @brief An optional lane identifier.
 */
class LaneIDOpt
{
  private:
    /**
     * @brief The internal value to indicate that this LaneIDOpt doesn't hold a value.
     */
    static constexpr int NULL_TAG = std::numeric_limits<int>::max();

    /**
     * @brief The interval value which is used in debug mode to indicates that
     * a LaneID is uninitialized.
     */
    static constexpr int INVALID_TAG = std::numeric_limits<int>::max() - 1;

  public:
    /**
     * Creates an uninitialized LaneIDOpt.
     *
     * The value of an uninitialized LaneIDOpt is undefined, so a value needs to
     * be assigned to it before this LaneIDOpt becomes usable. Note that an
     * uninitialized LaneIDOpt is different from a NULL LaneIDOpt.
     */
    LaneIDOpt()
    {
#ifndef NDEBUG
        id_ = INVALID_TAG;
#endif
    }

    /**
     * @brief Constructs a LaneIDOpt which holds the given value.
     */
    LaneIDOpt(LaneID id) { id_ = id.id_; }

    /**
     * @brief Returns a null LaneIDOpt (one which doesn't hold a value).
     */
    static LaneIDOpt null()
    {
        LaneIDOpt ret;
        ret.id_ = NULL_TAG;
        return ret;
    }

    /**
     * @brief Parses a LaneIDOpt from the given string.
     */
    static XodrParseResult<LaneIDOpt> parse(const std::string& s)
    {
        return XodrParseResult<LaneIDOpt>(LaneID::parse(s).value());
    }

    /**
     * @brief Compares the two LaneIDOpt's for equality.
     */
    bool operator==(const LaneIDOpt& b) const
    {
        assert(isValid() && b.isValid());
        return id_ == b.id_;
    }

    /**
     * @brief Compares this LaneIDOpt against the LaneIDOpt with the given id value.
     */
    bool operator==(int b) const
    {
        assert(isValid());
        return id_ == b;
    }

    /**
     * @brief This conversion operator returns true if this LaneIDOpt holds a
     * value, false if it's LaneIDOpt::null().
     *
     * This operator allows you to test use LaneIDOpt the same way you'd use a
     * pointer which can be nullptr, as in the following code:
     *
     *   if (laneIdOpt)
     *   {
     *       doSomethingWithLane(*laneIdOpt);
     *   }
     */
    operator bool() const
    {
        assert(isValid());
        return id_ != NULL_TAG;
    }

    /**
     * @brief Gets the LaneID of this LaneIDOpt. This function shouldn't be used
     * with null laneIDOpt's.
     */
    LaneID operator*() const
    {
        assert(isValid());
        assert(id_ != NULL_TAG);
        return LaneID(id_);
    }

  private:
#ifndef NDEBUG
    bool isValid() const { return id_ != INVALID_TAG; }
#endif

    int id_;
};

}}  // namespace aid::xodr