# Microsoft Developer Studio Generated NMAKE File, Based on QTEffects.dsp
!IF "$(CFG)" == ""
CFG=QTEffects - Win32 Release
!MESSAGE No configuration specified. Defaulting to QTEffects - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "QTEffects - Win32 Release" && "$(CFG)" != "QTEffects - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTEffects.mak" CFG="QTEffects - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QTEffects - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QTEffects - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "QTEffects - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\QTEffects.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EffectsUtilities.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTEffects.obj"
	-@erase "$(INTDIR)\QTEffects.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTEffects.exe"
	-@erase "$(OUTDIR)\QTEffects.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "." /I ".\Common Files" /I ".\Application Files" /I "..\..\QTDevWin\CIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTEffects.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTEffects.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTEffects.pdb" /machine:I386 /out:"$(OUTDIR)\QTEffects.exe" /libpath:".\..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\QTEffects.res" \
	"$(INTDIR)\QTEffects.obj" \
	"$(INTDIR)\EffectsUtilities.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj"

"$(OUTDIR)\QTEffects.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QTEffects - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\QTEffects.exe"


CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\EffectsUtilities.obj"
	-@erase "$(INTDIR)\EndianUtilities.obj"
	-@erase "$(INTDIR)\ImageCompressionUtilities.obj"
	-@erase "$(INTDIR)\QTEffects.obj"
	-@erase "$(INTDIR)\QTEffects.res"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\SpriteUtilities.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(INTDIR)\WiredSpriteUtilities.obj"
	-@erase "$(OUTDIR)\QTEffects.exe"
	-@erase "$(OUTDIR)\QTEffects.ilk"
	-@erase "$(OUTDIR)\QTEffects.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "." /I ".\Common Files" /I ".\Application Files" /I "..\..\QTDevWin\CIncludes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\QTEffects.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTEffects.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\QTEffects.pdb" /debug /machine:I386 /out:"$(OUTDIR)\QTEffects.exe" /pdbtype:sept /libpath:".\..\..\QTDevWin\Libraries" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\WinFramework.obj" \
	"$(INTDIR)\QTEffects.res" \
	"$(INTDIR)\QTEffects.obj" \
	"$(INTDIR)\EffectsUtilities.obj" \
	"$(INTDIR)\EndianUtilities.obj" \
	"$(INTDIR)\ImageCompressionUtilities.obj" \
	"$(INTDIR)\SpriteUtilities.obj" \
	"$(INTDIR)\WiredSpriteUtilities.obj"

"$(OUTDIR)\QTEffects.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("QTEffects.dep")
!INCLUDE "QTEffects.dep"
!ELSE 
!MESSAGE Warning: cannot find "QTEffects.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QTEffects - Win32 Release" || "$(CFG)" == "QTEffects - Win32 Debug"
SOURCE=".\Application Files\ComApplication.c"

"$(INTDIR)\ComApplication.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ComFramework.c"

"$(INTDIR)\ComFramework.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\EffectsUtilities.c"

"$(INTDIR)\EffectsUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\EndianUtilities.c"

"$(INTDIR)\EndianUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=".\Common Files\ImageCompressionUtilities.c"

"$(INTDIR)\ImageCompressionUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\QTEffects.c

"$(INTDIR)\QTEffects.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=".\Application Files\QTEffects.rc"

!IF  "$(CFG)" == "QTEffects - Win32 Release"


"$(INTDIR)\QTEffects.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTEffects.res" /i "Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "QTEffects - Win32 Debug"


"$(INTDIR)\QTEffects.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\QTEffects.res" /i "Application Files" /d "_DEBUG" $(SOURCE)


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


SOURCE=".\Common Files\WiredSpriteUtilities.c"

"$(INTDIR)\WiredSpriteUtilities.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

