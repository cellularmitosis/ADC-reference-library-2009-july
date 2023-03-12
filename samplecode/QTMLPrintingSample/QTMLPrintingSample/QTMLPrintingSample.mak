# Microsoft Developer Studio Generated NMAKE File, Based on QTMLPrintingSample.dsp
!IF "$(CFG)" == ""
CFG=QTMLPrintingSample - Win32 Debug
!MESSAGE No configuration specified. Defaulting to QTMLPrintingSample - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "QTMLPrintingSample - Win32 Release" && "$(CFG)" != "QTMLPrintingSample - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTMLPrintingSample.mak" CFG="QTMLPrintingSample - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QTMLPrintingSample - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QTMLPrintingSample - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "QTMLPrintingSample - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\QTMLPrintingSample.exe"


CLEAN :
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\QTCode.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\QTMLPrintingSample.exe"
	-@erase "$(OUTDIR)\QTMLPrintingSample.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\CommonFiles" /I "..\Application Files" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTMLPrintingSample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libc.lib qtmlclient.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTMLPrintingSample.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\QTMLPrintingSample.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\QTCode.obj"

"$(OUTDIR)\QTMLPrintingSample.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QTMLPrintingSample - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\QTMLPrintingSample.exe"


CLEAN :
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\QTCode.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\QTMLPrintingSample.exe"
	-@erase "$(OUTDIR)\QTMLPrintingSample.ilk"
	-@erase "$(OUTDIR)\QTMLPrintingSample.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\CommonFiles" /I "..\Application Files" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTMLPrintingSample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcd.lib qtmlclient.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTMLPrintingSample.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\QTMLPrintingSample.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\QTCode.obj"

"$(OUTDIR)\QTMLPrintingSample.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("QTMLPrintingSample.dep")
!INCLUDE "QTMLPrintingSample.dep"
!ELSE 
!MESSAGE Warning: cannot find "QTMLPrintingSample.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QTMLPrintingSample - Win32 Release" || "$(CFG)" == "QTMLPrintingSample - Win32 Debug"
SOURCE=.\Print.c

"$(INTDIR)\Print.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\QTCode.c

"$(INTDIR)\QTCode.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

