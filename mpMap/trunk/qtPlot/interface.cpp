#include "interface.h"
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include "interface2.h"
#include <vector>
extern "C"
{
	//this is the entry point for R, we want to pull in as little R stuff as possible so it goes in its own file
	Q_DECL_EXPORT SEXP plotQTImpl(SEXP data, SEXP imputedData, SEXP groups, SEXP auxillaryNumeric)
	{
		double* dataPtr = &(REAL(data)[0]);

		//There may or may not be imputed data input to the function (needed for ordering)
		double* imputedDataPtr;
		if(TYPEOF(imputedData) != NILSXP)
		{
			imputedDataPtr = REAL(imputedData);
		}
		else imputedDataPtr = NULL;
		

		int* groupsPtr = &(INTEGER(groups)[0]);
		int nMarkers = LENGTH(groups);

		std::vector<std::string> markerNames;
		SEXP dataDimNames = getAttrib(data, R_DimNamesSymbol);
		SEXP columnDimNames = VECTOR_ELT(dataDimNames, 1);
		for(int i = 0; i < nMarkers; i++)
		{
			markerNames.push_back(CHAR(STRING_ELT(columnDimNames, i)));
		}
		
		//There may or may not be auxillary numerical data
		double* auxData = NULL;
		int auxRows = 0;
		if(TYPEOF(auxillaryNumeric) != NILSXP)
		{
			auxData = REAL(auxillaryNumeric);
			SEXP dim = getAttrib(auxillaryNumeric, R_DimSymbol);
			auxRows = INTEGER(dim)[0];
		}

		std::vector<std::string> outputMarkerNames;
		std::vector<int> outputGroups;
		mpMap::plotQTImpl2(dataPtr, imputedDataPtr, groupsPtr, nMarkers, markerNames, outputMarkerNames, outputGroups, auxData, auxRows);

		SEXP retVal, R_markerNames, R_outputGroups;
		PROTECT(retVal = allocVector(VECSXP, 2));
			PROTECT(R_markerNames = allocVector(STRSXP, outputMarkerNames.size()));
				SET_VECTOR_ELT(retVal, 0, R_markerNames);
			UNPROTECT(1);
			PROTECT(R_outputGroups = allocVector(INTSXP, outputGroups.size()));
				SET_VECTOR_ELT(retVal, 1, R_outputGroups);
			UNPROTECT(1);
			for(int i = 0; i < outputMarkerNames.size(); i++)
			{
				SET_STRING_ELT(R_markerNames, i, mkChar(outputMarkerNames[i].c_str()));
				INTEGER(R_outputGroups)[i] = outputGroups[i];
			}
		UNPROTECT(1);
		return retVal;
	}
}