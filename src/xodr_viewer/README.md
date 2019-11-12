# XODR Viewer

The XODR Viewer is a simple utility which shows how to use the xodr library to
load an XODR file and tessellate it. The code of interest is in
`XodrViewerWindow::XodrView::paintEvent()`.

You're free to use this as a basis for your entry to the HackaTUM challenge, 
though most likely, you'll want to use the library in combination with a
different, 3D render engine instead (Unity, Unreal, ...), in which case this 
code would just be an example of how to use the XODR library.

To run the viewer, simply build the whole project using the CMakeLists.txt in
the src folder, then run the resulting xodr_viewer/xodr_viewer with the root
directory of this git tree as the current working directory.