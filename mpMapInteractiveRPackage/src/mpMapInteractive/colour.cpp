#include "colour.h"
namespace mpMap
{
	void constructColourTable(int n, QVector<QRgb>& vector)
	{
		vector.clear();
		QColor colour;
		//First n colours for actual values
		for(int counter = 0; counter < n; counter++)
		{
			colour.setHsvF(counter * (1.0f/6.0f)/(float) (n-1), 1.0, 1.0);
			vector.push_back(colour.rgb());
		}
		//and an additional for NA values
		const QColor black("black");
		vector.push_back(black.rgb());
		//Also add white (if we want to delete some columns / rows we'll want a smaller image, hence filling unused bit with white)
		const QColor white("white");
		vector.push_back(white.rgb());
	}
}