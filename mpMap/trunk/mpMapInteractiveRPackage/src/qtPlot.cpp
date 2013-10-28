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
#include <QMessageBox>
#include <QProgressBar>
#include "order.h"
#include <stdexcept>
#include <cmath>
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
		:originalMarkerNames(originalMarkerNames), currentMarkerNames(originalMarkerNames),originalGroups(originalGroups)
	{
		if(originalGroups.size() != originalMarkerNames.size()) throw std::runtime_error("Internal error");
		//set up identity permutation initially
		for(int i = 0; i < (int)originalGroups.size(); i++) identity.push_back(i);
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
			for(std::size_t i = 0; i < currentCumulativePermutation.size(); i++)
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

		for(std::size_t i = 0; i < permutation.size(); i++)
		{
			newCumulativePermutation[i] = currentCumulativePermutation[permutation[i]];
			currentMarkerNames[i] = originalMarkerNames[newCumulativePermutation[i]];
		}
		cumulativePermutations.push_back(newCumulativePermutation);
		groups.push_back(newGroups);
	}
	qtPlot::~qtPlot()
	{
		imageTiles.clear();
		delete[] imputedRawImageData;
		delete originalDataToChar;
		delete horizontalHighlight;
		delete verticalHighlight;
		delete intervalHighlight;
		delete singleHighlight;
		delete transparency;

		delete graphicsView;
		delete graphicsScene;
		delete joinGroupsLabel;
		delete group1Edit;
		delete group2Edit;
		delete orderAllExcept;
		delete groupsModeWidget;
		delete intervalModeWidget;
		delete singleModeWidget;
		delete auxillaryLabel;
		delete statusLabel;
		delete statusBar;
	}
	void qtPlot::addStatusBar()
	{
		statusBar = new QStatusBar();
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

		QLabel* orderLabel = new QLabel("Order all groups except (Ctrl + O)");
		orderLabel->setEnabled(true);
		orderLabel->setPalette(p);

		orderAllExcept = new QLineEdit;
		QRegExp intList(QString("(\\d+\\s*)*"));
		orderAllExcept->setValidator(new QRegExpValidator(intList));
		formLayout->addRow(orderLabel, orderAllExcept);
		return groupsModeWidget;
	}
	QFrame* qtPlot::createMode()
	{
		QFrame* comboContainer = new QFrame;
		QComboBox* comboMode = new QComboBox;
		comboMode->addItem("Groups");
		if(data->singleGroup())
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
	qtPlot::qtPlot(double* rawImageData, const std::vector<int>& originalGroups, const std::vector<std::string>& originalMarkerNames, double* auxData, int auxRows)
		:currentMode(Groups), horizontalGroup(-1), verticalGroup(-1), horizontalHighlight(NULL), verticalHighlight(NULL), intervalHighlight(NULL), singleHighlight(NULL), data(new qtPlotData(originalGroups, originalMarkerNames)), nOriginalMarkers((int)originalGroups.size()), rawImageData(rawImageData), imputedRawImageData(NULL), isFullScreen(false), highlightColour("blue"), startIntervalPos(-1), singleModePos(-1), auxData(auxData), auxRows(auxRows), computationMutex(QMutex::NonRecursive), transparency(NULL), orderAllExcept(NULL)
	{
		highlightColour.setAlphaF(0.3);
		int nMarkers = (int)originalGroups.size();
		initialiseImageData(nMarkers);
		QHBoxLayout* topLayout = new QHBoxLayout();
		graphicsScene = new QGraphicsScene();	
		graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
		
		//Add transparency quad
		QColor whiteColour("white");
		whiteColour.setAlphaF(0.4);
		QBrush whiteBrush(whiteColour);
		transparency = graphicsScene->addRect(0, 0, nMarkers, nMarkers, QPen(Qt::NoPen), whiteBrush);
		transparency->setZValue(0);
		
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
		group1 = std::atoi(group1Edit->text().toStdString().c_str());
		const std::vector<int>& currentGroups = data->getCurrentGroups();
		if(std::find(currentGroups.begin(), currentGroups.end(), group1) == currentGroups.end())
		{
			return;
		}
		group2 = std::atoi(group2Edit->text().toStdString().c_str());
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
	void qtPlot::signalMouseMove()
	{
		QPointF cursorPos = graphicsView->mapToScene(graphicsView->mapFromGlobal(QCursor::pos()));
		if(graphicsView->underMouse()) graphicsMouseMove(cursorPos);
	}
	void qtPlot::group1ReturnPressed()
	{
		int group = std::atoi(group1Edit->text().toStdString().c_str());
		const std::vector<int>& currentGroups = data->getCurrentGroups();
		if(std::find(currentGroups.begin(), currentGroups.end(), group) != currentGroups.end())
		{
			int start = data->startOfGroup(group);
			int end = data->endOfGroup(group);
			graphicsView->fitInView(start,start,end-start,end-start,Qt::KeepAspectRatioByExpanding);
			signalMouseMove();
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
			horizontalHighlight = NULL;
		}
		if(verticalGroup != -1)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(verticalHighlight));
			delete verticalHighlight;
			verticalHighlight = NULL;
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
			horizontalHighlight = NULL;
		}
		if(verticalGroup != -1)
		{
			graphicsScene->removeItem(static_cast<QGraphicsItem*>(verticalHighlight));
			delete verticalHighlight;
			verticalHighlight = NULL;
		}

		int firstHorizontalIndex = data->startOfGroup(newHorizontalGroup);
		int lastHorizontalIndex = data->endOfGroup(newHorizontalGroup);

		int firstVerticalIndex = data->startOfGroup(newVerticalGroup);
		int lastVerticalIndex = data->endOfGroup(newVerticalGroup);

		int nMarkers = data->getMarkerCount();
		horizontalHighlight = graphicsScene->addRect(0, firstHorizontalIndex, nMarkers, lastHorizontalIndex - firstHorizontalIndex, QPen(Qt::NoPen), highlightColour);
		verticalHighlight = graphicsScene->addRect(firstVerticalIndex, 0, lastVerticalIndex - firstVerticalIndex, nMarkers, QPen(Qt::NoPen), highlightColour);
		verticalHighlight->setZValue(2);
		horizontalHighlight->setZValue(2);
		
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
		intervalHighlight->setZValue(2);
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
			singleHighlight->setZValue(2);
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
	
		const std::vector<int>& permutation = data->getCurrentPermutation();
		const std::vector<int>& groups = data->getCurrentGroups();
		int nMarkers = data->getMarkerCount();
		
		std::vector<int> uniqueGroups = groups;
		//sort
		std::sort(uniqueGroups.begin(), uniqueGroups.end());
		//discard duplicates
		uniqueGroups.erase(std::unique(uniqueGroups.begin(), uniqueGroups.end()), uniqueGroups.end());
		
		//pre-cache some data, so it doesn't need to be recomputed in a deeply nested loop
		size_t nGroups = uniqueGroups.size();
		int* startGroups = new int[nGroups];
		int* endGroups = new int[nGroups];
		std::vector<std::vector<int> > expectedIndices;
		expectedIndices.resize(nGroups);
		for(size_t i = 0; i < nGroups; i++)
		{
			int currentGroup = uniqueGroups[i];
			startGroups[i] = data->startOfGroup(currentGroup);
			endGroups[i] = data->endOfGroup(currentGroup);

			std::vector<int>& currentGroupIndices = expectedIndices[i];
			currentGroupIndices.reserve(permutation.size());
			for(size_t j = 0; j != permutation.size(); j++)
			{
				if(groups[j] == currentGroup) currentGroupIndices.push_back(permutation[j]);
			}
		}
		
		//Go through the image data and delete any groups that have changed in their row / column member indices
		for(size_t rowGroupCounter = 0; rowGroupCounter < nGroups; rowGroupCounter++)
		{
			int rowGroup = uniqueGroups[rowGroupCounter];
			for(int columnGroupCounter = 0; columnGroupCounter < nGroups; columnGroupCounter++)
			{
				int columnGroup = uniqueGroups[columnGroupCounter];
				std::set<imageTile, imageTileComparer>::const_iterator located = imageTile::find(imageTiles, rowGroup, columnGroup);
				int startOfRowGroup = startGroups[rowGroupCounter], startOfColumnGroup = startGroups[columnGroupCounter];;

				/*for(int i = 0; i != permutation.size(); i++)
				{
					if(groups[i] == rowGroup) expectedRowIndices.push_back(permutation[i]);
					if(groups[i] == columnGroup) expectedColumnIndices.push_back(permutation[i]);
				}*/
				std::vector<int>& expectedRowIndices = expectedIndices[rowGroupCounter];
				std::vector<int>& expectedColumnIndices = expectedIndices[columnGroupCounter];

				if(located != imageTiles.end())
				{
					if(!located->checkIndices(expectedRowIndices, expectedColumnIndices))
					{
						graphicsScene->removeItem(located->getItem());
						imageTiles.erase(located);
					}
				}
				//Add in any rows / column that are now missing
				located = imageTile::find(imageTiles, rowGroup, columnGroup);
				if(located == imageTiles.end())
				{
					imageTile newTile = imageTile(originalDataToChar, nOriginalMarkers, rowGroup, columnGroup, expectedRowIndices, expectedColumnIndices, graphicsScene);
					imageTiles.insert(newTile);
				}
				//set position
				located = imageTile::find(imageTiles, rowGroup, columnGroup);
				if(located == imageTiles.end())
				{
					throw std::runtime_error("Internal error");
				}
				QGraphicsPixmapItem* currentItem = located->getItem();
				currentItem->setPos(startOfRowGroup, startOfColumnGroup);
			}
		}
		//Go through and remove unnecessary groups. Anything that doesn't match here just gets wiped
		std::set<imageTile>::iterator currentTile = imageTiles.begin();
		while(currentTile != imageTiles.end())
		{
			{
				int rowGroup = currentTile->getRowGroup(), columnGroup = currentTile->getColumnGroup();
				std::vector<int>::iterator findRowGroup = std::find(uniqueGroups.begin(), uniqueGroups.end(), rowGroup);
				std::vector<int>::iterator findColumnGroup = std::find(uniqueGroups.begin(), uniqueGroups.end(), columnGroup);
				//does the group still exist?
				if(findRowGroup == uniqueGroups.end() || findColumnGroup == uniqueGroups.end())
				{
					goto delete_tile;
				}
				int rowGroupIndexInAll = std::distance(uniqueGroups.begin(), findRowGroup);
				int columnGroupIndexInAll = std::distance(uniqueGroups.begin(), findColumnGroup);
				std::vector<int>& expectedRowIndices = expectedIndices[rowGroupIndexInAll];
				std::vector<int>& expectedColumnIndices = expectedIndices[columnGroupIndexInAll];
				//Are the row and column indices correct?
				/*{
					for(int i = 0; i != permutation.size(); i++)
					{
						if(groups[i] == rowGroup) newRowIndices.push_back(permutation[i]);
						if(groups[i] == columnGroup) newColumnIndices.push_back(permutation[i]);
					}*/
					if(!currentTile->checkIndices(expectedRowIndices, expectedColumnIndices)) goto delete_tile;
				//}
				QGraphicsPixmapItem* pixMapItem = currentTile->getItem();
				if(rowGroupIndexInAll %2 == columnGroupIndexInAll %2)
				{
					pixMapItem->setZValue(1);
				}
				else
				{
					pixMapItem->setZValue(-1);
				}
				currentTile++;
				continue;
			}
delete_tile:
			graphicsScene->removeItem(currentTile->getItem());
			std::set<imageTile>::iterator nextTile = currentTile;
			nextTile++;

			imageTiles.erase(currentTile);
			currentTile = nextTile;
			continue;

		}
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
		//convert to pixmap
		//QPixmap totalPixmap = QPixmap::fromImage(*image);
		//pixmapItem = graphicsScene->addPixmap(totalPixmap);
		//pixmapItem->setZValue(0);

		//Add transparency / highlighting of different groups
		/*QColor whiteColour("white");
		whiteColour.setAlphaF(0.4);
		QBrush whiteBrush(whiteColour);
		for(int i = 0; i < nGroups; i++)
		{
			int startGroup1 = startGroups[i];
			int endGroup1 = endGroups[i];
			for(int j = 0; j < nGroups; j++)
			{
				int startGroup2 = startGroups[j];
				int endGroup2 = endGroups[j];
				if(i %2 == j %2)
				{
					transparency.push_back(graphicsScene->addRect(startGroup1, startGroup2, endGroup1 - startGroup1, endGroup2 - startGroup2, QPen(Qt::NoPen), whiteBrush));
				}
			}
		}*/
		delete[] startGroups;
		delete[] endGroups;
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
	void qtPlot::doImputation()
	{
		if(imputedRawImageData == NULL) imputedRawImageData = new double[nOriginalMarkers * nOriginalMarkers];
		
		memcpy(imputedRawImageData, rawImageData, sizeof(double)*nOriginalMarkers * nOriginalMarkers);
		//a vector of linkage groups, which assigns a group to EVERY MARKER ORIGINALLY PRESENT
		const std::vector<int>& oldGroups = data->getCurrentGroups();
		int additionalGroupNumber = *std::max_element(oldGroups.begin(), oldGroups.end()) + 1;
		std::vector<int> newGroups(nOriginalMarkers, additionalGroupNumber);
		const std::vector<int>& currentPermutation = data->getCurrentPermutation();
		for(size_t i = 0; i < currentPermutation.size(); i++) newGroups[currentPermutation[i]] = oldGroups[i];
		std::string error;
		error.resize(200);
		bool ok = imputeInternal(imputedRawImageData, NULL, NULL, nOriginalMarkers, &(newGroups[0]), &(error[0]), 200);
		if(!ok) throw std::runtime_error("Imputation failed!");
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
			if(event->key() == Qt::Key_O && (event->modifiers() & Qt::ControlModifier))
			{
					if(attemptBeginComputation())
					{
						QMessageBox confirm;
						confirm.setText("This could take a while. Continue?");
						confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
						confirm.setDefaultButton(QMessageBox::No);
						int ret = confirm.exec();
						if(ret == QMessageBox::Yes)
						{
							//do the imputation again - If groups have been joined then we could end up with NAs in the recombination fraction matrix. Remember that the imputation only removes NAs between markers IN THE SAME GROUP, using the group structure as currently set. We need to assign a group to EVERY marker that was ORIGINALLY here. So everything that has been deleted, and therefore doesn't have a group, goes in (max(group) + 1). 
							doImputation();
							
							std::vector<int> uniqueGroups = data->getCurrentGroups();
							std::sort(uniqueGroups.begin(), uniqueGroups.end());
							uniqueGroups.erase(std::unique(uniqueGroups.begin(), uniqueGroups.end()), uniqueGroups.end());
							size_t nGroups = uniqueGroups.size();
							
							QRegExp intListRegex(QString("(\\d+)"));
							QString exceptionsText = orderAllExcept->text();
							int pos = 0;
							std::vector<int> exceptionsList;
							while((pos = intListRegex.indexIn(exceptionsText, pos))!= -1)
							{
								bool ok;
								exceptionsList.push_back(intListRegex.cap(1).toInt(&ok));
								pos += intListRegex.matchedLength();
							}

							QProgressBar* progress = new QProgressBar;
							statusBar->addWidget(progress);
							progress->setMinimum(0);
							progress->setMaximum(nGroups - exceptionsList.size());
							
							for(int groupCounter = 0; groupCounter < nGroups; groupCounter++)
							{
								if(std::find(exceptionsList.begin(), exceptionsList.end(), uniqueGroups[groupCounter]) != exceptionsList.end()) continue;
								const std::vector<int>& currentPermutation = data->getCurrentPermutation();
								int startOfGroup = data->startOfGroup(uniqueGroups[groupCounter]);
								int endOfGroup = data->endOfGroup(uniqueGroups[groupCounter]);
								int nSubMarkers = endOfGroup - startOfGroup;
								//The ordering code crashes with only one or two markers. So avoid that case. 
								if(nSubMarkers >= 3)
								{
									//get out the permutation that is going to be applied just to this bit
									std::vector<int> resultingPermutation;
									order(imputedRawImageData, nOriginalMarkers, currentPermutation, startOfGroup, endOfGroup, resultingPermutation);
								
									//extend this to a permutation that will be applied to the whole matrix
									std::vector<int> totalResultingPermutation;
									totalResultingPermutation.resize(nMarkers);
									//mostly it's the identity, because we're only ordering a small part of the matrix in this step
									for(int i = 0; i < nMarkers; i++)
									{
										totalResultingPermutation[i] = i;
									}
									//...but part is not the identity
									for(int i = 0; i < nSubMarkers; i++)
									{
										totalResultingPermutation[i + startOfGroup] = startOfGroup + resultingPermutation[i];
									}
									data->applyPermutation(totalResultingPermutation, data->getCurrentGroups());
								}
								progress->setValue(groupCounter+1);
							}
							statusBar->removeWidget(progress);
							
							//copied from applyPermutation
							updateImageFromRaw();
							setBoundingBox(data->getMarkerCount());

							delete progress;
						}
						endComputation();
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
					renewGroupsHighlighting(x, y);
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
					//we only need to do imputation here if it hasn't been done previously.
					//Reason: This option is only available if there's only one group. In which case no group joining is possible
					if(imputedRawImageData == NULL) doImputation();
					//permutation from just the submatrix
					std::vector<int> resultingPermutation;
					//the identity
					std::vector<int> identityPermutation;
					for(int i = 0; i < nMarkers; i++) identityPermutation.push_back(i);
					const std::vector<int>& currentPermutation = data->getCurrentPermutation();
					int start = std::min(this->startIntervalPos, this->endIntervalPos);
					int end = std::max(this->startIntervalPos, this->endIntervalPos);
					int nSubMarkers = end + 1 - start;
					//The ordering code crashes with only one or two markers
					if(nSubMarkers >= 3)
					{
						order(imputedRawImageData, nOriginalMarkers, currentPermutation, start, end+1, resultingPermutation);
						//and the conversion of the submatrix permutation to the bigger matrix
						std::vector<int> totalPermutation = identityPermutation;
						for(int i = 0; i < nSubMarkers; i++)
						{
							totalPermutation[i + start] = identityPermutation[start + resultingPermutation[i]];
						}
						applyPermutation(totalPermutation, data->getCurrentGroups());
					}
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
					//remove one element from groups vector
					const std::vector<int>& previousGroups = data->getCurrentGroups();
					std::vector<int> newGroups(nMarkers-1);
					std::copy(previousGroups.begin(), previousGroups.begin() + singleModePos, newGroups.begin());
					std::copy(previousGroups.begin() + singleModePos +1, previousGroups.end(), newGroups.begin() + singleModePos);
					applyPermutation(permutation, newGroups);
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
