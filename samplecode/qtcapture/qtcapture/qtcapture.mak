# Microsoft Developer Studio Generated NMAKE File, Based on qtcapture.dsp
!IF "$(CFG)" == ""
CFG=qtcapture - Win32 Debug
!MESSAGE No configuration specified. Defaulting to qtcapture - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "qtcapture - Win32 Release" && "$(CFG)" != "qtcapture - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qtcapture.mak" CFG="qtcapture - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qtcapture - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qtcapture - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qtcapture - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\qtcapture.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\qtcapture.obj"
	-@erase "$(INTDIR)\qtcapture.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\qtcapture.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I ".\." /I ".\Application Files" /I ".\Common Files" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\qtcapture.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\qtcapture.res" /i ".\Common Files" /i ".\Application Files" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\qtcapture.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\qtcapture.pdb" /machine:I386 /out:"$(OUTDIR)\qtcapture.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\qtcapture.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\qtcapture.res" \
	"$(INTDIR)\EndianUtilities.obj"

"$(OUTDIR)\qtcapture.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Adding Macintosh resources
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\qtcapture.exe"
   MyRezWackRelease.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "qtcapture - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\qtcapture.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\qtcapture.obj"
	-@erase "$(INTDIR)\qtcapture.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\qtcapture.exe"
	-@erase "$(OUTDIR)\qtcapture.ilk"
	-@erase "$(OUTDIR)\qtcapture.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /I ".\." /I ".\Application Files" /I ".\Common Files" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\qtcapture.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\qtcapture.res" /i ".\Common Files" /i ".\Application Files" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\qtcapture.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\qtcapture.pdb" /debug /machine:I386 /out:"$(OUTDIR)\qtcapture.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\qtcapture.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\qtcapture.res" \
	"$(INTDIR)\EndianUtilities.obj"

"$(OUTDIR)\qtcapture.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Adding Macintosh resources
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\qtcapture.exe"
   MyRezWackDebug.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("qtcapture.dep")
!INCLUDE "qtcapture.dep"
!ELSE 
!MESSAGE Warning: cannot find "qtcapture.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "qtcapture - Win32 Release" || "$(CFG)" == "qtcapture - Win32 Debug"
SOURCE=".\Application Files\ComApplication.c"

"$(INTDIR)\ComApplication.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ComFramework.c"

"$(INTDIR)\ComFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\EndianUtilities.c"

"$(INTDIR)\EndianUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\qtcapture.c

"$(INTDIR)\qtcapture.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=".\Application Files\qtcapture.rc"

!IF  "$(CFG)" == "qtcapture - Win32 Release"


"$(INTDIR)\qtcapture.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\qtcapture.res" /i ".\Common Files" /i ".\Application Files" /i "Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "qtcapture - Win32 Debug"


"$(INTDIR)\qtcapture.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\qtcapture.res" /i ".\Common Files" /i ".\Application Files" /i "Application Files" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\QTUtilities.c"

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\WinFramework.c"

"$(INTDIR)\WinFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

