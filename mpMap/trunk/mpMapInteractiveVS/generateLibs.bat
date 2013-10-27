cd ..
if not exist libs (
	mkdir libs
)
cd libs
if %1 == x64 (
	if not exist x64 (
		mkdir x64
	)
	cd x64
	if not exist R.lib (
		echo Generating R.lib, 64-bit...
		echo LIBRARY R > R.def
		echo EXPORTS >> R.def
		dumpbin /exports "%R_HOME%/bin/x64/R.dll" | sed -n "s/     ...... .... 0....... \([^ ]*\)/    \1/p" >> R.def
		lib /machine:X64 /nodefaultlib /def:R.def
		echo Finished generating R.lib
	)
	if not exist mpMap.lib (
		echo Generating mpMap.lib, 64-bit...
		echo LIBRARY mpMap > mpMap.def
		echo EXPORTS >> mpMap.def
		dumpbin /exports "%R_HOME%/library/mpMap/libs/x64/mpMap.dll" | sed -n "s/     ...... .... 0....... \([^ ]*\)/    \1/p" >> mpMap.def
		lib /machine:X64 /nodefaultlib /def:mpMap.def
		echo Finished generating R.lib
	)
	cd ..
)
if %1 == Win32 (
	if not exist i386 (
		mkdir i386
	)
	cd x64
	if not exist R.lib (
		echo Generating R.lib, 32-bit...
		echo LIBRARY R > R.def
		echo EXPORTS >> R.def
		dumpbin /exports "%R_HOME%/bin/i386/R.dll" | sed -n "s/     ...... .... 0....... \([^ ]*\)/    \1/p" >> R.def
		lib /machine:IX386 /nodefaultlib /def:R.def
		echo Finished generating R.lib
	)
	if not exist mpMap.lib (
		echo Generating mpMap.lib, 32-bit...
		echo LIBRARY mpMap > mpMap.def
		echo EXPORTS >> mpMap.def
		dumpbin /exports "%R_HOME%/library/mpMap/libs/i386/mpMap.dll" | sed -n "s/     ...... .... 0....... \([^ ]*\)/    \1/p" >> mpMap.def
		lib /machine:IX386 /nodefaultlib /def:mpMap.def
		echo Finished generating R.lib
	)
	cd ..
)
cd ..