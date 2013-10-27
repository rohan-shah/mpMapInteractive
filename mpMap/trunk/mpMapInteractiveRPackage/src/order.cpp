#include "order.h"
#include <map>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <R.h>
#include <Rdefines.h>
namespace mpMap
{
	SEXP makeCall(std::map<std::string, SEXP> arguments, std::string name)
	{
		SEXP call, t;
		PROTECT(t = call = allocList(arguments.size() + 1));
			SET_TYPEOF(call, LANGSXP);
			SETCAR(t, install(name.c_str()));
			t = CDR(t);

			for(std::map<std::string, SEXP>::iterator i = arguments.begin(); i!= arguments.end(); i++)
			{
				SETCAR(t, i->second);
				SET_TAG(t, install(i->first.c_str()));
				t = CDR(t);
			}
	
			int errorOccurred;
			SEXP result = R_tryEval(call, R_GlobalEnv, &errorOccurred);
			if(errorOccurred) throw std::runtime_error("Internal error");
		UNPROTECT(1);
		return(result);
	}
	SEXP constructSeriateControl(int count)
	{
		SEXP list;
		PROTECT(list = allocVector(VECSXP, 1));
			SEXP nreps;
			PROTECT(nreps = allocVector(INTSXP, 1));
				SET_VECTOR_ELT(list, 0, nreps);
			UNPROTECT(1);
			INTEGER(nreps)[0] = count;

			SEXP names;
			PROTECT(names = allocVector(STRSXP, 1));
				setAttrib(list, R_NamesSymbol, names);
			UNPROTECT(1);
			SET_STRING_ELT(names, 0, mkChar("nrep"));
		UNPROTECT(1);
		return list;
	}
	SEXP orderInternal(std::string method, SEXP distMatrix)
	{
		SEXP R_method;
		PROTECT(R_method = allocVector(STRSXP, 1));
			SET_STRING_ELT(R_method, 0, mkChar(method.c_str()));

			SEXP control;
			PROTECT(control = constructSeriateControl(5));

				std::map<std::string, SEXP> arguments;
				arguments.insert(std::make_pair("x", distMatrix));
				arguments.insert(std::make_pair("method", R_method));
				arguments.insert(std::make_pair("control", control));

				SEXP result = makeCall(arguments, "seriate");
			UNPROTECT(1);
		UNPROTECT(1);
		return result;
	}
	SEXP asDist(SEXP matrix)
	{
		std::map<std::string, SEXP> arguments;
		arguments.insert(std::make_pair("m", matrix));

		SEXP result = makeCall(arguments, "as.dist");
		return result;
	}
	void getOrder(std::vector<int>& outputOrder, SEXP seriateResult)
	{
		std::map<std::string, SEXP> arguments;
		arguments.insert(std::make_pair("x", seriateResult));

		SEXP result = makeCall(arguments, "get_order");
		outputOrder.insert(outputOrder.begin(), INTEGER(result), INTEGER(result) + LENGTH(result));
	}
	double criterion(SEXP seriateResult, SEXP matrix)
	{
		SEXP AR_events;
		PROTECT(AR_events = allocVector(STRSXP, 1));
			SET_STRING_ELT(AR_events, 0, mkChar("AR_events"));
			std::map<std::string, SEXP> arguments;
			arguments.insert(std::make_pair("x", matrix));
			if(TYPEOF(seriateResult) != NILSXP)
			{
				arguments.insert(std::make_pair("order", seriateResult));
			}
			arguments.insert(std::make_pair("method", AR_events));
			
			SEXP result = makeCall(arguments, "criterion");
		UNPROTECT(1);
		return REAL(result)[0];
	}
	void order(double* rawData, int nOriginalMarkers, const std::vector<int>& permutation, int startIndex, int endIndex, std::vector<int>& resultingPermutation)
	{
		int nSubMarkers = endIndex - startIndex;
		SEXP R_submatrix;
		
		PROTECT(R_submatrix = allocVector(REALSXP, nSubMarkers*nSubMarkers));
			SEXP R_submatrixDim;
			PROTECT(R_submatrixDim = allocVector(INTSXP, 2));
				INTEGER(R_submatrixDim)[0] = INTEGER(R_submatrixDim)[1] = nSubMarkers;
				setAttrib(R_submatrix, R_DimSymbol, R_submatrixDim);
			UNPROTECT(1);
			double* mem = REAL(R_submatrix);
			for(int i = 0; i < nSubMarkers; i++)
			{
				for(int j = 0; j < nSubMarkers; j++)
				{
					mem[i + j * nSubMarkers] = rawData[permutation[i + startIndex] + permutation[j + startIndex] * nOriginalMarkers];
				}
			}
			SEXP distSubmatrix;
			PROTECT(distSubmatrix = asDist(R_submatrix));

				std::string methods[6] = {"TSP", "OLO", "ARSA", "MDS", "GW", "HC"};
				std::vector<std::vector<int> > methodResults;
				std::vector<double> objectiveFunctionValues;

				for(int methodIndex = 0; methodIndex < 6; methodIndex++)
				{
					SEXP R_result;
					std::vector<int> currentMethodResult;
					PROTECT(R_result = orderInternal(methods[methodIndex], distSubmatrix));
						getOrder(currentMethodResult, R_result);
						methodResults.push_back(currentMethodResult);
						objectiveFunctionValues.push_back(criterion(R_result, distSubmatrix));
					UNPROTECT(1);
				}

				//now consider identity permutation
				std::vector<int> identity; 
				for(int i = 0; i < nSubMarkers; i++) identity.push_back(i+1);
				methodResults.push_back(identity);
				objectiveFunctionValues.push_back(criterion(R_NilValue, distSubmatrix));

				resultingPermutation = methodResults[std::distance(objectiveFunctionValues.begin(), std::min_element(objectiveFunctionValues.begin(), objectiveFunctionValues.end()))];
				//Permutation stuff in R is indexed at base 1, but we want it indexed starting at 0.
				for(int i = 0; i < nSubMarkers; i++) resultingPermutation[i] -= 1;
			UNPROTECT(1);
		UNPROTECT(1);
	}
}
