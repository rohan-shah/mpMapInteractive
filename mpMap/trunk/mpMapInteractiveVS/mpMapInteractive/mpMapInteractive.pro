TEMPLATE = lib
HEADERS += ..\..\mpMapInteractiveRPackage\src\interface.h ..\..\mpMapInteractiveRPackage\src\qtPlot.h ..\..\mpMapInteractiveRPackage\src\colour.h ..\..\mpMapInteractiveRPackage\src\ZoomGraphicsView.h ..\..\mpMapInteractiveRPackage\src\order.h ..\..\mpMapInteractiveRPackage\src\imageTileComparer.h ..\..\mpMapInteractiveRPackage\src\imageTile.h ..\..\mpMapInteractiveRPackage\src\validateMPCross.h
SOURCES += ..\..\mpMapInteractiveRPackage\src\interface.cpp ..\..\mpMapInteractiveRPackage\src\qtPlot.cpp ..\..\mpMapInteractiveRPackage\src\colour.cpp ..\..\mpMapInteractiveRPackage\src\ZoomGraphicsView.cpp ..\..\mpMapInteractiveRPackage\src\order.cpp ..\..\mpMapInteractiveRPackage\src\imageTileComparer.cpp ..\..\mpMapInteractiveRPackage\src\imageTile.cpp ..\..\mpMapInteractiveRPackage\src\validateMPCross.cpp
LIBPATH += $$quote(C:\Program Files\Microsoft SDKs\Windows\v7.1\Lib\x64)
LIBPATH += ..\libs\\\$(Platform)
QT += widgets
INCLUDEPATH += \$(R_HOME)/include
INCLUDEPATH += ..\Rcpp
INCLUDEPATH += ..\Rcpp\include
LIBS += R.lib mpMap.lib Rcpp.lib
LIBPATH += ..\\\$(Platform)\\\$(Configuration)