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
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QStandardItemModel>
#include <QApplication>
#include "order.h"
namespace mpMap
{
	int qtPlotData::getMarkerCount() const
	{
		if(cumulativePermutations.size() == 0)
		{
			return (int)originalMarkerNames.size();
		}
		return (int)cumulativePermutations[cumulativePermutations.size()-1].size();
	}
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
	bool qtPlotData::singleGroup()
	{
		const std::vector<int>& currentGroups = getCurrentGroups();
		int group = currentGroups[0];
		for(std::vector<int>::const_iterator i = currentGroups.begin(); i != currentGroups.end(); i++)
		{
			if(*i != group) return false;
		}
		return true;
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
		:originalGroups(originalGroups), originalMarkerNames(originalMarkerNames), currentMarkerNames(originalMarkerNames)
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
			currentMarkerNames.clear();
			currentMarkerNames.resize(currentCumulativePermutation.size());
			for(int i = 0; i < currentCumulativePermutation.size(); i++)
			{
				currentMarkerNames[i] = originalMarkerNames[currentCumulativePermutation[i]];
			}
		}
	}
	void qtPlotData::applyPermutation(const std::vector<int>& permutation, const std::vector<int>& newGroups)
	{
		const std::vector<int>& currentCumulativePermutation = getCurrentPermutation();
		std::vector<int> newCumulativePermutation; 
		newCumulativePermutation.resize(permutation.size());

		currentMarkerNames.resize(permutation.size());

		for(int i = 0; i < permutation.size(); i++)
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
	void qtPlot::addStatusBar()
	{
		QStatusBar* statusBar = new QStatusBar();
		statusLabel = new QLabel();
		statusLabel->setText("");
		statusBar->addPermanentWidget(statusLabel);

		auxillaryLabel = new QLabel();
		auxillaryLabel->setText("");
		statusBar->addWidget(auxillaryLabel);
		auxillaryLabel->setTextFormat(Qt::RichText);

		setStatusBar(statusBar);
	}
	void qtPlot::initialiseImageData(int nMarkers)
	{
		//this is the image that contains the current data. As opposed to originalDatToChar, which contains the data IN THE ORIGINAL ORDERING
		image = QSharedPointer<QImage>(new QImage(nMarkers, nMarkers, QImage::Format_Indexed8));
		//get 100 colours
		QVector<QRgb> colours;
		constructColourTable(nColours, colours);
		image->setColorTable(colours);

		originalDataToChar = new uchar[nMarkers * nMarkers];
		//scale data from float to integer
		for(int i = 0; i < nMarkers; i++)
		{
			for(int j = 0; j < nMarkers; j++)
			{
				double pixelVal = rawImageData[i * nMarkers + j];
				//NA values have different colour
				if(pixelVal != pixelVal)
				{
					//The nColours + 1 indexed colour is the NA colour
					originalDataToChar[i * nMarkers + j] = (uchar)nColours;
				}
				else originalDataToChar[i * nMarkers + j] = (uchar)std::floor(0.5f + (nColours - 1)* pixelVal / 0.5);
			}
		}
	}
	QFrame* qtPlot::addIntervalMode()
	{
		QFrame* intervalModeWidget = new QFrame;
		QFormLayout* formLayout = new QFormLayout;
		
		QLabel* undoLabel = new QLabel(QString("Undo (Ctrl + U)"));
		//set up pallete to highlight enabled labels / shortcuts
		QPalette p = undoLabel->palette();
		p.setColor(QPalette::Active, QPalette::WindowText, QColor("blue"));
		undoLabel->setPalette(p);
		formLayout->addRow(undoLabel, new QLabel(""));

		QLabel* orderLabel = new QLabel(QString("Order (Ctrl + O)"));
		orderLabel->setPalette(p);
		formLayout->addRow(orderLabel, new QLabel(""));

		QLabel* reverseLabel = new QLabel(QString("Reverse (Ctrl + R)"));
		orderLabel->setPalette(p);
		formLayout->addRow(reverseLabel, new QLabel(""));

		intervalModeWidget->setLayout(formLayout);
		return intervalModeWidget;
	}
	QFrame* qtPlot::addGroupsMode()
	{
		QFrame* groupsModeWidget = new QFrame;
		//set up layout on left hand side for labels / inputs
		QFormLayout* formLayout = new QFormLayout;

		QLabel* undoLabel = new QLabel(QString("Undo (Ctrl + U)"));
		//set up pallete to highlight enabled labels / shortcuts
		QPalette p = undoLabel->palette();
		p.setColor(QPalette::Active, QPalette::WindowText, QColor("blue"));
		undoLabel->setPalette(p);
		formLayout->addRow(undoLabel, new QLabel(""));

		joinGroupsLabel = new QLabel("Join groups (Ctrl + j)");
		joinGroupsLabel->setEnabled(false);
		joinGroupsLabel->setPalette(p);
		formLayout->addRow(joinGroupsLabel, new QLabel(""));

		QHBoxLayout* gotoLayout = new QHBoxLayout;
		group1Edit = new QLineEdit;
		group1Edit->setValidator(new QIntValidator());
		group2Edit = new QLineEdit;
		group2Edit->setValidator(new QIntValidator());

		setTabOrder(group1Edit, group2Edit);
		setTabOrder(group2Edit, group1Edit);
		
		QObject::connect(group1Edit, SIGNAL(returnPressed()), this, SLOT(group1ReturnPressed()));
		QObject::connect(group2Edit, SIGNAL(returnPressed()), this, SLOT(group2ReturnPressed()));
		gotoLayout->addWidget(group1Edit);
		gotoLayout->addWidget(group2Edit);
		QLabel* gotoLabel = new QLabel("Goto groups (Ctrl + G)");
		gotoLabel->setPalette(p);
		formLayout->addRow(gotoLabel, gotoLayout);
		groupsModeWidget->setLayout(formLayout);
		return groupsModeWidget;
	}
	QFrame* qtPlot::createMode()
	{
		QFrame* comboContainer = new QFrame;
		QComboBox* comboMode = new QComboBox;
		comboMode->addItem("Groups");
		if(data->singleGroup() && imputedRawImageData != NULL)
		{
			comboMode->addItem("Single marker");
			comboMode->addItem("Interval");
		}
		QObject::connect(comboMode, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(modeChanged(const QString&)));

		QFormLayout* modeLayout = new QFormLayout;
		modeLayout->addRow("Mode", comboMode);
		comboContainer->setLayout(modeLayout);
		return comboContainer;
	}
	void qtPlot::modeChanged(const QString& mode)
	{
		if(mode == "Groups")
		{
			currentMode = Groups;
			deleteIntervalHighlighting();
			deleteSingleHighlighting();
			groupsModeWidget->show();
			intervalModeWidget->hide();
			singleModeWidget->hide();
		}
		else if(mode == "Interval")
		{
			currentMode = Interval;
			deleteGroupsHighlighting();
			deleteSingleHighlighting();
			groupsModeWidget->hide();
			singleModeWidget->hide();
			intervalModeWidget->show();
		}
		else
		{
			currentMode = Single;
			singleModeWidget->show();
			groupsModeWidget->hide();
			intervalModeWidget->hide();

			deleteIntervalHighlighting();
			deleteGroupsHighlighting();
		}
	}
	QWidget* qtPlot::addLeftSidebar()
	{
		QWidget* leftSidebar = new QWidget;
		QVBoxLayout* sidebarLayout = new QVBoxLayout;
		QFrame* modeWidget = createMode();

		groupsModeWidget = addGroupsMode();
		intervalModeWidget = addIntervalMode();
		intervalModeWidget->hide();

		singleModeWidget = addSingleMode();
		singleModeWidget->hide();

		sidebarLayout->setAlignment(Qt::AlignTop);
		
		sidebarLayout->addWidget(modeWidget, 0, Qt::AlignTop);
		sidebarLayout->addSpacing(1);
		sidebarLayout->addWidget(groupsModeWidget, 1, Qt::AlignTop);
		sidebarLayout->addWidget(intervalModeWidget, 1, Qt::AlignTop);
		sidebarLayout->addWidget(singleModeWidget, 1, Qt::AlignTop);
		
		leftSidebar->setLayout(sidebarLayout);
		leftSidebar->setMinimumWidth(400);
		return leftSidebar;
	}
	void qtPlot::setBoundingBox(int nMarkers)
	{
		//leave some space around the outside, when zooming. 
		QRectF bounding;
		bounding.setX(0 - nMarkers/20.0);
		bounding.setY(0 - nMarkers/20.0);
		bounding.setWidth(nMarkers + nMarkers/10.0);
		bounding.setHeight(nMarkers + nMarkers/10.0);
		graphicsView->setSceneRect(bounding);
	}
	qtPlot::qtPlot(double* rawImageData, double* imputedRawImageData, const std::vector<int>& originalGroups, const std::vector<std::string>& originalMarkerNames, double* auxData, int auxRows)
		:rawImageData(rawImageData), data(new qtPlotData(originalGroups, originalMarkerNames)), isFullScreen(false), horizontalGroup(-1), verticalGroup(-1), pixmapItem(NULL), horizontalHighlight(NULL), verticalHighlight(NULL), currentMode(Groups), startIntervalPos(-1), 
		intervalHighlight(NULL), highlightColour("blue"), imputedRawImageData(imputedRawImageData), computationMutex(QMutex::NonRecursive), singleModePos(-1), nOriginalMarkers((int)originalGroups.size()), singleHighlight(NULL), auxData(auxData), auxRows(auxRows)
	{
		highlightColour.setAlphaF(0.3);
		int nMarkers = (int)originalGroups.size();
		initialiseImageData(nMarkers);
		QHBoxLayout* topLayout = new QHBoxLayout();
		graphicsScene = new QGraphicsScene();	
		graphicsView = new ZoomGraphicsView(graphicsScene);
		updateImageFromRaw();
		graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		//mouse move events should be handled at a higher level
		graphicsScene->installEventFilter(this);
		graphicsView->viewport()->installEventFilter(this);
		
		setBoundingBox(nMarkers);

		addStatusBar();

		QWidget* sidebarWidget = addLeftSidebar();

		//add form layout to top level layout (same level as the graphics view)
		topLayout->addWidget(sidebarWidget, 0);
		topLayout->addWidget(graphicsView);
		topLayout->setStretchFactor(graphicsView, 1);

		//no margins needed
		QWidget* topLayoutWidget = new QWidget;
		topLayout->setContentsMargins(0,0,0,0);
		topLayoutWidget->setContentsMargins(0,0,0,0);
		topLayoutWidget->setLayout(topLayout);
		setCentralWidget(topLayoutWidget);
		//setCentralWidget(graphicsView);
		graphicsView->setFocus();
	}
	void qtPlot::graphicsLeaveEvent(QEvent*)
	{
		statusLabel->setText(QString(""));
		auxillaryLabel->setText(QString(""));
	}
	void qtPlot::group2ReturnPressed()
	{
		int group1, group2;
		try
		{
			group1 = std::stoi(group1Edit->text().toStdString());
			const std::vector<int>& currentGroups = data->getCurrentGroups();
			if(std::find(currentGroups.begin(), currentGroups.end(), group1) == currentGroups.end())
			{
				return;
			}
			group2 = std::stoi(group2Edit->text().toStdString());
			if(std::find(currentGroups.begin(), currentGroups.end(), group2) == currentGroups.end())
			{
				return;
			}
			int start1 = data->startOfGroup(group1);
			int end1 = data->endOfGroup(group1);
			int start2 = data->startOfGroup(group2);
			int end2 = data->endOfGroup(group2);
			graphicsView->fitInView(start2,start1,end2-start2,end1-start1,Qt::KeepAspectRatio);
			signalMouseMove();
		}
		catch(...)
		{
			return;
		}
	}
	void qtPlot::signalMouseMove()
	{
		QPointF cursorPos = graphicsView->mapToScene(graphicsView->mapFromGlobal(QCursor::pos()));
		if(graphicsView->underMouse()) graphicsMouseMove(cursorPos);
	}
	void qtPlot::group1ReturnPressed()
	{
		try
		{
			int group = std::stoi(group1Edit->text().toStdString());
			const std::vector<int>& currentGroups = data->getCurrentGroups();
			if(std::find(currentGroups.begin(), currentGroups.end(), group) != currentGroups.end())
			{
				int start = data->startOfGroup(group);
				int end = data->endOfGroup(group);
				graphicsView->fitInView(start,start,end-start,end-start,Qt::KeepAspectRatioByExpanding);
				signalMouseMove();
			}
		}
		catch(...)
		{
		}
	}
	const qtPlotData& qtPlot::getData()
	{
		return *data;
	}
	void qtPlot::deleteGroupsHighlighting()
	{
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
		horizontalGroup = -1;
		verticalGroup = -1;
	}
	void qtPlot::renewGroupsHighlighting(int x, int y)
	{
		//are we not highlighting anything?
		if(x == -1 || y == -1)
		{
			//is this different from previous state?
			if(horizontalGroup == -1 || verticalGroup == -1)
			{
				return;
			}
			deleteGroupsHighlighting();
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

		int nMarkers = data->getMarkerCount();
		horizontalHighlight = graphicsScene->addRect(0, firstHorizontalIndex, nMarkers, lastHorizontalIndex - firstHorizontalIndex, QPen(Qt::NoPen), highlightColour);
		verticalHighlight = graphicsScene->addRect(firstVerticalIndex, 0, lastVerticalIndex - firstVerticalIndex, nMarkers, QPen(Qt::NoPen), highlightColour);
		
		horizontalGroup = newHorizontalGroup;
		verticalGroup = newVerticalGroup;
		
		graphicsScene->update();
	}
	void qtPlot::setIntervalHighlighting(int start, int end)
	{
		deleteIntervalHighlighting();
		int nMarkers = data->getMarkerCount();
		if(start < end)
		{
			intervalHighlight = graphicsScene->addRect(start, 0, end - start + 1, nMarkers, QPen(Qt::NoPen), highlightColour);
		}
		else
		{
			intervalHighlight = graphicsScene->addRect(end, 0, start - end + 1, nMarkers, QPen(Qt::NoPen), highlightColour);
		}
		startIntervalPos = start;
		endIntervalPos = end;
		intervalHighlight->setZValue(-1);
	}
	bool qtPlot::eventFilter(QObject* object, QEvent *event)
	{
		QSharedPointer<qtPlotData> data = this->data;
		if(object == graphicsScene && event->type() == QEvent::GraphicsSceneMouseMove)
		{
			QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
			QPointF scenePos = mouseEvent->scenePos();
			graphicsMouseMove(scenePos);
			return true;
		}
		if(currentMode == Interval)
		{
			if(object == graphicsScene && event->type() == QEvent::GraphicsSceneMousePress)
			{
				int nMarkers = data->getMarkerCount();
				QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
				QPointF scenePos = mouseEvent->scenePos();
				qreal x_ = scenePos.x(), y_ = scenePos.y();
				int x = (int)(x_  + 0), y = (int)(y_ + 0);
				if(x < 0) x = 0;
				if(x >= nMarkers) x = nMarkers - 1;
				if(y < 0) y = 0; 
				if(y >= nMarkers) y = nMarkers - 1;
				Qt::MouseButtons pressed = mouseEvent->buttons();
				if(pressed & Qt::RightButton)
				{
					deleteIntervalHighlighting();
				}
				else if(pressed & Qt::LeftButton)
				{
					if(QApplication::keyboardModifiers() & Qt::ShiftModifier && startIntervalPos > -1)
					{
						endIntervalPos = x;
						setIntervalHighlighting(startIntervalPos, endIntervalPos);
					}
					else
					{
						deleteIntervalHighlighting();
						startIntervalPos = x;
						endIntervalPos = -1;
					}
				}
				return true;
			}
		}
		if(currentMode == Single)
		{
			if(object == graphicsScene && event->type() == QEvent::GraphicsSceneMousePress)
			{
				int nMarkers = data->getMarkerCount();
				QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
				QPointF scenePos = mouseEvent->scenePos();
				qreal x_ = scenePos.x(), y_ = scenePos.y();
				int x = (int)(x_  + 0), y = (int)(y_ + 0);

				Qt::MouseButtons pressed = mouseEvent->buttons();
				if(pressed & Qt::RightButton)
				{
					deleteSingleHighlighting();
				}
				else if(x >= 0 && x < nMarkers && y >= 0 && y < nMarkers && pressed & Qt::LeftButton)
				{
					singleModePos = x;
					setSingleHighlighting(singleModePos);
				}
				return true;
			}
		}
		if(event->type() == QEvent::Leave)
		{
			statusLabel->setText(QString(""));
			if(currentMode == Groups) deleteGroupsHighlighting();
			return true;
		}
		return false;
	}
	void qtPlot::setSingleHighlighting(int pos)
	{
		deleteSingleHighlighting();
		if(pos > -1)
		{
			int nMarkers = data->getMarkerCount();
			singleHighlight = graphicsScene->addRect(pos, 0, 1, nMarkers, QPen(Qt::NoPen), highlightColour);
			singleHighlight->setZValue(-1);
			singleModePos = pos;
		}
	}
	void qtPlot::deleteSingleHighlighting()
	{
		if(singleHighlight != NULL)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(singleHighlight));
			delete singleHighlight;
			singleHighlight = NULL;
		}
		singleModePos = -1;
	}
	void qtPlot::graphicsMouseMove(QPointF scenePos)
	{
		const double threshold = 1e-5;
		qreal x_ = scenePos.x(), y_ = scenePos.y();
		int x = (int)(x_  + 0), y = (int)(y_ + 0);
		std::stringstream ss;
		int nMarkers = data->getMarkerCount();
		if(0 <= x && x < nMarkers && 0 <= y && y < nMarkers)
		{
			const std::vector<int>& currentPermutation = data->getCurrentPermutation();
			int xMarker = currentPermutation[x];
			int yMarker = currentPermutation[y];
			if(auxData != NULL)
			{
				std::stringstream aux_ss;
				aux_ss << "Column = (";
				for(int i = 0; i < auxRows-1; i++)
				{
					double value = (auxData + xMarker * auxRows)[i];
					if(value < 1e-5)
					{
						aux_ss << "<b>" << value << "</b>, ";
					}
					else aux_ss << value << ", ";
				}
				double value = (auxData + xMarker * auxRows)[auxRows-1];
				if(value < 1e-5)
				{
					aux_ss << "<b>" << value << "</b>";
				}
				else aux_ss << value;
				aux_ss << "), Row = (";
				for(int i = 0; i < auxRows-1; i++)
				{
					double value = (auxData + yMarker * auxRows)[i];
					if(value < threshold)
					{
						aux_ss << "<b>" << value << "</b>, ";
					}
					else aux_ss << value << ", ";
				}
				value = (auxData + yMarker * auxRows)[auxRows-1];
				if(value < 1e-5)
				{
					aux_ss << "<b>" << value << "</b>";
				}
				else aux_ss << value;
				aux_ss << ")";
				auxillaryLabel->setText(QString(aux_ss.str().c_str()));
			}
			const std::vector<std::string> currentMarkerNames = data->getCurrentMarkerNames();
			const std::vector<int> currentGroups = data->getCurrentGroups();
			ss << "Markers (column = " << currentMarkerNames[x] << ", row = " << currentMarkerNames[y] << ")\t\t\t";
			ss << "Groups (column = " << currentGroups[x] << ", row = " << currentGroups[y] << ")\t\t\t";
			
		}
		ss << "position (" << x << ", " << y << ")";
		statusLabel->setText(QString(ss.str().c_str()));
		if(currentMode == Groups)
		{
			if(0 <= x && x < nMarkers && 0 <= y && y < nMarkers)
			{
				renewGroupsHighlighting(x, y);
				const std::vector<int> currentGroups = data->getCurrentGroups();
				joinGroupsLabel->setEnabled(currentGroups[y] != currentGroups[x]);
			}
			else
			{
				deleteGroupsHighlighting();
				joinGroupsLabel->setEnabled(false);
			}

		}
	}
	void qtPlot::joinGroups(int x, int y)
	{
		int nMarkers = data->getMarkerCount();
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
	void qtPlot::applyPermutation(const std::vector<int>& permutation, const std::vector<int>& newGroups)
	{
		data->applyPermutation(permutation, newGroups);
		//swap out image for new one
		updateImageFromRaw();
		setBoundingBox(data->getMarkerCount());
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
		
		image->fill(QColor("white"));
		const std::vector<int>& permutation = data->getCurrentPermutation();
		int nMarkers = data->getMarkerCount();
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
		while(indexFinal >= 0 && permutation[indexFinal] == indexFinal + (nOriginalMarkers - nMarkers));
		//now indexFinal is the first (counting backwards from the end) index which is not the identity
		for(int i = 0; i < countInitial; i++)
		{
			uchar* rawData = image->scanLine(i);
			memcpy(rawData + 0, &(originalDataToChar[i * nOriginalMarkers + 0]), countInitial);
			memcpy(rawData + indexFinal + 1, &(originalDataToChar[i * nOriginalMarkers + indexFinal+1 + (nOriginalMarkers - nMarkers)]), nMarkers - indexFinal - 1);
		}
		for(int i = nMarkers-1; i > indexFinal; i--)
		{
			uchar* rawData = image->scanLine(i);
			memcpy(rawData + 0, &(originalDataToChar[(i + (nOriginalMarkers - nMarkers)) * nOriginalMarkers + 0]), countInitial);
			memcpy(rawData + indexFinal + 1, &(originalDataToChar[(i + (nOriginalMarkers - nMarkers)) * nOriginalMarkers + indexFinal+1 + (nOriginalMarkers - nMarkers)]), nMarkers - indexFinal - 1);
		}
		#pragma omp parallel for
		for(int i = 0; i < nMarkers; i++)
		{
			uchar* rawData = image->scanLine(i);
			if(i >= countInitial && i <= indexFinal)
			{
				for(int j = 0; j < nMarkers; j++)
				{
					rawData[j] = originalDataToChar[permutation[i] * nOriginalMarkers + permutation[j]];
				}
			}
			else
			{
				for(int j = countInitial; j <= indexFinal; j++)
				{
					rawData[j] = originalDataToChar[permutation[i] * nOriginalMarkers + permutation[j]];
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
		whiteColour.setAlphaF(0.4);
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

		setBoundingBox(nMarkers);
		//signal redraw
		graphicsScene->update();
	}
	void qtPlot::undo()
	{
		data->undo();
		updateImageFromRaw();
		if(currentMode == Groups) 
		{
			//if there's been a structural change, we HAVE to redo the highlighting, irrespective of whether the highlighted group number is the same
			deleteGroupsHighlighting();
			signalMouseMove();
		}
	}
	void qtPlot::keyPressEvent(QKeyEvent* event)
	{
		int nMarkers = data->getMarkerCount();
		if(currentMode == Groups)
		{
			if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
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
					QPointF cursorPos = graphicsView->mapToScene(graphicsView->mapFromGlobal(QCursor::pos()));
					if(graphicsView->underMouse()) graphicsMouseMove(cursorPos);
					return;
				}
			}
			if(event->key() == Qt::Key_J && (event->modifiers() & Qt::ControlModifier))
			{
				QSharedPointer<qtPlotData> data = this->data;
				QPointF cursorPos = graphicsView->mapToScene(graphicsView->mapFromGlobal(QCursor::pos()));
				int x = cursorPos.x(), y = cursorPos.y();
				if(0 <= x && x < nMarkers && 0 <= y && y < nMarkers && attemptBeginComputation())
				{
					joinGroups(x, y);
					endComputation();
				}
			}
			if(event->key() == Qt::Key_G && (event->modifiers() & Qt::ControlModifier))
			{
				group1Edit->setFocus();
			}
		}
		else if(currentMode == Interval)
		{
			if(event->key() == Qt::Key_O && (event->modifiers() & Qt::ControlModifier))
			{
				//See documentation for attemptBeginComputation
				if(startIntervalPos > -1 && endIntervalPos > -1 && attemptBeginComputation())
				{
					//permutation from just the submatrix
					std::vector<int> resultingPermutation;
					//the identity
					std::vector<int> identityPermutation;
					for(int i = 0; i < nMarkers; i++) identityPermutation.push_back(i);
					const std::vector<int>& currentPermutation = data->getCurrentPermutation();
					int start = std::min(this->startIntervalPos, this->endIntervalPos);
					int end = std::max(this->startIntervalPos, this->endIntervalPos);
					int nSubMarkers = end + 1 - start;
					order(imputedRawImageData, nOriginalMarkers, currentPermutation, start, end+1, resultingPermutation);
					//and the conversion of the submatrix permutation to the bigger matrix
					std::vector<int> totalPermutation = identityPermutation;
					for(int i = 0; i < nSubMarkers; i++)
					{
						totalPermutation[i + start] = identityPermutation[start + resultingPermutation[i]];
					}
					applyPermutation(totalPermutation, data->getCurrentGroups());
					endComputation();
				}
			}
			else if(event->key() == Qt::Key_R && (event->modifiers() & Qt::ControlModifier))
			{
				if(startIntervalPos > -1 && endIntervalPos > -1 && attemptBeginComputation()) 
				{
					int start = std::min(this->startIntervalPos, this->endIntervalPos);
					int end = std::max(this->startIntervalPos, this->endIntervalPos);
					std::vector<int> resultingPermutation;
					for(int i = 0; i < nMarkers; i++) resultingPermutation.push_back(i);
					for(int i = start; i <= end; i++) resultingPermutation[i] = end - (i-start);
					applyPermutation(resultingPermutation, data->getCurrentGroups());
					endComputation();
				}
			}

		}
		else if(currentMode == Single)
		{
			if(event->key() == Qt::Key_Delete)
			{
				if(singleModePos > -1 && attemptBeginComputation())
				{
					std::vector<int> permutation;
					int nMarkers = data->getMarkerCount();
					for(int i = 0; i < nMarkers; i++)
					{
						if(i != singleModePos) permutation.push_back(i);
					}
					applyPermutation(permutation, data->getCurrentGroups());
					nMarkers = data->getMarkerCount();
					if(singleModePos >= nMarkers) singleModePos -= 1;
					setSingleHighlighting(singleModePos);
					endComputation();
				}
			}
		}
		if(event->key() == Qt::Key_U && (event->modifiers() & Qt::ControlModifier))
		{
			QSharedPointer<qtPlotData> data = this->data;
			undo();
		}
		QMainWindow::keyPressEvent(event);
	}
	QFrame* qtPlot::addSingleMode()
	{
		QFrame* singleModeWidget = new QFrame;
		QFormLayout* formLayout = new QFormLayout;
		
		QLabel* undoLabel = new QLabel(QString("Undo (Ctrl + U)"));
		//set up pallete to highlight enabled labels / shortcuts
		QPalette p = undoLabel->palette();
		p.setColor(QPalette::Active, QPalette::WindowText, QColor("blue"));
		undoLabel->setPalette(p);
		formLayout->addRow(undoLabel, new QLabel(""));

		QLabel* deleteLabel = new QLabel(QString("Delete (Del)"));
		deleteLabel->setPalette(p);
		formLayout->addRow(deleteLabel, new QLabel(""));

		singleModeWidget->setLayout(formLayout);
		return singleModeWidget;
	}
	bool qtPlot::attemptBeginComputation()
	{
		if(computationMutex.tryLock())
		{
			return true;
		}
		return false;
	}
	void qtPlot::endComputation()
	{
		computationMutex.unlock();
	}
	void qtPlot::closeEvent(QCloseEvent* event)
	{
		event->accept();
	}
	void qtPlot::deleteIntervalHighlighting()
	{
		if(intervalHighlight != NULL)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(intervalHighlight));
			delete intervalHighlight;
			intervalHighlight = NULL;
		}
		startIntervalPos = endIntervalPos = -1;
	}
}