#include "xodr_object_reference.h"

#include <cassert>
#include <sstream>

namespace aid { namespace xodr {

XodrParseResult<XodrObjectReference> XodrObjectReference::parse(const std::string& txt)
{
    XodrObjectReference ret;
    ret.id_ = txt;
    ret.index_ = INVALID_VALUE;
    return ret;
}

bool XodrObjectReference::operator==(const std::string& b) const
{
    return id_ == b;
}

bool XodrObjectReference::operator!=(const std::string& b) const
{
    return id_ != b;
}

bool XodrObjectReference::hasValue() const
{
    assert(index_ != INVALID_VALUE);
    return index_ != NULL_VALUE;
}

int XodrObjectReference::index() const
{
    assert(index_ >= 0);
    return index_;
}

void XodrObjectReference::resolve(const std::map<std::string, int>& idToIndex, const std::string& objTypeName)
{
    assert(index_ == INVALID_VALUE);

    auto it = idToIndex.find(id_);
    if (it == idToIndex.end())
    {
        std::stringstream err;
        err << "There's no " << objTypeName << " with identifier '" << id_ << "'.";
        throw std::runtime_error(err.str());
    }

    index_ = it->second;
}

void XodrObjectReference::resolve(const std::map<std::string, int>& idToIndex, const std::string& nullValue,
                                  const std::string& objTypeName)
{
    assert(index_ == INVALID_VALUE);

    if (id_ == nullValue)
    {
        index_ = NULL_VALUE;
    }
    else
    {
        resolve(idToIndex, objTypeName);
        assert(index_ != NULL_VALUE);
    }
}

}}  // namespace aid::xodr