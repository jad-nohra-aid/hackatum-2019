cmake_minimum_required(VERSION 3.10)

include(GoogleTest)

find_package(Eigen3 REQUIRED)

include_directories(. ${EIGEN3_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})

add_library(xodr
	elevation.cpp
	junction_parser.cpp
	junction.cpp
	lane_attributes.cpp
	lane_section_parser.cpp
	lane_section.cpp
	odrSpiral/odrSpiral.c
	poly3.cpp
	reference_line_parser.cpp
	reference_line.cpp
	road_link_parser.cpp
	road_link.cpp
	road_object_outline.cpp
	road_object.cpp
	road_parser.cpp
	road.cpp
	units.cpp
	validation/junction_validation.cpp
	validation/lane_link_validation.cpp
	validation/road_link_validation.cpp
	xml/xml_attribute_parsers.cpp
	xml/xml_parse_result.cpp
	xml/xml_reader.cpp
	xodr_map.cpp
	xodr_map_keys.cpp
	xodr_object_reference.cpp
	xodr_reader.cpp)

add_executable(xodr_tests
	test/xml/test_xml_attribute_parsers.cpp
	test/xml/test_xml_child_element_parsers.cpp
	test/xml/test_xml_reader.cpp
	test/xodr/test_junction.cpp
	test/xodr/test_lane_attributes.cpp
	test/xodr/test_lane_section.cpp
	test/xodr/test_parse_junction.cpp
	test/xodr/test_parse_lane_section.cpp
	test/xodr/test_parse_reference_line.cpp
	test/xodr/test_parse_road.cpp
	test/xodr/test_parse_road_link.cpp
	test/xodr/test_parse_road_object.cpp
	test/xodr/test_poly3.cpp
	test/xodr/test_reference_line.cpp
	test/xodr/test_road.cpp
	test/xodr/test_xodr_map.cpp
	test/xodr/test_xodr_object_reference.cpp
	test/xodr/test_xodr_utils.cpp)

target_link_libraries(xodr_tests xodr gtest_main gtest tinyxml proj pthread)
