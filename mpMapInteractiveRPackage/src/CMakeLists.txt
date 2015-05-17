cmake_minimum_required(VERSION 3.1)
project(mpMapInteractive)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(R REQUIRED)
find_package(Rcpp REQUIRED)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

#Construct R.lib if we're using Visual studio on Windows
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	#R does not appear to tell us whether a debug build was selected, so just build a release build every time 
	message(STATUS "Checking that 64-bit R was selected")
	execute_process(COMMAND ${R_COMMAND} --no-restore --no-save -e "q()" OUTPUT_VARIABLE R_OUTPUT)
	string(FIND "${R_OUTPUT}" "64-bit" R_OUTPUT_64_BIT_INDEX)
	string(FIND "${R_OUTPUT}" "32-bit" R_OUTPUT_32_BIT_INDEX)
	if(NOT ${R_OUTPUT_32_BIT_INDEX} EQUAL -1)
		message(FATAL_ERROR "Checking that 64-bit R was selected - 32-bit R found")
	elseif(${R_OUTPUT_64_BIT_INDEX} EQUAL -1)
	 	message(STATUS ${R_OUTPUT})
		message(FATAL_ERROR "Checking that 64-bit R was selected - Unable to identify as either 64 or 32 bit")
	endif()
	message(STATUS "Checking that 64-bit R was selected - passed")

	message(STATUS "Checking that 64-bit compiler was selected")
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} ERROR_VARIABLE COMPILER_OUTPUT_STRING OUTPUT_QUIET)
	string(FIND "${COMPILER_OUTPUT_STRING}" x86 COMPILER_OUTPUT_STRING_X86_INDEX)
	if(NOT ${COMPILER_OUTPUT_STRING_X86_INDEX} EQUAL -1)
		message(FATAL_ERROR "Checking that 64-bit compiler was selected - FAILED\nPlease ensure that you select the x64 developer tools prompt")
	endif()
	string(FIND "${COMPILER_OUTPUT_STRING}" "x64" COMPILER_OUTPUT_STRING_X64_INDEX)
	if(${COMPILER_OUTPUT_STRING_X64_INDEX} EQUAL -1)
		message(FATAL_ERROR "Checking that 64-bit compiler was selected - FAILED\nPlease ensure that you select the x64 developer tools prompt")
	endif()
	message(STATUS "Checking that 64-bit compiler was selected - passed")

	#If we're using MSVC, work out where lib.exe is
	get_filename_component(COMPILER_PATH "${CMAKE_CXX_COMPILER}" DIRECTORY CACHE)
	message(STATUS "Searching for lib.exe")
	find_program(LIB_EXECUTABLE lib PATHS ${COMPILER_PATH})
	if(${LIB_EXECUTABLE} STREQUAL LIB_EXECUTABLE-NOTFOUND)
		message(FATAL_ERROR "Searching for lib.exe - not found")
	else()
		message(STATUS "Searching for lib.exe - found")
	endif()

	#If we're using MSVC, work out where dumpbin.exe is
	message(STATUS "Searching for dumpbin.exe")
	find_program(DUMPBIN_EXECUTABLE dumpbin PATHS ${COMPILER_PATH})
	if(${DUMPBIN_EXECUTABLE} STREQUAL DUMPBIN_EXECUTABLE-NOTFOUND)
		message(FATAL_ERROR "Searching for dumpbin.exe - not found")
	else()
		message(STATUS "Searching for dumpbin.exe - found")
	endif()

	message(STATUS "Generating import library for R.dll")
	file(WRITE "${PROJECT_BINARY_DIR}/R.def" "LIBRARY R\n")
	file(APPEND "${PROJECT_BINARY_DIR}/R.def" "EXPORTS\n")
	execute_process(COMMAND "${DUMPBIN_EXECUTABLE}" /exports "${R_HOME}/bin/x64/R.dll" ERROR_QUIET OUTPUT_VARIABLE DUMPBIN_OUTPUT)
	#Strip off the junk at the start and end
	string(REGEX MATCHALL "[ |\t]+[0-9]+[ |\t]+[0-9A-F]+[ |\t]+0[0-9A-F]+[ |\t]+[^\n|\r]+\n" R_EXPORTS_LIST_UNFILTERED "${DUMPBIN_OUTPUT}")
	foreach(symbolline IN LISTS R_EXPORTS_LIST_UNFILTERED)
		string(REGEX REPLACE "[ |\t]+[0-9]+[ |\t]+[0-9A-F]+[ |\t]+0[0-9A-F]+[ |\t]+([^\n|\r]+)\n" "\t\\1" symbol "${symbolline}")
		file(APPEND "${PROJECT_BINARY_DIR}/R.def" "${symbol}\n")
	endforeach(symbolline)
	execute_process(COMMAND "${LIB_EXECUTABLE}" /MACHINE:X64 /nodefaultlib "/def:${PROJECT_BINARY_DIR}/R.def" "/out:${PROJECT_BINARY_DIR}/R.lib" ERROR_QUIET OUTPUT_QUIET)
	file(REMOVE "${PROJECT_BINARY_DIR}/R.def")
	if(NOT EXISTS "${PROJECT_BINARY_DIR}/R.lib")
		message(FATAL_ERROR "Generating import library for R.dll - Error R.lib not found")
	endif()
	message(STATUS "Generating import library for R.dll - done")

	add_definitions(-Dsnprintf=_snprintf)

	#Put a Makefile.win in the source directory
	find_program(FULL_MAKE_PROGRAM nmake)
	set(FULL_MAKE_PROGRAM "\"${FULL_MAKE_PROGRAM}\" /f ")
	configure_file(${PROJECT_SOURCE_DIR}/Makefile.in ${PROJECT_SOURCE_DIR}/Makefile.win @ONLY)
endif()

#Make sure everything installed to the right place
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

#Now add the shared libarry target
set(SourceFiles colour.cpp imageTile.cpp imageTileComparer.cpp interface.cpp order.cpp qtPlot.cpp register.cpp validateMPCross.cpp ZoomGraphicsView.cpp)
set(HeaderFiles colour.h imageTile.h imageTileComparer.h interface.h order.h qtPlot.h validateMPCross.h ZoomGraphicsView.h)
qt5_wrap_cpp(mpMapInteractive_MOC_SOURCES ZoomGraphicsView.h qtPlot.h)
add_library(mpMapInteractive SHARED ${SourceFiles} ${HeaderFiles} ${mpMapInteractive_MOC_SOURCES})
target_link_libraries(mpMapInteractive PRIVATE Rcpp Qt5::Core Qt5::Widgets)

#Ensure that it's installed to the right place
install(TARGETS mpMapInteractive RUNTIME DESTINATION . LIBRARY DESTINATION . COMPONENT Runtime)

#If we're on windows we also need to install the QT libraries to the same place
if(WIN32)
	get_target_property(QtCore_location Qt5::Core LOCATION)
	install(FILES ${QtCore_location} DESTINATION . COMPONENT Runtime)

	GET_FILENAME_COMPONENT(QtBinaryPath ${QtCore_location} PATH)
	if(EXISTS ${QtBinaryPath}/icuin51.dll)
		install(FILES ${QtBinaryPath}/icuin51.dll DESTINATION . COMPONENT Runtime)
	endif()
	if(EXISTS ${QtBinaryPath}/icuuc51.dll)
		install(FILES ${QtBinaryPath}/icuuc51.dll DESTINATION . COMPONENT Runtime)
	endif()
	if(EXISTS ${QtBinaryPath}/icudt51.dll)
		install(FILES ${QtBinaryPath}/icudt51.dll DESTINATION . COMPONENT Runtime)
	endif()

	get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
	install(FILES ${QtWidgets_location} DESTINATION . COMPONENT Runtime)

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		get_target_property(QtGuiEgl_location Qt5::Gui_EGL LOCATION_DEBUG)
	else()
		get_target_property(QtGuiEgl_location Qt5::Gui_EGL LOCATION)
	endif()
	install(FILES ${QtGuiEgl_location} DESTINATION . COMPONENT Runtime)

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		get_target_property(QtGuiGlesV2_location Qt5::Gui_GLESv2 LOCATION_DEBUG)
	else()
		get_target_property(QtGuiGlesV2_location Qt5::Gui_GLESv2 LOCATION)
	endif()
	install(FILES ${QtGuiGlesV2_location} DESTINATION . COMPONENT Runtime)

	
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		get_target_property(QtGui_location Qt5::Gui LOCATION_DEBUG)
	else()
		get_target_property(QtGui_location Qt5::Gui LOCATION)
	endif()
	install(FILES ${QtGui_location} DESTINATION . COMPONENT Runtime)
	target_link_libraries(mpMapInteractive PRIVATE ${PROJECT_BINARY_DIR}/R.lib)

	get_filename_component(QT_LIB_DIR ${QtGui_location} DIRECTORY)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		install(FILES ${QT_LIB_DIR}/../plugins/platforms/qwindowsd.dll DESTINATION ../inst/libs/x64/platforms COMPONENT Runtime)
	else()
		install(FILES ${QT_LIB_DIR}/../plugins/platforms/qwindows.dll DESTINATION ../inst/libs/x64/platforms COMPONENT Runtime)
	endif()
endif()

add_custom_target(mpMapInteractive_install ALL COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/cmake_install.cmake")
add_dependencies(mpMapInteractive_install mpMapInteractive)
