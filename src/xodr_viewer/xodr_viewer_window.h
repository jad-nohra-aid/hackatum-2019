#pragma once

#include <QtWidgets/QMainWindow>

class QListWidgetItem;
class QListWidget;

namespace aid { namespace xodr {

/**
 * @brief The main window of the xodr_viewer.
 */
class XodrViewerWindow : public QMainWindow
{
  public:
	/**
	 * @brief Constructs an XodrViewerWindow instance.
	 */
	XodrViewerWindow();

  private:
    class XodrView;

    /**
     * @brief The callback called when a new xodr file in the side bar is selected.
     *
     * This function loads the xodr file with the given index and displays the
     * result in the main area of this window.
     *
     * @param index         The index of the selected xodr file.
     */
    void onXodrFileSelected(int index);

    QListWidget* sideBar_;
    XodrView* xodrView_;
};

}}  // namespace aid::xodr