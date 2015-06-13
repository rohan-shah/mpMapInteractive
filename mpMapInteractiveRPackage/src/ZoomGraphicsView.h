#ifndef ZOOM_GRAPHICS_HEADER_GUARD
#define ZOOM_GRAPHICS_HEADER_GUARD
#include <QGraphicsView>
namespace mpMapInteractive
{
	class ZoomGraphicsView : public QGraphicsView
	{
		Q_OBJECT
	public:
		ZoomGraphicsView(QGraphicsScene* scene);
	protected:
		void wheelEvent(QWheelEvent* event);
	};
}
#endif
