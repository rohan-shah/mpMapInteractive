TEMPLATE = lib
HEADERS += interface.h qtPlot.h interface2.h colour.h ZoomGraphicsView.h order.h
SOURCES += interface.cpp qtPlot.cpp interface2.cpp colour.cpp ZoomGraphicsView.cpp order.cpp
LIBPATH += $$quote(C:\Program Files\Microsoft SDKs\Windows\v7.1\Lib\x64)
LIBPATH += $$quote(C:\Program Files\R\R-2.15.1\bin\x64)
QT += widgets
INCLUDEPATH += "C:\Program Files\R\R-2.15.1\include"
LIBS+= _R.lib