#ifndef ZOOM_GRAPHICS_HEADER_GUARD
#define ZOOM_GRAPHICS_HEADER_GUARD
#include <QGraphicsView>
namespace mpMap
{
	class ZoomGraphicsView : public QGraphicsView
	{
	public:
		ZoomGraphicsView(QGraphicsScene* scene);
	protected:
		void ZoomGraphicsView::wheelEvent(QWheelEvent* event);
	};
}
#endif