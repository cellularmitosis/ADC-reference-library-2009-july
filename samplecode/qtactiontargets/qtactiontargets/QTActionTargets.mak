# Microsoft Developer Studio Generated NMAKE File, Based on QTActionTargets.dsp
!IF "$(CFG)" == ""
CFG=QTActionTargets - Win32 Debug
!MESSAGE No configuration specified. Defaulting to QTActionTargets - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "QTActionTargets - Win32 Release" && "$(CFG)" != "QTActionTargets - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTActionTargets.mak" CFG="QTActionTargets - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QTActionTargets - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QTActionTargets - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "QTActionTargets - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\QTActionTargets.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTActionTargets.obj"
	-@erase "$(INTDIR)\QTActionTargets.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTActionTargets.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\Common Files" /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\QTActionTargets.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTActionTargets.res" /i ".\Application Files" /i ".\Common Files" /i "." /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTActionTargets.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\QTActionTargets.pdb" /machine:I386 /out:"$(OUTDIR)\QTActionTargets.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\QTActionTargets.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj" \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\QTActionTargets.res" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WinFramework.obj"

"$(OUTDIR)\QTActionTargets.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Adding Macintosh Resources
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTActionTargets.exe"
   MyRezWackRelease.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "QTActionTargets - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\QTActionTargets.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTActionTargets.obj"
	-@erase "$(INTDIR)\QTActionTargets.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTActionTargets.exe"
	-@erase "$(OUTDIR)\QTActionTargets.ilk"
	-@erase "$(OUTDIR)\QTActionTargets.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\Common Files" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\QTActionTargets.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTActionTargets.res" /i ".\Application Files" /i ".\Common Files" /i "." /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTActionTargets.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTActionTargets.pdb" /debug /machine:I386 /out:"$(OUTDIR)\QTActionTargets.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\QTActionTargets.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj" \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\QTActionTargets.res" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WinFramework.obj"

"$(OUTDIR)\QTActionTargets.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=Adding Macintosh Resources
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTActionTargets.exe"
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
!IF EXISTS("QTActionTargets.dep")
!INCLUDE "QTActionTargets.dep"
!ELSE 
!MESSAGE Warning: cannot find "QTActionTargets.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QTActionTargets - Win32 Release" || "$(CFG)" == "QTActionTargets - Win32 Debug"
SOURCE=".\Application Files\ComApplication.c"

"$(INTDIR)\ComApplication.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ComFramework.c"

"$(INTDIR)\ComFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\EndianUtilities.c"

"$(INTDIR)\EndianUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ImageCompressionUtilities.c"

"$(INTDIR)\ImageCompressionUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\QTActionTargets.c

"$(INTDIR)\QTActionTargets.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=".\Application Files\QTActionTargets.rc"

!IF  "$(CFG)" == "QTActionTargets - Win32 Release"


"$(INTDIR)\QTActionTargets.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTActionTargets.res" /i ".\Application Files" /i ".\Common Files" /i "." /i "Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "QTActionTargets - Win32 Debug"


"$(INTDIR)\QTActionTargets.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTActionTargets.res" /i ".\Application Files" /i ".\Common Files" /i "." /i "Application Files" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\QTUtilities.c"

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\SpriteUtilities.c"

"$(INTDIR)\SpriteUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\WinFramework.c"

"$(INTDIR)\WinFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\WiredSpriteUtilities.c

"$(INTDIR)\WiredSpriteUtilities.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

