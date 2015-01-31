set(SourceFiles colour.cpp imageTile.cpp imageTileComparer.cpp interface.cpp order.cpp qtPlot.cpp register.cpp validateMPCross.cpp ZoomGraphicsView.cpp)
set(HeaderFiles colour.h imageTile.h imageTileComparer.h interface.h order.h qtPlot.h validateMPCross.h ZoomGraphicsView.h)
qt5_wrap_cpp(mpMapInteractive_MOC_SOURCES ZoomGraphicsView.h qtPlot.h)
add_library(mpMapInteractive SHARED ${SourceFiles} ${HeaderFiles} ${mpMapInteractive_MOC_SOURCES})
target_link_libraries(mpMapInteractive PRIVATE Rcpp Qt5::Core Qt5::Widgets)

install(TARGETS mpMapInteractive RUNTIME DESTINATION . COMPONENT Runtime)
if(WIN32)
	get_target_property(QtCore_location Qt5::Core LOCATION)
	install(FILES ${QtCore_location} DESTINATION . COMPONENT Runtime)

	GET_FILENAME_COMPONENT(QtBinaryPath ${QtCore_location} PATH)
	install(FILES ${QtBinaryPath}/icuin51.dll DESTINATION . COMPONENT Runtime)
	install(FILES ${QtBinaryPath}/icuuc51.dll DESTINATION . COMPONENT Runtime)
	install(FILES ${QtBinaryPath}/icudt51.dll DESTINATION . COMPONENT Runtime)

	get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
	install(FILES ${QtWidgets_location} DESTINATION . COMPONENT Runtime)

	get_target_property(QtGuiEgl_location Qt5::Gui_EGL LOCATION)
	install(FILES ${QtGuiEgl_location} DESTINATION . COMPONENT Runtime)

	get_target_property(QtGuiGlesV2_location Qt5::Gui_GLESv2 LOCATION)
	install(FILES ${QtGuiGlesV2_location} DESTINATION . COMPONENT Runtime)

	get_target_property(QtGui_location Qt5::Gui LOCATION)
	install(FILES ${QtGui_location} DESTINATION . COMPONENT Runtime)
endif()