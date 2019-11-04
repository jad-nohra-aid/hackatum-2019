#include "xml/xml_attribute_parsers.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(XmlAttributeParsersTest, testFieldParsers)
{
    XmlReader xml = XmlReader::fromText(
        "<elem c = '3' b = '2' a = '1' bbb = 'blabla'>"
        "</elem>");

    struct Attribs
    {
        int a_;
        int b_;
        int c_;
        std::string bbb_;
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addFieldParser("a", &Attribs::a_);
    parsers.addFieldParser("b", &Attribs::b_);
    parsers.addFieldParser("c", &Attribs::c_);
    parsers.addFieldParser("bbb", &Attribs::bbb_);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;

    parsers.parse(xml, result);
    EXPECT_EQ(result.value().a_, 1);
    EXPECT_EQ(result.value().b_, 2);
    EXPECT_EQ(result.value().c_, 3);
    EXPECT_EQ(result.value().bbb_, "blabla");
}

TEST(XmlAttributeParsersTest, testIgnoreOtherAttribs)
{
    XmlReader xml = XmlReader::fromText(
        "<elem a = '1' fluff = 'blabla'>"
        "</elem>");

    struct Attribs
    {
        int a_;
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addFieldParser("a", &Attribs::a_);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;

    parsers.parse(xml, result);
    EXPECT_EQ(result.value().a_, 1);
}

TEST(XmlAttributeParsersTest, testMissing)
{
    XmlReader xml = XmlReader::fromText(
        "<elem a = '1'>"
        "</elem>");

    struct Attribs
    {
        int a_;
        int b_;
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addFieldParser("a", &Attribs::a_);
    parsers.addFieldParser("b", &Attribs::b_);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;
    parsers.parse(xml, result);
    EXPECT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].category_, XmlParseError::Category::MISSING_ATTRIBUTE);
}

TEST(XmlAttributeParsersTest, testOptionalFieldParserAttribSpecified)
{
    XmlReader xml = XmlReader::fromText(
        "<elem a = '1'>"
        "</elem>");

    struct Attribs
    {
        int a_;
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addOptionalFieldParser("a", &Attribs::a_, 100);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;
    parsers.parse(xml, result);

    EXPECT_EQ(result.value().a_, 1);
}

TEST(XmlAttributeParsersTest, testOptionalFieldParserAttribMissing)
{
    XmlReader xml = XmlReader::fromText(
        "<elem>"
        "</elem>");

    struct Attribs
    {
        int a_;
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addOptionalFieldParser("a", &Attribs::a_, 100);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;
    parsers.parse(xml, result);

    EXPECT_EQ(result.value().a_, 100);
}

TEST(XmlAttributeParsersTest, testSetterParser)
{
    XmlReader xml = XmlReader::fromText(
        "<elem c = '3' b = '2' a = '1' bbb = 'blabla'>"
        "</elem>");

    struct Attribs
    {
        int a_;
        int b_;
        int c_;
        std::string bbb_;

        void setA(int a) { a_ = a; }
        void setB(int b) { b_ = b; }
        void setC(int c) { c_ = c; }
        void setBBB(const std::string& bbb) { bbb_ = bbb; }
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addSetterParser("a", &Attribs::setA);
    parsers.addSetterParser("b", &Attribs::setB);
    parsers.addSetterParser("c", &Attribs::setC);
    parsers.addSetterParser("bbb", &Attribs::setBBB);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;

    parsers.parse(xml, result);
    EXPECT_EQ(result.value().a_, 1);
    EXPECT_EQ(result.value().b_, 2);
    EXPECT_EQ(result.value().c_, 3);
    EXPECT_EQ(result.value().bbb_, "blabla");
}

TEST(XmlAttributeParsersTest, testOptionalSetterParserAttribSpecified)
{
    XmlReader xml = XmlReader::fromText(
        "<elem a = '1'>"
        "</elem>");

    struct Attribs
    {
        int a_;

        void setA(int a) { a_ = a; }
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addOptionalSetterParser("a", &Attribs::setA, 100);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;
    parsers.parse(xml, result);

    EXPECT_EQ(result.value().a_, 1);
}

TEST(XmlAttributeParsersTest, testOptionalSetterParserAttribMissing)
{
    XmlReader xml = XmlReader::fromText(
        "<elem>"
        "</elem>");

    struct Attribs
    {
        int a_;

        void setA(int a) { a_ = a; }
    };

    XmlAttributeParsers<XmlParseResult<Attribs>> parsers;
    parsers.addOptionalSetterParser("a", &Attribs::setA, 100);
    parsers.finalize();

    xml.readStartElement("elem");

    XmlParseResult<Attribs, XmlParseError> result;
    parsers.parse(xml, result);

    EXPECT_EQ(result.value().a_, 100);
}

}}  // namespace aid::xodr