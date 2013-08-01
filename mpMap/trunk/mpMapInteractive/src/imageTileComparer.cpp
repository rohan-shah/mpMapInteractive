#include "imageTileComparer.h"
#include "imageTile.h"
namespace mpMap
{
	bool imageTileComparer::operator()(const imageTile& first, const imageTile& second) const
	{
		int firstRowGroup = first.getRowGroup(), secondRowGroup = second.getRowGroup();
		if(firstRowGroup < secondRowGroup) return true;
		if(secondRowGroup < firstRowGroup) return false;
		int firstColumnGroup = first.getColumnGroup(), secondColumnGroup = second.getColumnGroup();
		return firstColumnGroup < secondColumnGroup;
	}
}
