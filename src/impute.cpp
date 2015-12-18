#include "impute.h"
#include <vector>
#include <map>
#include <math.h>
template<bool hasLOD, bool hasLKHD> bool imputeInternal(double* theta, double* lod, double* lkhd, int nMarkers, int* groups, int group, std::string& error)
{
	std::vector<int> markersThisGroup;
	for(int markerCounter = 0; markerCounter < nMarkers; markerCounter++)
	{
		if(group == groups[markerCounter]) markersThisGroup.push_back(markerCounter);
	}
	for(std::vector<int>::iterator marker1 = markersThisGroup.begin(); marker1 != markersThisGroup.end(); marker1++)
	{
		bool missing = false;
		for(std::vector<int>::iterator marker2 = markersThisGroup.begin(); marker2 != markersThisGroup.end(); marker2++)
		{
			//record whether there's a missing value for marker1 anywhere.
			float val = theta[*marker1 + *marker2 * nMarkers];
			if(val != val) 
			{
				missing = true;
				break;
			}
		}
		if(missing)
		{
			//There's a missing value for *marker1. So find the closest matching column and overwrite any missing values from that column.
			//Map with the difference as the key and the marker index as the value. These are sorted internally so we can go from smallest difference to lowest difference
			std::map<float, int> averageDifferences;
			//marker 2 is the candidate other marker
			for(std::vector<int>::iterator marker2 = markersThisGroup.begin(); marker2 != markersThisGroup.end(); marker2++)
			{
				if(marker2 == marker1) continue;
				
				float totalDifference = 0;
				int usableLocations = 0;
				for(std::vector<int>::iterator marker3 = markersThisGroup.begin(); marker3 != markersThisGroup.end(); marker3++)
				{
					float val = fabs(theta[*marker1 + *marker3 * nMarkers] - theta[*marker2 + *marker3 * nMarkers]);
					if(val == val)
					{
						totalDifference += val;
						usableLocations++;
					}
				}
				if(usableLocations != 0)
				{
					averageDifferences.insert(std::make_pair(totalDifference/usableLocations, *marker2));
				}
				else averageDifferences.insert(std::make_pair(std::numeric_limits<float>::quiet_NaN(), *marker2));
			}
			for(std::vector<int>::iterator marker2 = markersThisGroup.begin(); marker2 != markersThisGroup.end(); marker2++)
			{
				float val = theta[*marker1 + *marker2 * nMarkers];
				if(val != val) 
				{
					//go through the other markers from most similar to least similar, looking for something which has a value here. So marker3 is the 
					bool replacementFound = false;
					for(std::map<float, int>::iterator marker3 = averageDifferences.begin(); marker3 != averageDifferences.end(); marker3++)
					{
						float newValue = theta[marker3->second + *marker2 * nMarkers];
						if(newValue == newValue)
						{
							theta[*marker1 + *marker2 * nMarkers] = theta[marker3->second + *marker2*nMarkers];
							if(hasLOD) lod[*marker1 + *marker2 * nMarkers] = lod[marker3->second + *marker2*nMarkers];
							if(hasLKHD) lkhd[*marker1 + *marker2 * nMarkers] = lkhd[marker3->second + *marker2 * nMarkers];
							//We only need to copy the value from the best other similar marker, so we can break here
							replacementFound = true;
							break;
						}
					}
					if(!replacementFound)
					{
						std::stringstream ss;
						ss << "Unable to impute a value for marker " << (*marker1+1) << " and marker " << (*marker2+1);
						//Really annoying C-style string handling. 
						error = ss.str();
						return false;
					}
				}
			}
		}
	}
	return true;
}
bool impute(double* theta, double* lod, double* lkhd, int nMarkers, int* groups, int group, std::string& error)
{
	if(lod != NULL && lkhd != NULL)
	{
		return imputeInternal<true, true>(theta, lod, lkhd, nMarkers, groups, group, error);
	}
	else if(lod != NULL && lkhd == NULL)
	{
		return imputeInternal<true, false>(theta, lod, lkhd, nMarkers, groups, group, error);
	}
	else if(lod == NULL && lkhd != NULL)
	{
		return imputeInternal<false, true>(theta, lod, lkhd, nMarkers, groups, group, error);
	}
	else
	{
		return imputeInternal<false, false>(theta, lod, lkhd, nMarkers, groups, group, error);
	}
}
