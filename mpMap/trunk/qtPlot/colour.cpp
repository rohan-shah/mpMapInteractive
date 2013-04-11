#include "colour.h"
void constructColourTable(int n, QVector<QRgb>& vector)
{
	vector.clear();
	QColor colour;
	for(int counter = 0; counter < n; counter++)
	{
		colour.setHsvF(counter * (1.0f/6.0f)/(float) (n-1), 1.0, 1.0);
		vector.push_back(colour.rgb());
	}
}