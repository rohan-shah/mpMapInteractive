#ifndef IMAGE_TILE_HEADER_GUARD
#define IMAGE_TILE_HEADER_GUARD
#include "imageTileComparer.h"
#include <set>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <vector>
namespace mpMap
{
	class imageTile
	{
		public:
			imageTile(uchar* data, int dataRows, int rowGroup, int columnGroup, const std::vector<int>& rowIndices, const std::vector<int>& columnIndices, QGraphicsScene* graphicsScene);
			~imageTile();
			const std::vector<int>& getRowIndices() const;
			const std::vector<int>& getColumnIndices() const;
			int getRowGroup() const;
			int getColumnGroup() const;
			static std::set<imageTile, imageTileComparer>::const_iterator find(const std::set<imageTile, imageTileComparer>& collection, int rowGroup, int columnGroup);
			QGraphicsPixmapItem* getItem() const;
			bool checkIndices(const std::vector<int>& otherRowIndices, const std::vector<int>& otherColumnIndices) const;
		private:
			imageTile();
			std::vector<int> rowIndices, columnIndices;
			int rowGroup, columnGroup;
			QSharedPointer<QGraphicsPixmapItem> pixMapItem;
	};
}
#endif
