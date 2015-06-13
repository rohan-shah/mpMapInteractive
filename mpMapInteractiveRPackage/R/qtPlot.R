qtPlotMpMap2 <- function(mpcross, auxillaryNumeric = NULL)
{
	library(mpMap2)
	result <- .Call("qtPlotMpMap2", mpcross, auxillaryNumeric, PACKAGE="mpMapInteractive")
	markerNames <- result[[1]]
	groups <- result[[2]]
	names(groups) <- markerNames
	withoutLG <- as(mpcross, "mpcrossRF")
	selectedMethod <- selectMethod("subset", "mpcrossRF")
	subsetted <- mpMap2::subset(withoutLG, markers = markerNames)
	return(new("mpcrossLG", subsetted, lg = new("lg", groups = groups, allGroups = unique(groups))))
}
qtPlot <- function(mpcross, auxillaryNumeric = NULL)
{
	if(!is.null(auxillaryNumeric))
	{
		if(storage.mode(auxillaryNumeric) == "integer")
		{
			storage.mode(auxillaryNumeric) <- "numeric"
		}
		if(storage.mode(auxillaryNumeric) != "double")
		{
			stop("Input auxillaryNumeric must be a numeric matrix")
		}
		if(ncol(auxillaryNumeric) != ncol(mpcross$founders))
		{
			stop("Input auxillaryNumeric had the wrong number of columns")
		}
		markers <- colnames(auxillaryNumeric)
		if(length(markers) != ncol(mpcross$finals) || !(all(markers %in% colnames(mpcross$finals))))
		{
			stop("Column names of auxillaryNumeric did not match up with marker names of mpcross")
		}
	}
	if(inherits(mpcross, "mpcrossLG"))
	{
		return(qtPlotMpMap2(mpcross, auxillaryNumeric))
	}
	else
	{
		return(qtPlotMpMap(mpcross, auxillaryNumeric))	
	}
}
qtPlotMpMap <- function(mpcross, auxillaryNumeric = NULL)
{
	library(mpMap)
	result <- .Call("qtPlotMpMap", mpcross, auxillaryNumeric, PACKAGE="mpMapInteractive")
	markerNames <- result[[1]]
	groups <- result[[2]]
	names(groups) <- markerNames
	subsetted <- subset(mpcross, markers = markerNames)
	subsetted$lg$groups <- groups
	subsetted$lg$all.groups <- unique(groups)
	return(subsetted)
}