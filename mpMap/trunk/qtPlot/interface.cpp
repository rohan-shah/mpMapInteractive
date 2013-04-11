#include "interface.h"
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include "interface2.h"
extern "C"
{
	//this is the entry point for R, we want to pull in as little R stuff as possible so it goes in its own file
	Q_DECL_EXPORT SEXP plotQTImpl(SEXP data, SEXP groups, SEXP done_)
	{
		double* dataPtr = &(REAL(data)[0]);
		int dataDim;
		{
			SEXP dataDim_ = getAttrib(data, R_DimSymbol);
			dataDim = INTEGER(dataDim_)[0];
		}
		int* groupsPtr = &(INTEGER(groups)[0]);
		int nMarkers = LENGTH(groups);

		mpMap::plotQTImpl2(dataPtr, groupsPtr, nMarkers);

		int* done = &(LOGICAL(done_)[0]);
		done[0] = true;
		return R_NilValue;
	}
}