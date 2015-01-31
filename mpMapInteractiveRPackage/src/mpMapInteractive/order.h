#ifndef ORDER_HEADER_GUARD
#define ORDER_HEADER_GUARD
#include <vector>
//We have the raw data (static, so that changes to the image don't do anything)
//We have the number of markers in the raw data
//The current permutation to be applied
//Once the permutation is applied, what is the marker interval we wish to reorder (closed on left, open on right). 
//output parameter for resulting permutation. 
namespace mpMap
{
	void order(double* rawData, int nOriginalMarkers, const std::vector<int>& permutation, int startIndex, int endIndex, std::vector<int>& resultingPermutation);
}
#endif