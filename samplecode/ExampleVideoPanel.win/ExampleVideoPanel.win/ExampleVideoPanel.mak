# Microsoft Developer Studio Generated NMAKE File, Based on ExampleVideoPanel.dsp
!IF "$(CFG)" == ""
CFG=ExampleVideoPanel - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ExampleVideoPanel - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ExampleVideoPanel - Win32 Release" && "$(CFG)" != "ExampleVideoPanel - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ExampleVideoPanel.mak" CFG="ExampleVideoPanel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ExampleVideoPanel - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ExampleVideoPanel - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "ExampleVideoPanel - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : ".\Release\ExampleVideoPanel.qtr" "$(OUTDIR)\ExampleVideoPanel.qtx"


CLEAN :
	-@erase "$(INTDIR)\Dllmain.obj"
	-@erase "$(INTDIR)\ExampleVideoPanel.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ExampleVideoPanel.exp"
	-@erase "$(OUTDIR)\ExampleVideoPanel.ilk"
	-@erase "$(OUTDIR)\ExampleVideoPanel.lib"
	-@erase "$(OUTDIR)\ExampleVideoPanel.qtx"
	-@erase ".\Release\ExampleVideoPanel.qtr"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Includes" /I ".\PrefixIncludes" /I ".\Resources" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXAMPLEVIDEOPANEL_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ExampleVideoPanel.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\ExampleVideoPanel.pdb" /machine:I386 /def:".\ExampleVideoPanel.def" /out:"$(OUTDIR)\ExampleVideoPanel.qtx" /implib:"$(OUTDIR)\ExampleVideoPanel.lib" 
DEF_FILE= \
	".\ExampleVideoPanel.def"
LINK32_OBJS= \
	"$(INTDIR)\Dllmain.obj" \
	"$(INTDIR)\ExampleVideoPanel.obj"

"$(OUTDIR)\ExampleVideoPanel.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
TargetName=ExampleVideoPanel
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\Release\ExampleVideoPanel.qtr" "$(OUTDIR)\ExampleVideoPanel.qtx"
   attrib -R .\Release\ExampleVideoPanel.qtx
	..\..\QTDevWin\Tools\RezWack -d .\Release\ExampleVideoPanel.qtx -r     .\Release\ExampleVideoPanel.qtr -o .\Release\TempExampleVideoPanel.qtx
	attrib -R     .\Release\TempExampleVideoPanel.qtx
	attrib -R .\Release\ExampleVideoPanel.qtx
	del     .\Release\ExampleVideoPanel.qtx
	ren .\Release\TempExampleVideoPanel.qtx     ExampleVideoPanel.qtx
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ExampleVideoPanel - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : ".\Debug\ExampleVideoPanel.qtr" "$(OUTDIR)\ExampleVideoPanel.qtx"


CLEAN :
	-@erase "$(INTDIR)\Dllmain.obj"
	-@erase "$(INTDIR)\ExampleVideoPanel.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ExampleVideoPanel.exp"
	-@erase "$(OUTDIR)\ExampleVideoPanel.ilk"
	-@erase "$(OUTDIR)\ExampleVideoPanel.lib"
	-@erase "$(OUTDIR)\ExampleVideoPanel.pdb"
	-@erase "$(OUTDIR)\ExampleVideoPanel.qtx"
	-@erase ".\Debug\ExampleVideoPanel.qtr"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Includes" /I ".\PrefixIncludes" /I ".\Resources" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXAMPLEVIDEOPANEL_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ExampleVideoPanel.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\ExampleVideoPanel.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /def:".\ExampleVideoPanel.def" /out:"$(OUTDIR)\ExampleVideoPanel.qtx" /implib:"$(OUTDIR)\ExampleVideoPanel.lib" /pdbtype:sept 
DEF_FILE= \
	".\ExampleVideoPanel.def"
LINK32_OBJS= \
	"$(INTDIR)\Dllmain.obj" \
	"$(INTDIR)\ExampleVideoPanel.obj"

"$(OUTDIR)\ExampleVideoPanel.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetName=ExampleVideoPanel
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\Debug\ExampleVideoPanel.qtr" "$(OUTDIR)\ExampleVideoPanel.qtx"
   attrib -R .\Debug\ExampleVideoPanel.qtx
	..\..\QTDevWin\Tools\RezWack -d .\Debug\ExampleVideoPanel.qtx -r     .\Debug\ExampleVideoPanel.qtr -o .\Debug\TempExampleVideoPanel.qtx
	attrib -R     .\Debug\TempExampleVideoPanel.qtx
	attrib -R .\Debug\ExampleVideoPanel.qtx
	del     .\Debug\ExampleVideoPanel.qtx
	ren .\Debug\TempExampleVideoPanel.qtx     ExampleVideoPanel.qtx
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
!IF EXISTS("ExampleVideoPanel.dep")
!INCLUDE "ExampleVideoPanel.dep"
!ELSE 
!MESSAGE Warning: cannot find "ExampleVideoPanel.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ExampleVideoPanel - Win32 Release" || "$(CFG)" == "ExampleVideoPanel - Win32 Debug"
SOURCE=.\Sources\Dllmain.c

"$(INTDIR)\Dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ExamplePanelWin.r

!IF  "$(CFG)" == "ExampleVideoPanel - Win32 Release"

TargetDir=.\Release
TargetName=ExampleVideoPanel
InputPath=.\ExamplePanelWin.r

"$(INTDIR)\ExampleVideoPanel.qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i      ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)
<< 
	

!ELSEIF  "$(CFG)" == "ExampleVideoPanel - Win32 Debug"

TargetDir=.\Debug
TargetName=ExampleVideoPanel
InputPath=.\ExamplePanelWin.r

"$(INTDIR)\ExampleVideoPanel.qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i      ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)
<< 
	

!ENDIF 

SOURCE=.\Sources\ExampleVideoPanel.c

"$(INTDIR)\ExampleVideoPanel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

