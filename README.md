# hackatum-2019

To be able to build the xodr library and xodr_viewer, make sure to have the following packages installed:

Ubuntu:

GTest (follow instructions at https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)
libeigen3-dev
libproj-dev
libtinyxml-dev
qt5-default

Once these are installed, execute the following from the root of your git checkout:

cd src
mkdir build
cd build
cmake ..
make
cd ../..
src/build/xodr_viewer/xodr_viewer
