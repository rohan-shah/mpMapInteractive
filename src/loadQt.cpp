#include "loadQt.h"
extern "C"
{
	char* package_name = "mpMapInteractive";
	//here we need to pass in the directory in which this shared library is located. In the case of dynamic linkage to QT, we need to register this path so that QT can find the plugins directory
	Q_DECL_EXPORT SEXP loadQT()
	{
		QCoreApplication::addLibraryPath(QString("."));
		//only needs to be called once
		static bool called = false;
		if(called) return R_NilValue;

		char* empty = new char[1];
		*empty = 0;
		char* argv[] = {empty};

		int argc = 0;
		//We make an attempt here to tripper the QT runtime, at which point it will load up all the required plugins, while the 
		//working directory is correctly set. 
		QApplication app(argc, argv);
		delete[] empty;
		return R_NilValue;
	}
}