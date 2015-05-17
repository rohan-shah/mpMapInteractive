#include <Rcpp.h>
#include "interface.h"
extern "C"
{
	RcppExport void R_init_Rcpp(DllInfo *info);
	R_CallMethodDef callMethods[] = 
	{
		{"qtPlot", (DL_FUNC)&qtPlot, 2},
		{"loadQT", (DL_FUNC)&loadQT, 0},
		{NULL, NULL, 0}
	};
	RcppExport void R_init_mpMapInteractive(DllInfo *info)
	{
		R_registerRoutines(info, NULL, callMethods, NULL, NULL);
		R_init_Rcpp(info);
	}
}