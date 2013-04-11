#include <QtWidgets/QMainWindow>
#include "ZoomGraphicsView.h"
#include <QLabel>
namespace mpMap
{
	class qtPlot : public QMainWindow
	{
		Q_OBJECT
	public:
		qtPlot(double* data, int* groups, int nMarkers);
	protected:
		void closeEvent(QCloseEvent* event);
		void keyPressEvent(QKeyEvent* event);
		bool eventFilter(QObject *obj, QEvent *event);
	private:
		double* data;
		int* groups;
		int nMarkers;
		bool isFullScreen;
		ZoomGraphicsView* graphicsView;
		QLabel* statusLabel;
	};
}