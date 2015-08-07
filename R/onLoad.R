.onLoad <- function(libname, pkgname)
{
	#Are we currently running a test in a seperate process to test if we have graphics?
	if(.Platform$OS.type=="unix" && Sys.getenv("TESTING_MPMAP_INTERACTIVE_CAN_LOAD") != "TRUE")
	{
		#If not run a test to see if we can load with QT enabled - If we're running headless and it terminates the application, and there's no way to prevent this.
		executable <- commandArgs()[1]
		libPaths <- do.call(paste0, as.list(deparse(.libPaths())))
		libPaths <- gsub("\"", "\\\"", libPaths, fixed=TRUE)
		Sys.setenv("TESTING_MPMAP_INTERACTIVE_CAN_LOAD"="TRUE")
#This hideous complexity is because if there's no graphics then the shared library loading terminates with a SIGABRT. In this case bash adds its own error output which escapes from the attempt by R to catch all output (>/dev/null 2>&1. So that command has to be run in a subshell which returns a different code (1 in this case)
		command <- paste0("((", executable, " -e ", "\"library(mpMapInteractive, lib.loc=", libPaths,")\"", " ) || false) >/dev/null 2>&1")
		suppressWarnings(result <- system(command, intern=FALSE, ignore.stdout=FALSE, ignore.stderr=FALSE, wait=TRUE))
		Sys.unsetenv("TESTING_MPMAP_INTERACTIVE_CAN_LOAD")
		if(result != 0)
		{
			warning("Unable to load graphical component")
			Sys.setenv("QT_QPA_PLATFORM" = "offscreen")
		}
	}

	couldLoad <- try(library.dynam(package="mpMapInteractive", chname="mpMapInteractive", lib.loc = .libPaths()), silent=TRUE)
	wd <- getwd()
	if(class(couldLoad) != "try-error")
	{
		setwd(dirname(couldLoad[["path"]]))
		.Call("loadQT", PACKAGE="mpMapInteractive")
		setwd(wd)
	}
}
