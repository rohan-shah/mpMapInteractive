#include <QtWidgets/QMainWindow>
#include "ZoomGraphicsView.h"
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <vector>
namespace mpMap
{
	struct qtPlotData
	{
		qtPlotData(const std::vector<int>& originalGroups, const std::vector<std::string>& originalMarkerNames);
		int qtPlotData::startOfGroup(int group);
		int qtPlotData::endOfGroup(int group);
		const std::vector<int>& getCurrentPermutation() const;
		const std::vector<int>& getCurrentGroups() const;
		void applyPermutation(std::vector<int>& permutation, std::vector<int>& newGroups);
		const std::vector<std::string> getCurrentMarkerNames() const;
		void undo();
	private:
		int nMarkers;
		qtPlotData(){};
		
		std::vector<std::string> originalMarkerNames;
		std::vector<std::string> currentMarkerNames;
		
		std::vector<std::vector<int> > cumulativePermutations;
		std::vector<std::vector<int> > groups;

		std::vector<int> originalGroups;
		std::vector<int> identity;
	};
	void reorderImage(uchar* original, uchar* output, int nMarkers);
	class qtPlot : public QMainWindow
	{
		Q_OBJECT
	public:
		~qtPlot();
		qtPlot(double* rawImageData, const std::vector<int>& groups, int nMarkers, const std::vector<std::string>& markerNames);
		const qtPlotData& getData();
	protected:
		void closeEvent(QCloseEvent* event);
		void keyPressEvent(QKeyEvent* event);
		bool eventFilter(QObject *obj, QEvent *event);
	private:
		QSharedPointer<QImage> image;
		void joinMarkers(int x, int y);
		void undo();
		void renewHighlighting(int x, int y);
		void updateImageFromRaw();
		void applyPermutation(std::vector<int>& permutation, std::vector<int>& newGroups);
		//The previously highlighted horizontal and vertical group
		int horizontalGroup, verticalGroup;
		QGraphicsRectItem* horizontalHighlight, *verticalHighlight;
		//Some functions make structural changes to this. So to update the data set we just switch out this data object for a new one, which is an atomic operation. And functions that depend on the 
		//data make a copy and use that. 
		QSharedPointer<qtPlotData> data;
		double* rawImageData;
		int nMarkers;
		bool isFullScreen;
		ZoomGraphicsView* graphicsView;
		QLabel* statusLabel;
		QGraphicsScene* graphicsScene;
		QGraphicsPixmapItem* pixmapItem;
		uchar* originalDataToChar;
		static const int nColours = 100;
		std::vector<QGraphicsRectItem*> transparency;
	};
}