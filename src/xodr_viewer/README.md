# XODR Viewer

The XODR Viewer is a simple utility which let's you select and visualize an XODR
file, using a simple outline based renderer. The main purpose of this tool is to
show how to use the XODR library. The code of interest is in `XodrViewerWindow::XodrView::paintEvent()`.

You're free to use the viewer code as a basis for your entry to the HackaTUM
challenge, though most likely, you'll want to base your submission on a 
different, 3D render engine instead (Unity, Unreal, ...). In this case, the
viewer code would serve as an example of how to use the XODR library.   

To run the viewer, simply build the whole project using the CMakeLists.txt in
the src folder, then run the resulting xodr_viewer/xodr_viewer with the root
directory of this git tree as the current working directory.