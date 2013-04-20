#include <QtWidgets/qapplication.h>
struct SEXPREC;
typedef SEXPREC* SEXP;
extern "C"
{
	Q_DECL_EXPORT SEXP plotQTImpl(SEXP data, SEXP groups);
}