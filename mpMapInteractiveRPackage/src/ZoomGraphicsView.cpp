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
		QPointF currentCentre = mapToScene(viewport()->rect()).boundingRect().center();
		QPointF pointBeforeScale = this->mapToScene(event->pos());
		scale(factor, factor);
		QPointF pointAfterScale = this->mapToScene(event->pos());
		this->centerOn(currentCentre + (pointBeforeScale - pointAfterScale));
	}
}