#include "xml/xml_reader.h"

#include <stdexcept>
#include <sstream>
#include <vector>

namespace aid { namespace xodr {

XmlReader XmlReader::fromFile(const std::string& fileName)
{
    XmlReader ret;
    ret.initFromFile(fileName);
    return ret;
}

void XmlReader::initFromFile(const std::string& fileName)
{
    if (!doc_.LoadFile(fileName.c_str()))
    {
        throw std::runtime_error(doc_.ErrorDesc());
    }

    curElement_ = nullptr;
}

XmlReader XmlReader::fromText(const std::string& text)
{
    XmlReader ret;
    ret.initFromText(text);
    return ret;
}

void XmlReader::initFromText(const std::string& text)
{
    doc_.Parse(text.c_str());
    if (doc_.Error())
    {
        throw std::runtime_error(doc_.ErrorDesc());
    }

    curElement_ = nullptr;
}

void XmlReader::readStartElement()
{
    if (!tryReadStartElement())
    {
        throw std::runtime_error("Start element expected.");
    }
}

void XmlReader::readStartElement(const std::string& expectedName)
{
    if (!tryReadStartElement(expectedName))
    {
        std::stringstream msg;
        msg << "Element of type \"" << expectedName << "\" expected.";
        throw std::runtime_error(msg.str());
    }
}

void XmlReader::readEndElement()
{
    assert(curElement_);

    if (endOfElement_)
    {
        if (curElement_->NextSiblingElement())
        {
            throw std::runtime_error("End element expected");
        }

        curElement_ = static_cast<TiXmlElement*>(curElement_->Parent());
        assert(!curElement_ || curElement_->Type() == TiXmlNode::TINYXML_ELEMENT);
    }
    else
    {
        if (curElement_->FirstChildElement())
        {
            throw std::runtime_error("End element expected");
        }

        endOfElement_ = true;
    }
}

void XmlReader::skipToEndElement()
{
    assert(curElement_);

    if (endOfElement_)
    {
        curElement_ = static_cast<TiXmlElement*>(curElement_->Parent());
        assert(!curElement_ || curElement_->Type() == TiXmlNode::TINYXML_ELEMENT);
    }
    else
    {
        endOfElement_ = true;
    }
}

bool XmlReader::tryReadStartElement()
{
    if (!curElement_)
    {
        curElement_ = doc_.RootElement();
        endOfElement_ = false;
        return true;
    }
    else if (!endOfElement_)
    {
        TiXmlElement* nextElem = curElement_->FirstChildElement();
        if (nextElem)
        {
            curElement_ = nextElem;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        TiXmlElement* nextElem = curElement_->NextSiblingElement();
        if (nextElem)
        {
            curElement_ = nextElem;
            endOfElement_ = false;
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool XmlReader::tryReadStartElement(const std::string& expectedName)
{
    if (!curElement_)
    {
        TiXmlElement* nextElem = doc_.RootElement();
        if (nextElem && nextElem->Value() == expectedName)
        {
            curElement_ = nextElem;
            endOfElement_ = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (!endOfElement_)
    {
        TiXmlElement* nextElem = curElement_->FirstChildElement();
        if (nextElem && nextElem->Value() == expectedName)
        {
            curElement_ = nextElem;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        TiXmlElement* nextElem = curElement_->NextSiblingElement();
        if (nextElem && nextElem->Value() == expectedName)
        {
            curElement_ = nextElem;
            endOfElement_ = false;
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool XmlReader::tryReadEndElement()
{
    assert(curElement_);

    if (endOfElement_)
    {
        if (curElement_->NextSiblingElement())
        {
            return false;
        }

        curElement_ = static_cast<TiXmlElement*>(curElement_->Parent());
        return true;
    }
    else
    {
        if (curElement_->FirstChildElement())
        {
            return false;
        }

        endOfElement_ = true;
        return true;
    }
}

std::string XmlReader::getCurElementName() const
{
    assert(curElement_);
    return curElement_->Value();
}

std::vector<XmlReader::Attrib> XmlReader::getAttributes() const
{
    assert(curElement_);
    assert(!endOfElement_);

    std::vector<Attrib> ret;
    for (const TiXmlAttribute* attrib = curElement_->FirstAttribute(); attrib; attrib = attrib->Next())
    {
        ret.push_back({attrib->Name(), attrib->Value()});
    }
    return ret;
}

std::string XmlReader::getAttribute(const std::string& name) const
{
    assert(curElement_);
    assert(!endOfElement_);

    const char* ret = curElement_->Attribute(name.c_str());
    if (!ret)
    {
        std::stringstream err;
        err << "Attribute '" << name << ". expected.";
        throw std::runtime_error(err.str());
    }

    return ret;
}

std::string XmlReader::getText() const
{
    assert(curElement_);
    assert(!endOfElement_);

    const TiXmlNode* firstChild = curElement_->FirstChild();
    if (firstChild != curElement_->LastChild() || firstChild->Type() != TiXmlNode::TINYXML_TEXT)
    {
        throw std::runtime_error("Text expected.");
    }

    const TiXmlText* text = firstChild->ToText();
    return text->Value();
}

int XmlReader::getLineNumber() const
{
    assert(!endOfElement_);  // Not implemented yet for end elements.
    return curElement_->Row();
}

int XmlReader::getColumnNumber() const
{
    assert(!endOfElement_);  // Not implemented yet for end elements.
    return curElement_->Column();
}

}}  // namespace aid::xodr
