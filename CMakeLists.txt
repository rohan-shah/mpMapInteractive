project(mpMapInteractive)
cmake_minimum_required(VERSION 3.0)
option(USE_CUSTOM_RCPP "Use custom Rcpp package" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(R REQUIRED)
if(USE_CUSTOM_RCPP)
	find_package(Rcpp REQUIRED)
endif()

set(RFiles onLoad.R qtPlot.R Rcpp_exceptions.R)

#This is the target that copies the NAMESPACE, DESCRIPTION and R files to the binary directory
add_custom_target(copyPackage ALL)
#If we're on visual studio and not using NMake makefiles, assume that we're building a visual studio project, which has multiple configuration subfolders
if(WIN32 AND (NOT USE_CUSTOM_RCPP))
	message(FATAL On windows you must specify USE_CUSTOM_RCPP)
endif()
if(WIN32 AND (NOT "${CMAKE_GENERATOR}" STREQUAL "NMake Makefiles"))
	foreach(BUILD_TYPE IN LISTS CMAKE_CONFIGURATION_TYPES)
		#If we're using a custom Rcpp, copy one DESCRIPTION file
		add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/DESCRIPTIONCustomRcpp ${CMAKE_BINARY_DIR}/${BUILD_TYPE}/DESCRIPTION)
		#Copy NAMESPACE
		add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/NAMESPACE ${CMAKE_BINARY_DIR}/${BUILD_TYPE}/NAMESPACE)
		#Copy R files
		foreach(RFile IN LISTS RFiles)
			add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/R/${RFile} ${CMAKE_BINARY_DIR}/${BUILD_TYPE}/R/${RFile})
		endforeach()
	endforeach()
#In this case we assume that we're building a single configuration
else()
	#If we're using a custom Rcpp, copy one DESCRIPTION file
	if(USE_CUSTOM_RCPP)
		add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/DESCRIPTIONCustomRcpp ${CMAKE_BINARY_DIR}/DESCRIPTION)
	#otherwise use the other
	else()
		add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/DESCRIPTION ${CMAKE_BINARY_DIR}/DESCRIPTION)
	endif()
	#Copy NAMESPACE
	add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/NAMESPACE ${CMAKE_BINARY_DIR}/NAMESPACE)
	#Copy R files
	foreach(RFile IN LISTS RFiles)
		add_custom_command(TARGET copyPackage COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/R/${RFile} ${CMAKE_BINARY_DIR}/R/${RFile})
	endforeach()
endif()
add_subdirectory(src)

if(WIN32)
	if("${CMAKE_GENERATOR}" STREQUAL "NMake Makefiles")
		install(CODE "execute_process(COMMAND \"${R_COMMAND}\" CMD INSTALL --no-multiarch . WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\")")
	else()
		install(CODE "execute_process(COMMAND \"${R_COMMAND}\" CMD INSTALL --no-multiarch \${CMAKE_INSTALL_CONFIG_NAME} WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\")")
	endif()
else()
	install(CODE "execute_process(COMMAND \"${R_COMMAND}\" CMD INSTALL . WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\")")
endif()
