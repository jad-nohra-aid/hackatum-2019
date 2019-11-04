#include "xodr_object_reference.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(XodrObjectReferenceTest, testParse)
{
    XodrObjectReference ref = XodrObjectReference::parse("targetObjId").value();
    EXPECT_EQ(ref.id(), "targetObjId");

    EXPECT_TRUE(ref == "targetObjId");
    EXPECT_FALSE(ref == "targetObjId?");

    EXPECT_FALSE(ref != "targetObjId");
    EXPECT_TRUE(ref != "targetObjId?");
}

TEST(XodrObjectReferenceTest, testResolve)
{
    XodrObjectReference ref = XodrObjectReference::parse("targetObjId").value();

    std::map<std::string, int> idToIndex;
    idToIndex.insert(std::make_pair("targetObjId?", 1));
    idToIndex.insert(std::make_pair("targetObjId", 2));
    idToIndex.insert(std::make_pair("noooooo", 3));

    ref.resolve(idToIndex, "Gadget");
    EXPECT_EQ(ref.index(), 2);
}

TEST(XodrObjectReferenceTest, testResolveFailure)
{
    XodrObjectReference ref = XodrObjectReference::parse("targetObjId").value();

    std::map<std::string, int> idToIndex;
    idToIndex.insert(std::make_pair("me?", 1));
    idToIndex.insert(std::make_pair("not me...", 2));
    idToIndex.insert(std::make_pair("noooooo", 3));

    EXPECT_ANY_THROW(ref.resolve(idToIndex, "Gadget"));
}

TEST(XodrObjectReference, testHasValue)
{
    XodrObjectReference ref = XodrObjectReference::parse("id1").value();

    std::map<std::string, int> idToIndex;
    idToIndex.insert(std::make_pair("id1", 1));

    ref.resolve(idToIndex, "-1", "Gadget");

    EXPECT_TRUE(ref.hasValue());
    EXPECT_EQ(ref.index(), 1);
}

TEST(XodrObjectReference, testHasNullValue)
{
    XodrObjectReference ref = XodrObjectReference::parse("-1").value();

    std::map<std::string, int> idToIndex;
    idToIndex.insert(std::make_pair("id1", 1));

    ref.resolve(idToIndex, "-1", "Gadget");

    EXPECT_FALSE(ref.hasValue());
}

}}  // namespace aid::xodr