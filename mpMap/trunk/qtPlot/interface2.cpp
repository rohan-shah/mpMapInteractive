#include "qtPlot.h"
#include "interface2.h"
#include <QApplication>
namespace mpMap
{
	void plotQTImpl2(double* data, int* originalGroups_, int nMarkers, const std::vector<std::string>& markerNames, std::vector<std::string>& outputMarkerNames, std::vector<int>& outputGroups)
	{
		char* argv[] = {""};
		int argc = 0;
		QApplication app(argc, argv);
		
		std::vector<int> originalGroups(originalGroups_, originalGroups_+nMarkers);
		mpMap::qtPlot plot(data, originalGroups, nMarkers, markerNames);
		plot.show();
		app.exec();
		const qtPlotData& outputData = plot.getData();
		outputGroups = outputData.getCurrentGroups();
		outputMarkerNames = outputData.getCurrentMarkerNames();
	}
}