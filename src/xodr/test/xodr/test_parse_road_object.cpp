#include "road_object.h"
#include "xodr_map.h"
#include "road_object_outline.h"
#include "../test_config.h"

#include <gtest/gtest.h>

namespace aid { namespace xodr {

TEST(ParseRoadObject, testParseWidthLength)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' length='10'"
        "    width='11' height='20' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");
    RoadObject roadObject = std::move(RoadObject::parseXml(xml).value());

    EXPECT_EQ(roadObject.type(), RoadObject::Type::TREE);
    EXPECT_EQ(roadObject.name(), "ForestTree06.flt");
    EXPECT_EQ(roadObject.id(), "tree 12");
    EXPECT_EQ(roadObject.s(), 25);
    EXPECT_EQ(roadObject.t(), -10);
    EXPECT_EQ(roadObject.zOffset(), 1);
    EXPECT_EQ(roadObject.validLength(), 2);
    EXPECT_EQ(roadObject.orientation(), RoadObject::Orientation::NONE);

    EXPECT_EQ(roadObject.hasBoxGeometry(), true);
    EXPECT_EQ(roadObject.hasCylinderGeometry(), false);
    EXPECT_EQ(roadObject.hasOutlineGeometry(), false);
    EXPECT_EQ(roadObject.length(), 10);
    EXPECT_EQ(roadObject.width(), 11);
    EXPECT_EQ(roadObject.height(), 20);

    EXPECT_EQ(roadObject.heading(), 1);
    EXPECT_EQ(roadObject.pitch(), .1);
    EXPECT_EQ(roadObject.roll(), .01);
}

TEST(ParseRoadObject, testParseRadius)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' radius='10'"
        "    height='20' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");
    RoadObject roadObject = std::move(RoadObject::parseXml(xml).value());

    EXPECT_EQ(roadObject.type(), RoadObject::Type::TREE);
    EXPECT_EQ(roadObject.name(), "ForestTree06.flt");
    EXPECT_EQ(roadObject.id(), "tree 12");
    EXPECT_EQ(roadObject.s(), 25);
    EXPECT_EQ(roadObject.t(), -10);
    EXPECT_EQ(roadObject.zOffset(), 1);
    EXPECT_EQ(roadObject.validLength(), 2);
    EXPECT_EQ(roadObject.orientation(), RoadObject::Orientation::NONE);

    EXPECT_EQ(roadObject.hasBoxGeometry(), false);
    EXPECT_EQ(roadObject.hasCylinderGeometry(), true);
    EXPECT_EQ(roadObject.hasOutlineGeometry(), false);
    EXPECT_EQ(roadObject.radius(), 10);
    EXPECT_EQ(roadObject.height(), 20);

    EXPECT_EQ(roadObject.heading(), 1);
    EXPECT_EQ(roadObject.pitch(), .1);
    EXPECT_EQ(roadObject.roll(), .01);
}

TEST(ParseRoadObject, testParseWidthLengthAndRadius)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' length='10'"
        "    width='11' radius='12' height='20' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");

    XodrParseResult<RoadObject> result(RoadObject::parseXml((xml)));
    EXPECT_EQ(result.errors().size(), 1);
}

TEST(ParseRoadObject, testParseWidthNoLength)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none'"
        "    width='11' height='20' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");

    XodrParseResult<RoadObject> result(RoadObject::parseXml((xml)));
    EXPECT_EQ(result.errors().size(), 1);
}

TEST(ParseRoadObject, testLengthNoWidth)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' length='10'"
        "    height='20' radius='30' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");

    XodrParseResult<RoadObject> result(RoadObject::parseXml((xml)));
    EXPECT_EQ(result.errors().size(), 2);
}

TEST(ParseRoadObject, testLengthWidthNoHeight)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' length='10'"
        "    width='20' radius='30' hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");

    XodrParseResult<RoadObject> result(RoadObject::parseXml((xml)));
    EXPECT_EQ(result.errors().size(), 2);
}

TEST(ParseRoadObject, testParseRadiusNoHeight)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='tree' name='ForestTree06.flt' id='tree 12' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' radius='10'"
        "    hdg='1' pitch='.1' roll='.01'/>");

    xml.readStartElement("object");

    XodrParseResult<RoadObject> result(RoadObject::parseXml((xml)));
    EXPECT_EQ(result.errors().size(), 1);
}

TEST(ParseRoadObject, testParseOutline)
{
    XodrReader xml = XodrReader::fromText(
        "<object type='building' name='Tower' id='tower 1' s='25' t='-10' "
        "    zOffset='1' validLength='2' orientation='none' hdg='1' pitch='.1' roll='.01'>"
        "  <outline>"
        "    <cornerRoad s='.5' t='3' dz='.1' height='4'/>"
        "    <cornerLocal u='1' v='2' z='3' height='4'/>"
        "  </outline>"
        "</object>");

    xml.readStartElement("object");
    RoadObject roadObject = std::move(RoadObject::parseXml(xml).value());

    EXPECT_EQ(roadObject.type(), RoadObject::Type::BUILDING);
    EXPECT_EQ(roadObject.name(), "Tower");
    EXPECT_EQ(roadObject.id(), "tower 1");
    EXPECT_EQ(roadObject.s(), 25);
    EXPECT_EQ(roadObject.t(), -10);
    EXPECT_EQ(roadObject.zOffset(), 1);
    EXPECT_EQ(roadObject.validLength(), 2);
    EXPECT_EQ(roadObject.orientation(), RoadObject::Orientation::NONE);

    EXPECT_EQ(roadObject.hasBoxGeometry(), false);
    EXPECT_EQ(roadObject.hasCylinderGeometry(), false);
    EXPECT_EQ(roadObject.hasOutlineGeometry(), true);

    EXPECT_EQ(roadObject.heading(), 1);
    EXPECT_EQ(roadObject.pitch(), .1);
    EXPECT_EQ(roadObject.roll(), .01);

    const RoadObjectOutline& outline = roadObject.outline();

    const auto& corners = outline.corners();
    EXPECT_EQ(corners.size(), 2);

    {
        const RoadObjectOutline::Corner& corner = corners[0];
        EXPECT_EQ(corner.which(), 0);

        const auto& cornerRoad = boost::get<RoadObjectOutline::CornerRoad>(corner);
        EXPECT_EQ(cornerRoad.s(), .5);
        EXPECT_EQ(cornerRoad.t(), 3);
        EXPECT_EQ(cornerRoad.dz(), .1);
        EXPECT_EQ(cornerRoad.height(), 4);
    }

    {
        const RoadObjectOutline::Corner& corner = corners[1];
        EXPECT_EQ(corner.which(), 1);

        const auto& cornerLocal = boost::get<RoadObjectOutline::CornerLocal>(corner);
        EXPECT_EQ(cornerLocal.u(), 1);
        EXPECT_EQ(cornerLocal.v(), 2);
        EXPECT_EQ(cornerLocal.z(), 3);
        EXPECT_EQ(cornerLocal.height(), 4);
    }
}

TEST(ParseRoadObject, testParseFullDocBox)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_road_objects/box_road_objects.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    xodrMap.validate();
}

TEST(ParseRoadObject, testParseFullDocCylinder)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_road_objects/cylinder_road_objects.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    xodrMap.validate();
}

TEST(ParseRoadObject, testParseFullDocOutline)
{
    XodrReader xml =
        XodrReader::fromFile(std::string(TEST_DATA_PATH_PREFIX) + "xodr/test_road_objects/outline_road_objects.xodr");

    xml.readStartElement("OpenDRIVE");
    XodrMap xodrMap = std::move(XodrMap::parseXml(xml).value());

    xodrMap.validate();
}

}}  // namespace aid::xodr
