#include "qtPlot.h"
#include <sstream>
#include <QtGui>
#include "colour.h"
#include <QVector>
#include <QGraphicsView>
#include "ZoomGraphicsView.h"
#include <QStatusBar>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <exception>
namespace mpMap
{
	const std::vector<int>& qtPlotData::getCurrentGroups() const
	{
		if(groups.size() == 0) return originalGroups;
		return groups[groups.size()-1];
	}
	int qtPlotData::startOfGroup(int group)
	{
		const std::vector<int>& currentGroups = getCurrentGroups();
		return std::distance(currentGroups.begin(), std::find(currentGroups.begin(), currentGroups.end(), group));
	}
	int qtPlotData::endOfGroup(int group)
	{
		const std::vector<int>& currentGroups = getCurrentGroups();
		return std::distance(currentGroups.begin(), std::find(currentGroups.rbegin(), currentGroups.rend(), group).base());
	}
	const std::vector<std::string> qtPlotData::getCurrentMarkerNames() const
	{
		return currentMarkerNames;
	}
	qtPlotData::qtPlotData(const std::vector<int>& originalGroups, const std::vector<std::string>& originalMarkerNames)
		:originalGroups(originalGroups), originalMarkerNames(originalMarkerNames), nMarkers(originalMarkerNames.size()), currentMarkerNames(originalMarkerNames)
	{
		if(originalGroups.size() != originalMarkerNames.size()) throw std::runtime_error("Internal error");
		//set up identity permutation initially
		for(int i = 0; i < originalGroups.size(); i++) identity.push_back(i);
	}
	const std::vector<int>& qtPlotData::getCurrentPermutation() const
	{
		if(cumulativePermutations.size() == 0)
		{
			return identity;
		}
		return cumulativePermutations[cumulativePermutations.size()-1];
	}
	void qtPlotData::undo()
	{
		if(cumulativePermutations.size() != 0)
		{
			cumulativePermutations.pop_back();
			groups.pop_back();
			
			const std::vector<int>& currentCumulativePermutation = getCurrentPermutation();
			for(int i = 0; i < nMarkers; i++)
			{
				currentMarkerNames[i] = originalMarkerNames[currentCumulativePermutation[i]];
			}
		}
	}
	void qtPlotData::applyPermutation(std::vector<int>& permutation, std::vector<int>& newGroups)
	{
		const std::vector<int>& currentCumulativePermutation = getCurrentPermutation();
		std::vector<int> newCumulativePermutation; 
		newCumulativePermutation.resize(nMarkers);

		currentMarkerNames.resize(nMarkers);

		for(int i = 0; i < nMarkers; i++)
		{
			newCumulativePermutation[i] = currentCumulativePermutation[permutation[i]];
			currentMarkerNames[i] = originalMarkerNames[newCumulativePermutation[i]];
		}
		cumulativePermutations.push_back(newCumulativePermutation);
		groups.push_back(newGroups);
	}
	qtPlot::~qtPlot()
	{
		delete originalDataToChar;
		if(pixmapItem != NULL) delete pixmapItem;
	}
	qtPlot::qtPlot(double* rawImageData, const std::vector<int>& originalGroups, int nMarkers, const std::vector<std::string>& originalMarkerNames)
		:rawImageData(rawImageData), data(new qtPlotData(originalGroups, originalMarkerNames)), nMarkers(nMarkers), isFullScreen(false), horizontalGroup(-1), verticalGroup(-1), pixmapItem(NULL), horizontalHighlight(NULL), verticalHighlight(NULL)
	{
		image = QSharedPointer<QImage>(new QImage(nMarkers, nMarkers, QImage::Format_Indexed8));
		//get 100 colours
		QVector<QRgb> colours;
		constructColourTable(nColours, colours);
		image->setColorTable(colours);

		originalDataToChar = new uchar[nMarkers * nMarkers];
		for(int i = 0; i < nMarkers; i++)
		{
			for(int j = 0; j < nMarkers; j++)
			{
				originalDataToChar[i * nMarkers + j] = (uchar)(0.5f + nColours * rawImageData[i * nMarkers + j] / 0.5f);
			}
		}

		graphicsScene = new QGraphicsScene();	
		graphicsView = new ZoomGraphicsView(graphicsScene);
		updateImageFromRaw();
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
	const qtPlotData& qtPlot::getData()
	{
		return *data;
	}
	void qtPlot::renewHighlighting(int x, int y)
	{
		//are we not highlighting anything?
		if(x == -1 || y == -1)
		{
			//is this different from previous state?
			if(horizontalGroup == -1 || verticalGroup == -1)
			{
				return;
			}
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(horizontalHighlight));
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(verticalHighlight));
			delete horizontalHighlight;
			delete verticalHighlight;
			horizontalGroup = -1;
			verticalGroup = -1;
			return;
		}
		const std::vector<int>& currentGroups = data->getCurrentGroups();
		int newHorizontalGroup = currentGroups[y];
		int newVerticalGroup = currentGroups[x];
		//If we're highlighting the same stuff as previously, do nothing
		if(newHorizontalGroup == horizontalGroup && newVerticalGroup == verticalGroup)
		{
			return;
		}
		if(horizontalGroup != -1)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(horizontalHighlight));
			delete horizontalHighlight;
		}
		if(verticalGroup != -1)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(verticalHighlight));
			delete verticalHighlight;
		}

		int firstHorizontalIndex = data->startOfGroup(newHorizontalGroup);
		int lastHorizontalIndex = data->endOfGroup(newHorizontalGroup);

		int firstVerticalIndex = data->startOfGroup(newVerticalGroup);
		int lastVerticalIndex = data->endOfGroup(newVerticalGroup);

		QColor highlightColour("blue");
		highlightColour.setAlphaF(0.3);
		horizontalHighlight = graphicsScene->addRect(0, firstHorizontalIndex, nMarkers, lastHorizontalIndex - firstHorizontalIndex, QPen(Qt::NoPen), highlightColour);
		verticalHighlight = graphicsScene->addRect(firstVerticalIndex, 0, lastVerticalIndex - firstVerticalIndex, nMarkers, QPen(Qt::NoPen), highlightColour);
		
		horizontalGroup = newHorizontalGroup;
		verticalGroup = newVerticalGroup;
		
		graphicsScene->update();
}
	bool qtPlot::eventFilter(QObject *, QEvent *event)
	{
		QSharedPointer<qtPlotData> data = this->data;
		if(event->type() == QEvent::MouseMove || event->type() == QEvent::GraphicsSceneMouseMove)
		{
			QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
			QPointF scenePos = mouseEvent->scenePos();
			qreal x_ = scenePos.x(), y_ = scenePos.y();
			int x = (int)(x_  + 0), y = (int)(y_ + 0);
			std::stringstream ss;
			if(0 <= x && x < nMarkers && 0 <= y && y < nMarkers)
			{
				const std::vector<std::string> currentMarkerNames = data->getCurrentMarkerNames();
				renewHighlighting(x, y);
				ss << "Markers (" << currentMarkerNames[x] << ", " << currentMarkerNames[y] << "), ";
			}
			else
			{
				renewHighlighting(-1, -1);
			}
			ss << "position (" << x << ", " << y << ")";
			statusLabel->setText(QString(ss.str().c_str()));
			
			return true;
		}
		return false;
	}
	void qtPlot::joinMarkers(int x, int y)
	{
		if(!(0 <= x && x < nMarkers && 0 <= y && y < nMarkers))
		{
			throw std::runtime_error("Internal error");
		}
		const std::vector<int>& currentGroups = data->getCurrentGroups();
		int group1 = currentGroups[x];
		int group2 = currentGroups[y];
		
		//can't join a group to itself
		if(group1 == group2) return;

		int newGroup = std::min(group1, group2);

		int startGroup1 = data->startOfGroup(group1);
		int startGroup2 = data->startOfGroup(group2);
		int endGroup1 = data->endOfGroup(group1), endGroup2 = data->endOfGroup(group2);

		std::vector<int> permutation;
		permutation.resize(nMarkers);
		std::vector<int> newGroups;
		newGroups.resize(nMarkers);

		int counter = 0;
		while(counter < std::min(startGroup1, startGroup2))
		{
			permutation[counter] = counter;
			newGroups[counter] = currentGroups[counter];
			counter++;
		}
		//put in groups 1 and 2
		for(int i = startGroup1; i < endGroup1; i++)
		{
			permutation[counter] = i;
			newGroups[counter] = newGroup;
			counter++;
		}
		for(int i = startGroup2; i < endGroup2; i++)
		{
			permutation[counter] = i;
			newGroups[counter] = newGroup;
			counter++;
		}
		//now put in everything else
		for(int i = std::min(startGroup1, startGroup2); i < nMarkers; i++)
		{
			if(currentGroups[i] != group1 && currentGroups[i] != group2)
			{
				permutation[counter] = i;
				newGroups[counter] = currentGroups[i];
				counter++;
			}
		}
		applyPermutation(permutation, newGroups);
	}
	void qtPlot::applyPermutation(std::vector<int>& permutation, std::vector<int>& newGroups)
	{
		data->applyPermutation(permutation, newGroups);
		//swap out image for new one
		updateImageFromRaw();
	}
	void qtPlot::updateImageFromRaw()
	{
		if(pixmapItem != NULL) 
		{
			graphicsScene->removeItem(pixmapItem);
			delete pixmapItem;
			pixmapItem = NULL;
		}
		for(std::vector<QGraphicsRectItem*>::iterator i = transparency.begin(); i != transparency.end(); i++)
		{
			graphicsScene->removeItem(*i);
			delete *i;
		}
		transparency.clear();
		
		const std::vector<int>& permutation = data->getCurrentPermutation();
		//Set up image data (accounting for padding
		/*		
		#pragma omp parallel for
		for(int i = 0; i < nMarkers; i++)
		{
			uchar* rawData = image->scanLine(i);
			for(int j = 0; j < nMarkers; j++)
			{
				rawData[j] = originalDataToChar[permutation[i] * nMarkers + permutation[j]];
			}
		}*/
		//Actually, a common use-case is that the start and end are (partially) the identity permutation, so do some optimisation for this case
		int countInitial = -1;
		do
		{
			countInitial++;
		}
		while(countInitial < nMarkers && permutation[countInitial] == countInitial);
		//now countInitial is the first entry in the permutation that's not the identity
		//we don't do anything if the whole thing is the identity
		if(countInitial == nMarkers) 
		{
			for(int i = 0; i < nMarkers; i++)
			{
				uchar* rawData = image->scanLine(i);
				memcpy(rawData, originalDataToChar, nMarkers);
			}
		}

		int indexFinal = nMarkers;
		do
		{
			indexFinal--;
		}
		while(indexFinal >= 0 && permutation[indexFinal] == indexFinal);

		for(int i = 0; i < countInitial; i++)
		{
			uchar* rawData = image->scanLine(i);
			memcpy(rawData + 0, &(originalDataToChar[permutation[i] * nMarkers + 0]), countInitial-1);
		}
		for(int i = nMarkers-1; i > indexFinal; i--)
		{
			uchar* rawData = image->scanLine(i);
			memcpy(rawData + indexFinal + 1, &(originalDataToChar[permutation[i] * nMarkers + indexFinal+1]), nMarkers - indexFinal - 1);
		}
		#pragma omp parallel for
		for(int i = 0; i < nMarkers; i++)
		{
			uchar* rawData = image->scanLine(i);
			if(i >= countInitial && i <= indexFinal)
			{
				for(int j = 0; j < nMarkers; j++)
				{
					rawData[j] = originalDataToChar[permutation[i] * nMarkers + permutation[j]];
				}
			}
			else
			{
				for(int j = countInitial; j <= indexFinal; j++)
				{
					rawData[j] = originalDataToChar[permutation[i] * nMarkers + permutation[j]];
				}
			}
		}
		//convert to pixmap
		QPixmap totalPixmap = QPixmap::fromImage(*image);
		pixmapItem = graphicsScene->addPixmap(totalPixmap);
		pixmapItem->setZValue(-1);

		//Add transparency / highlighting of different groups
		std::vector<int> groups = data->getCurrentGroups();
		std::vector<int>::iterator newEnd = std::unique(groups.begin(), groups.end());
		int nGroups = std::distance(groups.begin(), newEnd);
		QColor whiteColour("white");
		whiteColour.setAlphaF(0.3);
		QBrush whiteBrush(whiteColour);
		for(int i = 0; i < nGroups; i++)
		{
			int startGroup1 = data->startOfGroup(groups[i]);
			int endGroup1 = data->endOfGroup(groups[i]);
			for(int j = 0; j < nGroups; j++)
			{
				int startGroup2 = data->startOfGroup(groups[j]);
				int endGroup2 = data->endOfGroup(groups[j]);
				if(i %2 == j %2)
				{
					transparency.push_back(graphicsScene->addRect(startGroup1, startGroup2, endGroup1 - startGroup1, endGroup2 - startGroup2, QPen(Qt::NoPen), whiteBrush));
				}
			}
		}


		//signal redraw
		graphicsScene->update();
	}
	void qtPlot::undo()
	{
		data->undo();
		updateImageFromRaw();
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
		if(event->key() == Qt::Key_J && (event->modifiers() & Qt::ControlModifier))
		{
			QSharedPointer<qtPlotData> data = this->data;
			QPointF cursorPos = graphicsView->mapToScene(graphicsView->mapFromGlobal(QCursor::pos()));
			int x = cursorPos.x(), y = cursorPos.y();
			if(0 <= x && x < nMarkers && 0 <= y && y < nMarkers)
			{
				joinMarkers(x, y);
			}
		}
		if(event->key() == Qt::Key_U && (event->modifiers() & Qt::ControlModifier))
		{
			QSharedPointer<qtPlotData> data = this->data;
			undo();
		}
		QMainWindow::keyPressEvent(event);
	}
	void qtPlot::closeEvent(QCloseEvent* event)
	{
		event->accept();
	}
}