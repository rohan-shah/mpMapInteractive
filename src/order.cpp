#include "order.h"
#include <map>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <Rcpp.h>
namespace mpMapInteractive
{
	void order(double* rawData, int nOriginalMarkers, const std::vector<int>& permutation, int startIndex, int endIndex, std::vector<int>& resultingPermutation)
	{
		int nSubMarkers = endIndex - startIndex;
		Rcpp::NumericMatrix subMatrix(nSubMarkers, nSubMarkers);

		Rcpp::Function asDist("as.dist"), seriate("seriate"), getOrder("get_order"), criterion("criterion");
		
		double* mem = &(subMatrix(0,0));
		//Determine whether or not the submatrix is zero, as seriation seems to screw up for all-zero matrices.
		bool nonZero = false;
		for(int i = 0; i < nSubMarkers; i++)
		{
			for(int j = 0; j < nSubMarkers; j++)
			{
				double* dest = mem + i + j * nSubMarkers;
				*dest = rawData[permutation[i + startIndex] + permutation[j + startIndex] * nOriginalMarkers];
				nonZero |= (*dest != 0);
			}
		}
		if(nonZero)
		{
			Rcpp::RObject distSubMatrix = asDist(Rcpp::Named("m") = subMatrix);

			std::string methods[6] = {"TSP", "OLO", "ARSA", "MDS", "GW", "HC"};
			Rcpp::List nRepsControl = Rcpp::List::create(Rcpp::Named("nreps") = 5);
			std::vector<std::vector<int> > methodResults;
			std::vector<double> objectiveFunctionValues;

			for(int methodIndex = 0; methodIndex < 6; methodIndex++)
			{
				Rcpp::RObject result;
				if(methods[methodIndex] == "ARSA")
				{
					result = seriate(Rcpp::Named("x") = distSubMatrix, Rcpp::Named("method") = methods[methodIndex], Rcpp::Named("control") = nRepsControl);
				}
				else result = seriate(Rcpp::Named("x") = distSubMatrix, Rcpp::Named("method") = methods[methodIndex]);
				std::vector<int> currentMethodResult = Rcpp::as<std::vector<int> >(getOrder(result));
				methodResults.push_back(currentMethodResult);

				Rcpp::NumericVector criterionResult = criterion(Rcpp::Named("x") = distSubMatrix, Rcpp::Named("order") = result, Rcpp::Named("method") = "AR_events");
				objectiveFunctionValues.push_back(Rcpp::as<double>(criterionResult));
			}

			//now consider identity permutation
			std::vector<int> identity; 
			for(int i = 0; i < nSubMarkers; i++) identity.push_back(i+1);
			methodResults.push_back(identity);
			objectiveFunctionValues.push_back(Rcpp::as<double>(criterion(Rcpp::Named("x") = distSubMatrix, Rcpp::Named("method") = "AR_events")));

			resultingPermutation = methodResults[std::distance(objectiveFunctionValues.begin(), std::min_element(objectiveFunctionValues.begin(), objectiveFunctionValues.end()))];
			//Permutation stuff in R is indexed at base 1, but we want it indexed starting at 0.
			for(int i = 0; i < nSubMarkers; i++) resultingPermutation[i] -= 1;
		}
		else
		{
			resultingPermutation.resize(nSubMarkers);
			for(int i = 0; i < nSubMarkers; i++) resultingPermutation[i] = i;
		}
	}
}
