#include <vector>
#include "interface.h"
#include <Rcpp.h>
#include "qtPlot.h"
#include <QtWidgets/qapplication.h>
extern "C"
{
	//bool validateMPCrossNoError(SEXP mpcross_, int& nFounders, bool checkPedigree = true, bool checkRF = false, bool checkLG = false, bool checkFID = true);
	typedef bool (*validateMPCrossNoError)(SEXP, int& nFounders, bool checkPedigree, bool checkRF, bool checkLG, bool checkFID);
	RcppExport SEXP qtPlotMpMap(SEXP mpcross__, SEXP auxillaryNumeric_)
	{
	BEGIN_RCPP
		int nFounders;
		std::string error;
		Rcpp::RObject mpcross_(mpcross__);
		validateMPCrossNoError functionPtr = (validateMPCrossNoError)R_GetCCallable("mpMap", "validateMPCrossNoError");
		if(functionPtr == NULL)
		{
			throw std::runtime_error("Package mpMap must be loaded in order to call qtPlot with an mpMap object");
		}
		bool valid = functionPtr(mpcross_, nFounders, false, true, false, false);
		if(!valid)
		{
			throw std::runtime_error("Input mpcross object was invalid, please run validate(object)");
		}
		{
			bool hasLG = functionPtr(mpcross_, nFounders, false, true, true, false);
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
						groups = Rcpp::as<Rcpp::IntegerVector>(groups_);
					}
					else 
					{
						Rcpp::NumericVector tmp = Rcpp::as<Rcpp::NumericVector>(groups_);
						groups = Rcpp::IntegerVector(tmp.size());
						for(int i = 0; i < tmp.size(); i++) groups[i] = (int)tmp[i];
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
			Rcpp::List dataDimNames = theta.attr("dimnames");
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
				char* argv[3];
				argv[0] = new char[1];
				argv[1] = new char[1];
				argv[2] = new char[1];
				argv[0][0] = argv[1][0] = argv[2][0] = 0;

				int argc = 1;
				QApplication app(argc, argv);
		
				std::vector<int> groupsVector = Rcpp::as<std::vector<int> >(groups);
				mpMapInteractive::qtPlot plot(&(theta[0]), groupsVector, markerNames, auxData, auxRows);
				plot.show();
				app.exec();
				const mpMapInteractive::qtPlotData& outputData = plot.getData();
				outputGroups = outputData.getCurrentGroups();
				outputMarkerNames = outputData.getCurrentMarkerNames();

				delete[] argv[0];
				delete[] argv[1];
				delete[] argv[2];
			}
			Rcpp::CharacterVector convertedOutputMarkerNames = Rcpp::wrap(outputMarkerNames);
			Rcpp::IntegerVector convertedOutputGroups = Rcpp::wrap(outputGroups);
			Rcpp::List retVal = Rcpp::List::create(convertedOutputMarkerNames, convertedOutputGroups);
			return retVal;
		}
	END_RCPP
	}
}
