TEMPLATE = lib
HEADERS += interface.h qtPlot.h interface2.h colour.h ZoomGraphicsView.h order.h imageTileComparer.h imageTile.h
SOURCES += interface.cpp qtPlot.cpp interface2.cpp colour.cpp ZoomGraphicsView.cpp order.cpp imageTileComparer.cpp imageTile.cpp
TARGET = mpMapInteractive
QT += widgets
CONFIG += plugin no_plugin_name_prefix

#Link to R
INCLUDEPATH += $$system(Rscript -e \"cat(Sys.getenv(\\\"R_INCLUDE_DIR\\\"))\")

#Link to mpMap
MPMAP_PACKAGE = $$system(Rscript -e \"options(warn=-1); suppressPackageStartupMessages(library(mpMap, quietly=TRUE)); cat(path.package(\\\"mpMap\\\"))\")
LIBS += $$system(${R_HOME}/bin/R CMD config --ldflags) $$MPMAP_PACKAGE/lib/$(R_ARCH)/libmpMap.a $$system(Rscript -e \"Rcpp:::LdFlags(static=TRUE)\")
QMAKE_LFLAGS_PLUGIN -= -dynamiclib
QMAKE_LFLAGS_PLUGIN += -bundle
QMAKE_EXTENSION_SHLIB = so
QMAKE_PREFIX_SHLIB =
