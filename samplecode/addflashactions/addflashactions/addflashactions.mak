# Microsoft Developer Studio Generated NMAKE File, Based on addflashactions.dsp
!IF "$(CFG)" == ""
CFG=addflashactions - Win32 Debug
!MESSAGE No configuration specified. Defaulting to addflashactions - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "addflashactions - Win32 Release" && "$(CFG)" != "addflashactions - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "addflashactions.mak" CFG="addflashactions - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "addflashactions - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "addflashactions - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "addflashactions - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\addflashactions.exe"


CLEAN :
	-@erase "$(INTDIR)\AddFlashActions.obj"
	-@erase "$(INTDIR)\FlashParser.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\addflashactions.exe"
	-@erase "$(OUTDIR)\addflashactions.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\addflashactions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\addflashactions.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\addflashactions.pdb" /machine:I386 /out:"$(OUTDIR)\addflashactions.exe" /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\AddFlashActions.obj" \
	"$(INTDIR)\FlashParser.obj"

"$(OUTDIR)\addflashactions.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "addflashactions - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\addflashactions.exe"


CLEAN :
	-@erase "$(INTDIR)\AddFlashActions.obj"
	-@erase "$(INTDIR)\FlashParser.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\addflashactions.exe"
	-@erase "$(OUTDIR)\addflashactions.ilk"
	-@erase "$(OUTDIR)\addflashactions.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\addflashactions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\addflashactions.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\addflashactions.pdb" /debug /machine:I386 /out:"$(OUTDIR)\addflashactions.exe" /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\AddFlashActions.obj" \
	"$(INTDIR)\FlashParser.obj"

"$(OUTDIR)\addflashactions.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("addflashactions.dep")
!INCLUDE "addflashactions.dep"
!ELSE 
!MESSAGE Warning: cannot find "addflashactions.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "addflashactions - Win32 Release" || "$(CFG)" == "addflashactions - Win32 Debug"
SOURCE=.\AddFlashActions.c

"$(INTDIR)\AddFlashActions.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FlashParser.c

"$(INTDIR)\FlashParser.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

