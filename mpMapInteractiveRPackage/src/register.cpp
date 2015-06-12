#include <Rcpp.h>
#include "interface.h"
#include "interface2.h"
#include "loadQt.h"
extern "C"
{
#ifdef CUSTOM_STATIC_RCPP
	RcppExport void R_init_Rcpp(DllInfo *info);
#endif
	R_CallMethodDef callMethods[] = 
	{
		{"qtPlotMpMap", (DL_FUNC)&qtPlotMpMap, 2},
		{"qtPlotMpMap2", (DL_FUNC)&qtPlotMpMap2, 2},
		{"loadQT", (DL_FUNC)&loadQT, 0},
		{NULL, NULL, 0}
	};
	RcppExport void R_init_mpMapInteractive(DllInfo *info)
	{
#ifdef CUSTOM_STATIC_RCPP
		R_init_Rcpp(info);
#endif
		R_registerRoutines(info, NULL, callMethods, NULL, NULL);
	}
}
