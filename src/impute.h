#ifndef IMPUTE_HEADER_GUARD
#define IMPUTE_HEADER_GUARD
#include <sstream>
#include "impute.h"
#include <limits>
bool impute(double* theta, double* lod, double* lkhd, int nMarkers, int* groups, int group, std::string& error);
#endif
