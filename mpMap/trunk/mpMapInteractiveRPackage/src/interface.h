#include <QtWidgets/qapplication.h>
#include <Rcpp.h>
struct SEXPREC;
typedef SEXPREC* SEXP;
extern "C"
{
	Q_DECL_EXPORT SEXP loadQT();
	Q_DECL_EXPORT SEXP qtPlot(SEXP data, SEXP auxillaryNumeric);
}