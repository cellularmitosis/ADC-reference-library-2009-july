# Microsoft Developer Studio Generated NMAKE File, Based on QTWiredActions.dsp
!IF "$(CFG)" == ""
CFG=QTWiredActions - Win32 Debug
!MESSAGE No configuration specified. Defaulting to QTWiredActions - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "QTWiredActions - Win32 Release" && "$(CFG)" != "QTWiredActions - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTWiredActions.mak" CFG="QTWiredActions - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QTWiredActions - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QTWiredActions - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "QTWiredActions - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\QTWiredActions.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\QTWiredActions.obj"
	-@erase "$(INTDIR)\QTWiredActions.res"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTWiredActions.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\Common Files" /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\QTWiredActions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTWiredActions.res" /i ".\Application Files" /i ".\Common Files" /i "." /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTWiredActions.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\QTWiredActions.pdb" /machine:I386 /out:"$(OUTDIR)\QTWiredActions.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\QTWiredActions.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj" \
	"$(INTDIR)\QTWiredActions.res"

"$(OUTDIR)\QTWiredActions.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTWiredActions.exe"
   MyRezWackRelease.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "QTWiredActions - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\QTWiredActions.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\QTWiredActions.obj"
	-@erase "$(INTDIR)\QTWiredActions.res"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTWiredActions.exe"
	-@erase "$(OUTDIR)\QTWiredActions.ilk"
	-@erase "$(OUTDIR)\QTWiredActions.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\Common Files" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\QTWiredActions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTWiredActions.res" /i ".\Application Files" /i ".\Common Files" /i "." /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTWiredActions.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTWiredActions.pdb" /debug /machine:I386 /out:"$(OUTDIR)\QTWiredActions.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\QTWiredActions.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj" \
	"$(INTDIR)\QTWiredActions.res"

"$(OUTDIR)\QTWiredActions.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTWiredActions.exe"
   MyRezWackDebug.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("QTWiredActions.dep")
!INCLUDE "QTWiredActions.dep"
!ELSE 
!MESSAGE Warning: cannot find "QTWiredActions.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QTWiredActions - Win32 Release" || "$(CFG)" == "QTWiredActions - Win32 Debug"
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


SOURCE=".\Common Files\QTUtilities.c"

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\QTWiredActions.c

"$(INTDIR)\QTWiredActions.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=".\Application Files\QTWiredActions.rc"

!IF  "$(CFG)" == "QTWiredActions - Win32 Release"


"$(INTDIR)\QTWiredActions.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTWiredActions.res" /i ".\Application Files" /i ".\Common Files" /i "." /i "Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "QTWiredActions - Win32 Debug"


"$(INTDIR)\QTWiredActions.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTWiredActions.res" /i ".\Application Files" /i ".\Common Files" /i "." /i "Application Files" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\SpriteUtilities.c"

"$(INTDIR)\SpriteUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\WinFramework.c"

"$(INTDIR)\WinFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\WiredSpriteUtilities.c

"$(INTDIR)\WiredSpriteUtilities.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

