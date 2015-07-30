#include "imageTile.h"
#include "colour.h"
namespace mpMapInteractive
{
	imageTile::~imageTile()
	{
	}
	imageTile::imageTile(uchar* data, int dataRows, int rowGroup, int columnGroup, const std::vector<int>& rowIndices, const std::vector<int>& columnIndices, QGraphicsScene* graphicsScene)
	:rowIndices(rowIndices), columnIndices(columnIndices), rowGroup(rowGroup), columnGroup(columnGroup)
	{
		QImage* image = new QImage((int)rowIndices.size(), (int)columnIndices.size(), QImage::Format_Indexed8);
		//get 100 colours
		QVector<QRgb> colours;
		constructColourTable(nColours, colours);
		image->setColorTable(colours);
		
		for(size_t j = 0; j < columnIndices.size(); j++)
		{
			uchar* reorderedData = image->scanLine((int)j);
			for(size_t i = 0; i < rowIndices.size(); i++)
			{
				reorderedData[i] = data[columnIndices[j] * dataRows + rowIndices[i]];
			}
		}
		QPixmap pixMap = QPixmap::fromImage(*image);
		pixMapItem = QSharedPointer<QGraphicsPixmapItem>(graphicsScene->addPixmap(pixMap));
		delete image;
	}
	imageTile::imageTile()
		:pixMapItem(NULL)
	{
	}
	bool imageTile::checkIndices(const std::vector<int>& otherRowIndices, const std::vector<int>& otherColumnIndices) const
	{
		if(rowIndices.size() != otherRowIndices.size() || columnIndices.size() != otherColumnIndices.size())
		{
				return false;
		}
		for(size_t i = 0; i < otherRowIndices.size(); i++)
		{
			if(otherRowIndices[(int)i] != rowIndices[(int)i]) return false;
		}
		for(size_t i = 0; i < otherColumnIndices.size(); i++)
		{
			if(otherColumnIndices[i] != columnIndices[i]) return false;
		}
		return true;
	}
	std::set<imageTile>::const_iterator imageTile::find(const std::set<imageTile, imageTileComparer>& collection, int rowGroup, int columnGroup)
	{
		imageTile toFind;
		toFind.rowGroup = rowGroup;
		toFind.columnGroup = columnGroup;
		return collection.find(toFind);
	}
	const std::vector<int>& imageTile::getRowIndices() const
	{
		return rowIndices;
	}
	const std::vector<int>& imageTile::getColumnIndices() const
	{
		return columnIndices;
	}
	QGraphicsPixmapItem* imageTile::getItem() const
	{
		return pixMapItem.data();
	}
	int imageTile::getRowGroup() const
	{
		return rowGroup;
	}
	int imageTile::getColumnGroup() const
	{
		return columnGroup;
	}
}
