## mpMapInteractive

This package provides an interactive method for constructing linkage groups and ordering the markers within linkage groups. As the package uses the Qt graphics framework, it must be compiled using CMake.

#Requirements

A copy of the Qt 5 graphics framework is required. 
 
On windows the customized version of Rcpp from github repository rohan-shah/Rcpp is also required. See the associated Readme file for details on compiling that code. On other platforms you may choose the customized version or the standard Rcpp package by setting the variable USE_CUSTOM_RCPP to either ON or OFF. 

##Compilation on Windows using CMake and Visual Studio

1. Run the cmake gui. 
2. Set Rcpp_DIR to the Rcpp binaries directory. 
3. Set Qt5_Dir to the directory containing Qt5Config.cmake (`<Qt5Root>`/lib/cmake/Qt5). 
4. Set R_COMMAND to `<R_HOME>`/bin/x64/R.exe. Ensure that you choose the 64-bit version. 
5. Enter the source directory and the binaries directory (E.g. `<mpMapInteractive>`/build for Visual Studio 64-bit output, or `<mpMapInteractive>`/release for NMakeMakefiles)
6. If the output is going to be NMake Makefiles, set CMAKE_BUILD_TYPE appropriately (E.g. as either Release or Debug)
7. Hit Configure and when prompted choose a Visual Studio 64-bit output, or NMake Makefiles.
8. When configuring succeeds, hit generate. 

The configuration scripts generate an import library for R.dll. This means that the scripts must be able to run cl.exe and lib.exe. If this step fails, check that cl.exe and lib.exe can run. If not, you may need to set up the correct environment for the compiler (by running a script such as vcvarsx86_amd64.bat) before running cmake. 

The package can now be compiled by either running nmake in the binaries directory (NMake Makefiles) or opening mpInteractive.sln in the binaries directory. Once the package is compiled a properly formed R package (including NAMESPACE, DESCRIPTION, .R files and C code) will have been constructed in the binaries directory. If Visual Studio output was selected, the package directory will be `<mpMapInteractiveBinaries>`/`<buildType>` (E.g. mpMapInteractive/build/Release for a release build). If NMake Makefiles output was selected, the package will be `<mpMapInteractiveBinaries>` (E.g mpMapInteractive/release). The package can be installed using the INSTALL target or by running R CMD INSTALL `<mpMapInteractiveBinariesDir>`

##Compilation on Linux using customized Rcpp

1. Choose a binaries directory, E.g. `<mpMapInteractiveRoot>`/release for a release build. 
2. Run cmake, specifying the variables Qt5_DIR, Rcpp_DIR and R_COMMAND. E.g. for a release build
  
  cmake `<mpMapInteractivRoot>` -DQt5_DIR=`<Qt5Root>`/lib/cmake/Qt5 -DRcpp_DIR=`<RcppRoot>`/release -DCMAKE_BUILD_TYPE=Release -DR_COMMAND=`<PathToR>` -DUSE_CUSTOM_RCPP=ON
3. Run make and then make install from the binaries directory

##Compilation on Linux using standard Rcpp

1. Choose a binaries directory, E.g. `<mpMapInteractiveRoot>`/release for a release build. 
2. Run cmake, specifying the variables Qt5_DIR and R_COMMAND. E.g. for a release build
  
  cmake `<mpMapInteractivRoot>` -DQt5_DIR=`<Qt5Root>`/lib/cmake/Qt5 -DCMAKE_BUILD_TYPE=Release -DR_COMMAND=`<PathToR>` -DUSE_CUSTOM_RCPP=OFF
3. Run make and then make install from the binaries directory
