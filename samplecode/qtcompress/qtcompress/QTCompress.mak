# Microsoft Developer Studio Generated NMAKE File, Based on QTCompress.dsp
!IF "$(CFG)" == ""
CFG=QTCompress - Win32 Debug
!MESSAGE No configuration specified. Defaulting to QTCompress - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "QTCompress - Win32 Release" && "$(CFG)" != "QTCompress - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTCompress.mak" CFG="QTCompress - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QTCompress - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QTCompress - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "QTCompress - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\QTCompress.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\QTCompress.obj"
	-@erase "$(INTDIR)\QTCompress.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\QTCompress.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\." /I ".\Common Files" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\QTCompress.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTCompress.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTCompress.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\QTCompress.pdb" /machine:I386 /out:"$(OUTDIR)\QTCompress.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTCompress.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\QTCompress.res"

"$(OUTDIR)\QTCompress.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QTCompress - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\QTCompress.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\QTCompress.obj"
	-@erase "$(INTDIR)\QTCompress.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\QTCompress.exe"
	-@erase "$(OUTDIR)\QTCompress.ilk"
	-@erase "$(OUTDIR)\QTCompress.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /I ".\Application Files" /I ".\." /I ".\Common Files" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\QTCompress.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTCompress.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTCompress.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTCompress.pdb" /debug /machine:I386 /out:"$(OUTDIR)\QTCompress.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTCompress.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\QTCompress.res"

"$(OUTDIR)\QTCompress.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

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
!IF EXISTS("QTCompress.dep")
!INCLUDE "QTCompress.dep"
!ELSE 
!MESSAGE Warning: cannot find "QTCompress.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QTCompress - Win32 Release" || "$(CFG)" == "QTCompress - Win32 Debug"
SOURCE=".\Application Files\ComApplication.c"

"$(INTDIR)\ComApplication.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ComFramework.c"

"$(INTDIR)\ComFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\QTCompress.c

"$(INTDIR)\QTCompress.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=".\Application Files\QTCompress.rc"

!IF  "$(CFG)" == "QTCompress - Win32 Release"


"$(INTDIR)\QTCompress.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTCompress.res" /i "Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "QTCompress - Win32 Debug"


"$(INTDIR)\QTCompress.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTCompress.res" /i "Application Files" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\QTUtilities.c"

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\WinFramework.c"

"$(INTDIR)\WinFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

