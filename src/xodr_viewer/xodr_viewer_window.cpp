#include "xodr_viewer_window.h"

#include <QtGui/QPainter>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollArea>

#include "bounding_rect.h"
#include "xodr/xodr_map.h"

namespace aid { namespace xodr {

static constexpr float DRAW_SCALE = 4;
static constexpr float DRAW_MARGIN = 200;

struct XodrFileInfo
{
    const char* name;
    const char* path;
};

static const XodrFileInfo xodrFiles[] = {
    {"Crossing8Course", "/home/vanderli/Work/HackaTUM/hackatum-2019/data/opendrive/Crossing8Course.xodr"},
    {"CulDeSac", "/home/vanderli/Work/HackaTUM/hackatum-2019/data/opendrive/CulDeSac.xodr"},
    {"Roundabout8Course", "/home/vanderli/Work/HackaTUM/hackatum-2019/data/opendrive/Roundabout8Course.xodr"},
    {"sample1.1", "/home/vanderli/Work/HackaTUM/hackatum-2019/data/opendrive/sample1.1.xodr"},
};

class XodrViewerWindow::XodrView : public QWidget
{
  public:
    XodrView(QWidget* parent = nullptr) : QWidget(parent) {}

    void setMap(std::unique_ptr<XodrMap>&& xodrMap);

    virtual void paintEvent(QPaintEvent* evnt) override;

  private:
    QPointF vecToQPointF(const Eigen::Vector2d pt) const;

    std::unique_ptr<XodrMap> xodrMap_;

    Eigen::Vector2d drawOffset_;
};

XodrViewerWindow::XodrViewerWindow()
{
    setWindowTitle("XODR Viewer");
    resize(1200, 900);

    sideBar_ = new QListWidget();
    for (const XodrFileInfo& fileInfo : xodrFiles)
    {
        sideBar_->addItem(fileInfo.name);
    }

    QDockWidget* sideBarDockWidget = new QDockWidget();
    sideBarDockWidget->setWindowTitle("XODR Files");
    sideBarDockWidget->setContentsMargins(0, 0, 0, 0);
    sideBarDockWidget->setWidget(sideBar_);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, sideBarDockWidget);

    QScrollArea* scrollArea = new QScrollArea();
    setCentralWidget(scrollArea);

    xodrView_ = new XodrView();
    scrollArea->setWidget(xodrView_);

    QObject::connect(sideBar_, &QListWidget::currentRowChanged, this, &XodrViewerWindow::onXodrFileSelected);
}

void XodrViewerWindow::onXodrFileSelected(int index)
{
    const char* path = xodrFiles[index].path;

    std::cout << "Loading xodr file: " << path << std::endl;

    XodrParseResult<XodrMap> fromFileRes = XodrMap::fromFile(path);

    if (!fromFileRes.hasFatalErrors())
    {
        std::unique_ptr<XodrMap> xodrMap(new XodrMap(std::move(fromFileRes.value())));
        xodrView_->setMap(std::move(xodrMap));
    }
    else
    {
        std::cout << "Errors: " << std::endl;
        for (const auto& err : fromFileRes.errors())
        {
            std::cout << err.description() << std::endl;
        }

        QMessageBox::critical(this, "XODR Viewer", QString("Failed to load xodr file %1.").arg(path));
    }
}

void XodrViewerWindow::XodrView::setMap(std::unique_ptr<XodrMap>&& xodrMap)
{
    xodrMap_ = std::move(xodrMap);

    BoundingRect boundingRect = xodrMapApproxBoundingRect(*xodrMap_);

    Eigen::Vector2d diag = boundingRect.max_ - boundingRect.min_;
    QSize size(static_cast<int>(std::ceil(diag.x() * DRAW_SCALE + 2 * DRAW_MARGIN)),
               static_cast<int>(std::ceil(diag.y() * DRAW_SCALE + 2 * DRAW_MARGIN)));
    resize(size);

    drawOffset_ = Eigen::Vector2d(-boundingRect.min_.x() * DRAW_SCALE + DRAW_MARGIN,
                                  boundingRect.max_.y() * DRAW_SCALE + DRAW_MARGIN);
}

void XodrViewerWindow::XodrView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    for (const Road& road : xodrMap_->roads())
    {
        const auto& laneSections = road.laneSections();
        for (int laneSectionIdx = 0; laneSectionIdx < (int)laneSections.size(); laneSectionIdx++)
        {
            const LaneSection& laneSection = laneSections[laneSectionIdx];

            auto refLineTessellation = road.referenceLine().tessellate(laneSection.startS(), laneSection.endS());
            auto boundaries = laneSection.tessellateLaneBoundaryCurves(refLineTessellation);

            for (const auto& boundary : boundaries)
            {
                QVector<QPointF> qtPoints;
                for (Eigen::Vector2d pt : boundary.vertices_)
                {
                    qtPoints.append(vecToQPointF(pt));
                }
                painter.drawPolyline(qtPoints);
            }
        }
    }
}

QPointF XodrViewerWindow::XodrView::vecToQPointF(const Eigen::Vector2d pt) const
{
    return QPointF(pt.x() * DRAW_SCALE + drawOffset_.x(), -pt.y() * DRAW_SCALE + drawOffset_.y());
}

}}  // namespace aid::xodr