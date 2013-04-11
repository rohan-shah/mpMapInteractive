#include "qtPlot.h"
#include <sstream>
#include <QtGui>
#include "colour.h"
#include <QVector>
#include <QGraphicsView>
#include "ZoomGraphicsView.h"
#include <QStatusBar>
#include <QGraphicsSceneMouseEvent>
namespace mpMap
{
	qtPlot::qtPlot(double* data, int* groups, int nMarkers)
		:data(data), groups(groups), nMarkers(nMarkers), isFullScreen(false)
	{
		const int nColours = 100;
		QSharedPointer<QImage> image(new QImage(nMarkers, nMarkers, QImage::Format_Indexed8));
		//get 100 colours
		QVector<QRgb> colours;
		constructColourTable(nColours, colours);
		image->setColorTable(colours);

		//Set up image data (accounting for padding
		for(int i = 0; i < nMarkers; i++)
		{
			uchar* rawData = image->scanLine(i);
			for(int j = 0; j < nMarkers; j++)
			{
				rawData[j] = (uchar)(0.5f + nColours * data[i * nMarkers + j] / 0.5f);
			}
		}
		//convert to pixmap
		QPixmap totalPixmap = QPixmap::fromImage(*image);
		
		QGraphicsScene* graphicsScene = new QGraphicsScene();
		graphicsScene->addPixmap(totalPixmap);

		QImage back = totalPixmap.toImage();
		volatile uchar* otherRawData = back.bits();

		graphicsView = new ZoomGraphicsView(graphicsScene);
		graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		//mouse move events should be handled at a higher level
		graphicsScene->installEventFilter(this);
		
		//leave some space around the outside, when zooming. 
		QRectF bounding = graphicsView->sceneRect();
		bounding.setX(bounding.x() - nMarkers/30.0);
		bounding.setY(bounding.y() - nMarkers/30.0);
		bounding.setWidth(bounding.width() + nMarkers/15.0);
		bounding.setHeight(bounding.height() + nMarkers/15.0);
		graphicsView->setSceneRect(bounding);

		QStatusBar* statusBar = new QStatusBar();
		statusLabel = new QLabel();
		statusLabel->setText("Some text");
		statusBar->addPermanentWidget(statusLabel);
		setStatusBar(statusBar);

		setCentralWidget(graphicsView);
	}
	bool qtPlot::eventFilter(QObject *, QEvent *event)
	{
		if(event->type() == QEvent::MouseMove || event->type() == QEvent::GraphicsSceneMouseMove)
		{
			QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
			QPointF scenePos = mouseEvent->scenePos();
			std::stringstream ss;
			ss << "(" << scenePos.x() << ", " << scenePos.y() << ")";
			statusLabel->setText(QString(ss.str().c_str()));
			return true;
		}
		return false;
	}
	void qtPlot::keyPressEvent(QKeyEvent* event)
	{
		if(event->key() == Qt::Key_Return)
		{
			Qt::KeyboardModifiers mod = event->modifiers();
			if(mod & Qt::ControlModifier)
			{
				if(isFullScreen)
				{
					isFullScreen = false;
					showNormal();
				}
				else
				{
					showFullScreen();
					isFullScreen = true;
				}
				return;
			}
		}
		QMainWindow::keyPressEvent(event);
	}
	void qtPlot::closeEvent(QCloseEvent* event)
	{
		event->accept();
	}
}