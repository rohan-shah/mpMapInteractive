#include <QtWidgets/qapplication.h>
#include <Rcpp.h>
extern "C"
{
	Q_DECL_EXPORT SEXP loadQT();
}