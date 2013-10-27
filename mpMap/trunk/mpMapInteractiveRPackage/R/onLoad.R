.onLoad <- function(libname, pkgname)
{
	couldLoad <- try(library.dynam(package="mpMapInteractive", chname="mpMapInteractive", lib.loc = .libPaths()), silent=TRUE)
	wd <- getwd()
	if(class(couldLoad) != "try-error")
	{
		setwd(dirname(couldLoad[["path"]]))
		.Call("loadQT", PACKAGE="mpMapInteractive")
		setwd(wd)
	}
}
