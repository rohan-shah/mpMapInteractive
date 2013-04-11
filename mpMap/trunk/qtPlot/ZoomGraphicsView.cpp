#include "ZoomGraphicsView.h"
#include <QWheelEvent>
#include <QGraphicsScene>
namespace mpMap
{
	ZoomGraphicsView::ZoomGraphicsView(QGraphicsScene* scene)
		:QGraphicsView(scene)
	{
		setMouseTracking(true);
	}
	void ZoomGraphicsView::wheelEvent(QWheelEvent* event)
	{
		qreal factor = 1.2;
		if (event->delta() < 0) factor = 1.0 / factor;
		QPointF centre = this->mapToScene(event->pos());
		scale(factor, factor);
		this->centerOn(centre);
	}
}