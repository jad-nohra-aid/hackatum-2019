#include <QtWidgets/QApplication>

#include "xodr_viewer_window.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QObject::connect(&a, &QApplication::lastWindowClosed, &a, &QApplication::quit);

    aid::xodr::XodrViewerWindow mainWnd;

    mainWnd.show();
    int ret = a.exec();

    return ret;
}
