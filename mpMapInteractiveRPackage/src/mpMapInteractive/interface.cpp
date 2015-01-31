#include <vector>
#include "interface.h"
#include <Rcpp.h>
#include "validateMPCross.h"
#include "qtPlot.h"
extern "C"
{
	RcppExport SEXP qtPlot(SEXP mpcross__, SEXP auxillaryNumeric_)
	{
	BEGIN_RCPP
		int nFounders;
		std::string error;
		Rcpp::RObject mpcross_(mpcross__);
		bool valid = validateMPCross(mpcross_, nFounders, error, false, true, false, false);
		if(!valid)
		{
			throw std::runtime_error(error);
		}
		{
			bool hasLG = validateMPCross(mpcross_, nFounders, error, false, true, true, false);
			Rcpp::List mpcross(mpcross__);
			Rcpp::List rf = Rcpp::as<Rcpp::List>(mpcross["rf"]);
			Rcpp::NumericMatrix theta(Rcpp::as<Rcpp::NumericMatrix>(rf["theta"]));
			Rcpp::List lg;
			Rcpp::IntegerVector groups;
		
			if(hasLG)
			{
				lg = Rcpp::as<Rcpp::List>(mpcross["lg"]);
				{
					Rcpp::RObject groups_(Rcpp::as<Rcpp::RObject>(lg["groups"]));
					if(groups_.sexp_type() == INTSXP)
					{
						groups = groups_.asSexp();
					}
					else 
					{
						Rcpp::NumericVector tmp = groups_.asSexp();
						groups = Rcpp::IntegerVector(tmp.size());
						std::copy(tmp.begin(), tmp.end(), groups.begin());
					}
				}
			}
			else 
			{
				groups = Rcpp::IntegerVector(theta.ncol());
				std::fill(groups.begin(), groups.end(), 1);
			}
			//check the auxillary numeric matrix
			Rcpp::RObject auxillaryNumeric(auxillaryNumeric_);
			if(auxillaryNumeric.sexp_type() != NILSXP)
			{
				if(auxillaryNumeric.sexp_type() != REALSXP)
				{
					throw std::runtime_error("Input auxillaryNumeric must be a numeric matrix");
				}
				Rcpp::RObject auxDim_ = auxillaryNumeric.attr("dim");
				if(auxDim_.sexp_type() != INTSXP) 
				{
					throw std::runtime_error("Input auxillaryNumeric had dimensions of wrong type");
				}
				Rcpp::IntegerVector auxDim = Rcpp::as<Rcpp::IntegerVector>(auxDim_);
				if(auxDim.length() != 2)
				{
					throw std::runtime_error("Input auxillaryNumeric had dimensions of wrong length");
				}
				if(auxDim[1] != theta.ncol())
				{
					throw std::runtime_error("Input auxillaryNumeric had wrong number of columns");
				}
			}
		
			int nMarkers = groups.size();
			Rcpp::List dataDimNames = Rcpp::Language("dimnames", theta).eval();
			Rcpp::CharacterVector columnDimNames = dataDimNames[1];
			std::vector<std::string> markerNames = Rcpp::as<std::vector<std::string> >(columnDimNames);

			//There may or may not be auxillary numerical data
			double* auxData = NULL;
			int auxRows = 0;
			if(TYPEOF(auxillaryNumeric) != NILSXP)
			{
				try
				{
					auxRows = Rcpp::as<Rcpp::NumericMatrix>(auxillaryNumeric).nrow();
					auxData = REAL(auxillaryNumeric);
				}
				catch(...)
				{
					throw std::runtime_error("If auxillary data is input, it must be a numeric matrix");
				}
			}

			std::vector<std::string> outputMarkerNames;
			std::vector<int> outputGroups;

			{
				char* argv[] = {""};
				int argc = 0;
				QApplication app(argc, argv);
		
				std::vector<int> groupsVector = Rcpp::as<std::vector<int> >(groups);
				mpMap::qtPlot plot(&(theta[0]), groupsVector, markerNames, auxData, auxRows);
				plot.show();
				app.exec();
				const mpMap::qtPlotData& outputData = plot.getData();
				outputGroups = outputData.getCurrentGroups();
				outputMarkerNames = outputData.getCurrentMarkerNames();
			}
			Rcpp::CharacterVector convertedOutputMarkerNames = Rcpp::wrap(outputMarkerNames);
			Rcpp::IntegerVector convertedOutputGroups = Rcpp::wrap(outputGroups);
			Rcpp::List retVal = Rcpp::List::create(convertedOutputMarkerNames, convertedOutputGroups);
			return retVal;
		}
	END_RCPP
	}
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
