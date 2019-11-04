#include "xml/xml_reader.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(XmlReaderTest, testReadElement)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "  <child1>"
        "  </child1>"
        "  <child2>"
        "  </child2>"
        "</root>");

    xml.readStartElement();
    EXPECT_EQ(xml.getCurElementName(), "root");

    xml.readStartElement();
    EXPECT_EQ(xml.getCurElementName(), "child1");
    xml.readEndElement();

    xml.readStartElement();
    EXPECT_EQ(xml.getCurElementName(), "child2");
    xml.readEndElement();

    EXPECT_ANY_THROW(xml.readStartElement());

    xml.readEndElement();
}

TEST(XmlReaderTest, testReadNamedElement)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "  <child1>"
        "  </child1>"
        "  <child2>"
        "  </child2>"
        "</root>");

    EXPECT_ANY_THROW(xml.readStartElement("notRoot"));
    xml.readStartElement("root");

    EXPECT_ANY_THROW(xml.readStartElement("notChild1"));
    xml.readStartElement("child1");
    xml.readEndElement();

    EXPECT_ANY_THROW(xml.readStartElement("notChild2"));
    xml.readStartElement("child2");
    xml.readEndElement();

    EXPECT_ANY_THROW(xml.readStartElement("nope"));

    xml.readEndElement();
}

TEST(XmlReaderTest, testTryReadNamedElement)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "  <child1>"
        "  </child1>"
        "  <child2>"
        "  </child2>"
        "</root>");

    EXPECT_FALSE(xml.tryReadStartElement("notRoot"));
    EXPECT_TRUE(xml.tryReadStartElement("root"));

    EXPECT_FALSE(xml.tryReadStartElement("notChild1"));
    EXPECT_TRUE(xml.tryReadStartElement("child1"));
    xml.readEndElement();

    EXPECT_FALSE(xml.tryReadStartElement("notChild2"));
    EXPECT_TRUE(xml.tryReadStartElement("child2"));
    xml.readEndElement();

    xml.readEndElement();
}

}}  // namespace aid::xodr