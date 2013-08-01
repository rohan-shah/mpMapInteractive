#ifndef IMAGE_TILE_COMPARER_HEADER_GUARD
#define IMAGE_TILE_COMPARER_HEADER_GUARD
namespace mpMap
{
	class imageTile;
	struct imageTileComparer
	{
		bool operator()(const imageTile& first, const imageTile& second) const;
	};
}
#endif
