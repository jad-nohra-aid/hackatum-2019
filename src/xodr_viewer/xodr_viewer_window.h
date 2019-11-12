#pragma once

#include <QtWidgets/QMainWindow>

class QListWidgetItem;
class QListWidget;

namespace aid { namespace xodr {

class XodrViewerWindow : public QMainWindow
{
  public:
    XodrViewerWindow();

  private:
    class XodrView;

    void onXodrFileSelected(int index);

    QListWidget* sideBar_;
    XodrView* xodrView_;
};

}}  // namespace aid::xodr