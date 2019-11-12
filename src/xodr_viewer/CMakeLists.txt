cmake_minimum_required(VERSION 3.10)

find_package(Qt5Widgets REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(..)

add_executable(xodr_viewer
	bounding_rect.cpp
	main.cpp
	xodr_viewer_window.cpp)

target_link_libraries(xodr_viewer xodr tinyxml Qt5::Widgets Eigen3::Eigen)