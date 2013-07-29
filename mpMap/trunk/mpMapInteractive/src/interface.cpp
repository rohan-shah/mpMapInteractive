#include <vector>
#include "interface.h"
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include "interface2.h"
extern "C"
{
	//this is the entry point for R, we want to pull in as little R stuff as possible so it goes in its own file
	Q_DECL_EXPORT SEXP plotQTImpl(SEXP data, SEXP groups, SEXP auxillaryNumeric)
	{
		double* dataPtr = &(REAL(data)[0]);

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
		mpMap::plotQTImpl2(dataPtr, groupsPtr, nMarkers, markerNames, outputMarkerNames, outputGroups, auxData, auxRows);

		SEXP retVal, R_markerNames, R_outputGroups;
		PROTECT(retVal = allocVector(VECSXP, 2));
			PROTECT(R_markerNames = allocVector(STRSXP, (R_len_t)outputMarkerNames.size()));
				SET_VECTOR_ELT(retVal, 0, R_markerNames);
			UNPROTECT(1);
			PROTECT(R_outputGroups = allocVector(INTSXP, (R_len_t)outputGroups.size()));
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
	//here we need to pass in the directory in which this shared library is located. In the case of dynamic linkage to QT, we need to register this path so that QT can find the plugins directory
	Q_DECL_EXPORT SEXP loadQT()
	{
		QCoreApplication::addLibraryPath(QString("."));
		//only needs to be called once
		static bool called = false;
		if(called) return R_NilValue;
		char* argv[] = {""};
		int argc = 0;
		//We make an attempt here to tripper the QT runtime, at which point it will load up all the required plugins, while the 
		//working directory is correctly set. 
		QApplication app(argc, argv);
		return R_NilValue;
	}
}