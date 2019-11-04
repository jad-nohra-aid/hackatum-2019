#include "xml/xml_child_element_parsers.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

struct ChildElem
{
    std::string name_;

    static XmlParseResult<ChildElem> parseXml(XmlReader& xml)
    {
        ChildElem ret;

        ret.name_ = xml.getAttribute("name");
        xml.readEndElement();

        return ret;
    }
};

TEST(XmlChildElementParsersTest, testParseOneChild)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "  <a name = 'Mueller'/>"
        "</root>");

    struct Obj
    {
        int numCalls_;
    };

    XmlChildElementParsers<XmlReader, XmlParseResult<Obj>> parsers;
    parsers.addParser("a", XmlChildElementParsers<XmlReader, XmlParseResult<Obj>>::Multiplicity::ONE,
                      [](XmlReader& xml, XmlParseResult<Obj>& obj) {
                          obj.value().numCalls_++;

                          xml.skipToEndElement();
                      });

    parsers.finalize();

    XmlParseResult<Obj> obj;

    xml.readStartElement("root");
    parsers.parse(xml, obj);

    EXPECT_EQ(obj.value().numCalls_, 1);
}

TEST(XmlChildElementParsersTest, testParseOneChild_Missing)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "</root>");

    struct Obj
    {
        int numCalls_;
    };

    XmlChildElementParsers<XmlReader, XmlParseResult<Obj>> parsers;
    parsers.addParser("a", XmlChildElementParsers<XmlReader, XmlParseResult<Obj>>::Multiplicity::ONE,
                      [](XmlReader& xml, XmlParseResult<Obj>& obj) {
                          obj.value().numCalls_++;

                          xml.skipToEndElement();
                      });

    parsers.finalize();

    XmlParseResult<Obj> obj;
    xml.readStartElement("root");
    parsers.parse(xml, obj);
    EXPECT_EQ(obj.errors().at(0).category_, XmlParseError::Category::MISSING_CHILD_ELEMENT);
    EXPECT_EQ(obj.errors().at(0).name_, "root");
    EXPECT_EQ(obj.errors().at(0).value_, "a");
}

TEST(XmlChildElementParsersTest, testParseOneChild_TooMany)
{
    XmlReader xml = XmlReader::fromText(
        "<root>"
        "  <a name = 'Mueller'/>"
        "  <a name = 'Schneider'/>"
        "</root>");

    struct Obj
    {
        int numCalls_;
    };

    XmlChildElementParsers<XmlReader, XmlParseResult<Obj>> parsers;
    parsers.addParser("a", XmlChildElementParsers<XmlReader, XmlParseResult<Obj>>::Multiplicity::ONE,
                      [](XmlReader& xml, XmlParseResult<Obj>& obj) {
                          obj.value().numCalls_++;

                          xml.skipToEndElement();
                      });

    parsers.finalize();

    XmlParseResult<Obj> obj;

    xml.readStartElement("root");
    parsers.parse(xml, obj);
    EXPECT_EQ(obj.errors().at(0).category_, XmlParseError::Category::DUPLICATE_CHILD_ELEMENT);
    EXPECT_EQ(obj.errors().at(0).name_, "root");
    EXPECT_EQ(obj.errors().at(0).value_, "a");
}

}}  // namespace aid::xodr
