if not exist ..\..\mpMapInteractiveRPackage\inst (
	mkdir ..\..\mpMapInteractiveRPackage\inst
)
if not exist ..\..\mpMapInteractiveRPackage\inst\libs (
	mkdir ..\mpMapInteractiveRPackage\inst\libs
)

if not exist ..\..\mpMapInteractiveRPackage\inst\libs\x64 (
	mkdir ..\..\mpMapInteractiveRPackage\inst\libs\x64
)
if not exist ..\..\mpMapInteractiveRPackage\inst\libs\i386 (
	mkdir ..\..\mpMapInteractiveRPackage\inst\libs\i386
)

rm ../../mpMapInteractiveRPackage/inst/libs/%1/*
cp ..\%1\%2\mpMapInteractive.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1

Qt5Widgetsd.lib;$(QTDIR)\lib\Qt5Guid.lib;$(QTDIR)\lib\Qt5Cored.lib;libEGLd.lib;libGLESv2d.lib;gdi32.lib;user32.lib;Qt5Widgetsd.lib;%(AdditionalDependencies)
if %2 == Debug (
	cp %3\bin\Qt5Widgetsd.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\Qt5Guid.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\libEGLd.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\Qt5Cored.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\libGLESv2d.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1

	if not exist ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms (
		mkdir ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms
	)
	cp %3\plugins\platforms\qwindowsd.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms

)
if %2 == Release (
	cp %3\bin\Qt5Widgets.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\Qt5Gui.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\libEGL.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\Qt5Core.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	cp %3\bin\libGLESv2.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
	
	if not exist ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms (
		mkdir ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms
	)
	cp %3\plugins\platforms\qwindows.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1\platforms
)

cp %3\bin\icudt51.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
cp %3\bin\icuin51.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1
cp %3\bin\icuuc51.dll ..\..\mpMapInteractiveRPackage\inst\libs\%1