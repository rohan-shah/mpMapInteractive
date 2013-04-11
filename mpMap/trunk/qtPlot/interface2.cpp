#include "qtPlot.h"
#include "interface2.h"
#include <QApplication>
namespace mpMap
{
	void plotQTImpl2(double* data, int* groups, int nMarkers)
	{
		char* argv[] = {""};
		int argc = 0;
		QApplication app(argc, argv);
		
		mpMap::qtPlot plot(data, groups, nMarkers);
		plot.show();
		app.exec();
	}
}