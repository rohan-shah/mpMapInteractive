#include <QtWidgets/QMainWindow>
#include "ZoomGraphicsView.h"
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QLineEdit>
#include <vector>
#include <QMutex>
#include <set>
#include "imageTileComparer.h"
#include "imageTile.h"
namespace mpMap
{
	struct qtPlotData
	{
		qtPlotData(const std::vector<int>& originalGroups, const std::vector<std::string>& originalMarkerNames);
		int startOfGroup(int group);
		int endOfGroup(int group);
		const std::vector<int>& getCurrentPermutation() const;
		const std::vector<int>& getCurrentGroups() const;
		void applyPermutation(const std::vector<int>& permutation, const std::vector<int>& newGroups);
		const std::vector<std::string> getCurrentMarkerNames() const;
		void undo();
		bool singleGroup();
		int getMarkerCount() const;
	private:
		qtPlotData(){};
		
		std::vector<std::string> originalMarkerNames;
		std::vector<std::string> currentMarkerNames;
		
		std::vector<std::vector<int> > cumulativePermutations;
		std::vector<std::vector<int> > groups;

		std::vector<int> originalGroups;
		std::vector<int> identity;
	};
	enum plotMode
	{
		Groups, Interval, Single
	};
	class qtPlot : public QMainWindow
	{
		Q_OBJECT
	public:
		~qtPlot();
		qtPlot(double* rawImageData, const std::vector<int>& groups, const std::vector<std::string>& markerNames, double* auxData, int auxRows);
		const qtPlotData& getData();
	protected:
		void closeEvent(QCloseEvent* event);
		void keyPressEvent(QKeyEvent* event);
		bool eventFilter(QObject *obj, QEvent *event);
	public slots:
		void group1ReturnPressed();
		void group2ReturnPressed();
		void graphicsLeaveEvent(QEvent*);
		void modeChanged(const QString&);
	private:
		void setBoundingBox(int nMarkers);
		void doImputation();

		void setIntervalHighlighting(int start, int end);
		void deleteIntervalHighlighting();

		void setSingleHighlighting(int pos);
		void deleteSingleHighlighting();
		
		void deleteGroupsHighlighting();
		void renewGroupsHighlighting(int x, int y);
		void signalMouseMove();
		void graphicsMouseMove(QPointF scenePos);
		
		plotMode currentMode;
		
		QFrame* addIntervalMode();
		QFrame* createMode();
		QWidget* addLeftSidebar();
		QFrame* addGroupsMode();
		QFrame* addSingleMode();

		void initialiseImageData(int nMarkers);
		void addStatusBar();
		void joinGroups(int x, int y);
		void undo();
		void updateImageFromRaw();
		void applyPermutation(const std::vector<int>& permutation, const std::vector<int>& newGroups);
		//The previously highlighted horizontal and vertical group
		int horizontalGroup, verticalGroup;
		QGraphicsRectItem* horizontalHighlight, *verticalHighlight;
		QGraphicsRectItem* intervalHighlight;
		QGraphicsRectItem* singleHighlight;
		//Some functions make structural changes to this. So to update the data set we just switch out this data object for a new one, which is an atomic operation. And functions that depend on the 
		//data make a copy and use that. 
		QSharedPointer<qtPlotData> data;

		//needed as the stride for the two double arrays below. 
		int nOriginalMarkers;
		double* rawImageData;
		double* imputedRawImageData;
		bool isFullScreen;
		ZoomGraphicsView* graphicsView;
		QLabel* statusLabel;
		QGraphicsScene* graphicsScene;
		std::set<imageTile, imageTileComparer> imageTiles;
		uchar* originalDataToChar;
		QLabel* joinGroupsLabel;
		QLineEdit* group1Edit;
		QLineEdit* group2Edit;
		QFrame* groupsModeWidget;
		QFrame* intervalModeWidget;
		QFrame* singleModeWidget;
		QColor highlightColour;
		int startIntervalPos, endIntervalPos;
		int singleModePos;
		QStatusBar* statusBar;

		//related to auxillary numeric data to be shown in status bar
		double* auxData;
		int auxRows;
		QLabel* auxillaryLabel;

		//we need a critical section around changes to the image state (basically anything that calls applyPermutation), because: The Ordering code calls into R, which will periodically
		//break out and process events, which will keep the window responsive. Which means that (for example) if you choose to order a large chunk of the image, and hit Ctrl + O again while it's doing this, it will 
		//call the ordering code again internally (from inside the first ordering computation). This is bade. 
		//We only want to be doing one bit of computation at a time. 
		QMutex computationMutex;
		bool attemptBeginComputation();
		void endComputation();
		
		
		QGraphicsRectItem* transparency;

		QLineEdit* orderAllExcept;
	};
}
