#include <Rcpp.h>
#include "interface.h"
#include "interface2.h"
#include "loadQt.h"
#include <internal.h>
extern "C"
{
	R_CallMethodDef callMethods[] = 
	{
		{"qtPlotMpMap", (DL_FUNC)&qtPlotMpMap, 2},
		{"qtPlotMpMap2", (DL_FUNC)&qtPlotMpMap2, 2},
		{"loadQT", (DL_FUNC)&loadQT, 0},
		{NULL, NULL, 0}
	};
	RcppExport void R_init_mpMapInteractive(DllInfo *info)
	{
		std::vector<R_CallMethodDef> callMethodsVector;
		R_CallMethodDef* mpMapInteractiveCallMethods = callMethods;
		while(mpMapInteractiveCallMethods->name != NULL) mpMapInteractiveCallMethods++;
		callMethodsVector.insert(callMethodsVector.begin(), callMethods, mpMapInteractiveCallMethods);

#ifdef CUSTOM_STATIC_RCPP
		R_CallMethodDef* RcppStartCallMethods = Rcpp_get_call();
		R_CallMethodDef* RcppEndCallMethods = RcppStartCallMethods;
		while(RcppEndCallMethods->name != NULL) RcppEndCallMethods++;
		callMethodsVector.insert(callMethodsVector.end(), RcppStartCallMethods, RcppEndCallMethods);
#endif
		R_CallMethodDef blank = {NULL, NULL, 0};
		callMethodsVector.push_back(blank);

		R_registerRoutines(info, NULL, &(callMethodsVector[0]), NULL, NULL);
#ifdef CUSTOM_STATIC_RCPP
		init_Rcpp_cache();
#endif
	}
}
