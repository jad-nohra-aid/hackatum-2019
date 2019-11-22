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

static constexpr float DRAW_SCALE = 8;
static constexpr float DRAW_MARGIN = 200;

struct XodrFileInfo
{
    const char* name;
    const char* path;
};

static const XodrFileInfo xodrFiles[] = {
    {"Crossing8Course", "data/opendrive/Crossing8Course.xodr"},
    {"CulDeSac", "data/opendrive/CulDeSac.xodr"},
    {"Roundabout8Course", "data/opendrive/Roundabout8Course.xodr"},
    {"sample1.1", "data/opendrive/sample1.1.xodr"},
};

/**
 * @brief The view which is shown inside the main area's QScrollArea.
 *
 * This view renders the XodrMap specified using the setMap function.
 */
class XodrViewerWindow::XodrView : public QWidget
{
  public:
	/**
	 * @brief Constructs a new XodrView.
	 *
	 * @param parent        The parent widget.
	 */
    XodrView(QWidget* parent = nullptr) : QWidget(parent) {}

    void setMap(std::unique_ptr<XodrMap>&& xodrMap);

    virtual void paintEvent(QPaintEvent* evnt) override;

  private:
	/**
	 * @brief Converts a point form XODR map coordinates to view coordinates.
	 *
	 * @param pt            The point in map coordinates.
	 * @return              The point in view coordinates.
	 */
    QPointF pointMapToView(const Eigen::Vector2d pt) const;

    std::unique_ptr<XodrMap> xodrMap_;

    /**
     * @brief The offset used in the pointMapToView function.
     *
     * See the pointMapToView function for the exact meaning of it.
     */
    Eigen::Vector2d mapToViewOffset_;
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

    // Compute the size big enough for the bounding rectangle, scaled by
    // DRAW_SCALE, and with margins of size DRAW_MARGIN on all sides.
    QSize size(static_cast<int>(std::ceil(diag.x() * DRAW_SCALE + 2 * DRAW_MARGIN)),
               static_cast<int>(std::ceil(diag.y() * DRAW_SCALE + 2 * DRAW_MARGIN)));
    resize(size);

    mapToViewOffset_ = Eigen::Vector2d(-boundingRect.min_.x() * DRAW_SCALE + DRAW_MARGIN,
                                  boundingRect.max_.y() * DRAW_SCALE + DRAW_MARGIN);
}

static bool showLaneType(LaneType laneType)
{
    return laneType == LaneType::DRIVING ||
        laneType == LaneType::SIDEWALK ||
        laneType == LaneType::BORDER;
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
            const auto& lanes = laneSection.lanes();

            for (size_t i = 0; i < boundaries.size(); i++)
            {
                const LaneSection::BoundaryCurveTessellation& boundary = boundaries[i];

                if(i == 0)
                {
                    // The left-most boundary. Only render it if the left-most 
                    // lane is visible.

                    if(!showLaneType(lanes[i].type()))
                    {
                        continue;
                    }
                }
                else if(i == boundaries.size() - 1)
                {
                    // A boundary between two lanes. Render it if at least one
                    // of the two adjacent lanes is visible.
                    if(!showLaneType(lanes[i - 1].type()))
                    {
                        continue;
                    }
                }
                else
                {
                    // The right-most boundary. Only render it if the right-most
                    // lane is visible.

                    if(!showLaneType(lanes[i - 1].type()) && 
                        !showLaneType(lanes[i].type()))
                    {
                        continue;
                    }
                }

                QVector<QPointF> qtPoints;
                for (Eigen::Vector2d pt : boundary.vertices_)
                {
                    qtPoints.append(pointMapToView(pt));
                }
                painter.drawPolyline(qtPoints);
            }
        }
    }
}

QPointF XodrViewerWindow::XodrView::pointMapToView(const Eigen::Vector2d pt) const
{
    // Scales the map point by DRAW_SCALE, flips the y axis, then applies
    // mapToViewOffset_, which is computes such that it shifts the view space
    // bounding rectangle to have margins of DRAW_MARGIN on all sides.
    return QPointF(pt.x() * DRAW_SCALE + mapToViewOffset_.x(), -pt.y() * DRAW_SCALE + mapToViewOffset_.y());
}

}}  // namespace aid::xodr