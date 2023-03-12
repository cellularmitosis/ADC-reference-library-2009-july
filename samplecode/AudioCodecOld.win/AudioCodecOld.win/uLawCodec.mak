# Microsoft Developer Studio Generated NMAKE File, Based on uLawCodec.dsp
!IF "$(CFG)" == ""
CFG=uLawCodec. - Win32 Release
!MESSAGE No configuration specified. Defaulting to uLawCodec. - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "uLawCodec. - Win32 Release" && "$(CFG)" !=\
 "uLawCodec. - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uLawCodec.mak" CFG="uLawCodec. - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uLawCodec. - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "uLawCodec. - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "uLawCodec. - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\.\Release
TargetName=uLawCodec
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\uLawCodec.qtx"

!ELSE 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\uLawCodec.qtx"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\uLawCompressor.obj"
	-@erase "$(INTDIR)\uLawDecompressor.obj"
	-@erase "$(OUTDIR)\uLawCodec.exp"
	-@erase "$(OUTDIR)\uLawCodec.lib"
	-@erase "$(OUTDIR)\uLawCodec.qtx"
	-@erase "$(OutDir)\resource.frk\$(TargetName)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /I\
 "..\..\QTDevWin\ComponentIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\uLawCodec.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libc.lib ..\..\QTDevWin\Libraries\qtmlclient.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\uLawCodec.pdb"\
 /machine:I386 /nodefaultlib /def:".\uLawCodec.def"\
 /out:"$(OUTDIR)\uLawCodec.qtx" /implib:"$(OUTDIR)\uLawCodec.lib" 
DEF_FILE= \
	".\uLawCodec.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\uLawCompressor.obj" \
	"$(INTDIR)\uLawDecompressor.obj"

"$(OUTDIR)\uLawCodec.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "uLawCodec. - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\.\Debug
TargetName=uLawCodec
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\uLawCodec.qtx"

!ELSE 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\uLawCodec.qtx"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\uLawCompressor.obj"
	-@erase "$(INTDIR)\uLawDecompressor.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\uLawCodec.exp"
	-@erase "$(OUTDIR)\uLawCodec.ilk"
	-@erase "$(OUTDIR)\uLawCodec.lib"
	-@erase "$(OUTDIR)\uLawCodec.pdb"
	-@erase "$(OUTDIR)\uLawCodec.qtx"
	-@erase "$(OutDir)\resource.frk\$(TargetName)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\QTDevWin\CIncludes" /I\
 "..\..\QTDevWin\ComponentIncludes" /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\uLawCodec.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libcd.lib ..\..\QTDevWin\Libraries\qtmlclient.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\uLawCodec.pdb" /debug\
 /machine:I386 /nodefaultlib /def:".\uLawCodec.def"\
 /out:"$(OUTDIR)\uLawCodec.qtx" /implib:"$(OUTDIR)\uLawCodec.lib" 
DEF_FILE= \
	".\uLawCodec.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\uLawCompressor.obj" \
	"$(INTDIR)\uLawDecompressor.obj"

"$(OUTDIR)\uLawCodec.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "uLawCodec. - Win32 Release" || "$(CFG)" ==\
 "uLawCodec. - Win32 Debug"
SOURCE=.\dllmain.c

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\uLawCodec.r

!IF  "$(CFG)" == "uLawCodec. - Win32 Release"

OutDir=.\.\Release
TargetPath=.\Release\uLawCodec.qtx
TargetName=uLawCodec
InputPath=.\uLawCodec.r

"$(OutDir)\resource.frk\$(TargetName)"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez   -i   ..\..\QTDevWin\RIncludes -i\
      ..\..\QTDevWin\CIncludes\ -o    $(TargetPath)  $(InputPath)

!ELSEIF  "$(CFG)" == "uLawCodec. - Win32 Debug"

OutDir=.\.\Debug
TargetPath=.\Debug\uLawCodec.qtx
TargetName=uLawCodec
InputPath=.\uLawCodec.r

"$(OutDir)\resource.frk\$(TargetName)"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez   -i   ..\..\QTDevWin\RIncludes -i\
      ..\..\QTDevWin\CIncludes\ -o    $(TargetPath)  $(InputPath)

!ENDIF 

SOURCE=.\uLawCompressor.c

!IF  "$(CFG)" == "uLawCodec. - Win32 Release"

DEP_CPP_ULAWC=\
	"..\..\QTDevWin\CIncludes\AEDataModel.h"\
	"..\..\QTDevWin\CIncludes\Aliases.h"\
	"..\..\QTDevWin\CIncludes\AppleEvents.h"\
	"..\..\QTDevWin\CIncludes\AppleTalk.h"\
	"..\..\QTDevWin\CIncludes\CodeFragments.h"\
	"..\..\QTDevWin\CIncludes\Collections.h"\
	"..\..\QTDevWin\CIncludes\Components.h"\
	"..\..\QTDevWin\CIncludes\ConditionalMacros.h"\
	"..\..\QTDevWin\CIncludes\Controls.h"\
	"..\..\QTDevWin\CIncludes\DateTimeUtils.h"\
	"..\..\QTDevWin\CIncludes\Dialogs.h"\
	"..\..\QTDevWin\CIncludes\Drag.h"\
	"..\..\QTDevWin\CIncludes\Endian.h"\
	"..\..\QTDevWin\CIncludes\Errors.h"\
	"..\..\QTDevWin\CIncludes\Events.h"\
	"..\..\QTDevWin\CIncludes\Files.h"\
	"..\..\QTDevWin\CIncludes\Finder.h"\
	"..\..\QTDevWin\CIncludes\Icons.h"\
	"..\..\QTDevWin\CIncludes\ImageCompression.h"\
	"..\..\QTDevWin\CIncludes\MacMemory.h"\
	"..\..\QTDevWin\CIncludes\MacTypes.h"\
	"..\..\QTDevWin\CIncludes\MacWindows.h"\
	"..\..\QTDevWin\CIncludes\Menus.h"\
	"..\..\QTDevWin\CIncludes\MixedMode.h"\
	"..\..\QTDevWin\CIncludes\Movies.h"\
	"..\..\QTDevWin\CIncludes\MoviesFormat.h"\
	"..\..\QTDevWin\CIncludes\Notification.h"\
	"..\..\QTDevWin\CIncludes\OSUtils.h"\
	"..\..\QTDevWin\CIncludes\Patches.h"\
	"..\..\QTDevWin\CIncludes\Processes.h"\
	"..\..\QTDevWin\CIncludes\QDOffscreen.h"\
	"..\..\QTDevWin\CIncludes\Quickdraw.h"\
	"..\..\QTDevWin\CIncludes\QuickdrawText.h"\
	"..\..\QTDevWin\CIncludes\Resources.h"\
	"..\..\QTDevWin\CIncludes\Sound.h"\
	"..\..\QTDevWin\CIncludes\StandardFile.h"\
	"..\..\QTDevWin\CIncludes\TextCommon.h"\
	"..\..\QTDevWin\CIncludes\TextEdit.h"\
	"..\..\QTDevWin\ComponentIncludes\ComponentDispatchHelper.c"\
	"..\..\QTDevWin\ComponentIncludes\Components.k.h"\
	"..\..\QTDevWin\ComponentIncludes\Sound.k.h"\
	".\uLawCodec.h"\
	".\ulawcodecdispatch.h"\
	
CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /X /I "..\..\QTDevWin\CIncludes" /I\
 "..\..\QTDevWin\ComponentIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\uLawCompressor.obj" : $(SOURCE) $(DEP_CPP_ULAWC) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "uLawCodec. - Win32 Debug"

DEP_CPP_ULAWC=\
	"..\..\QTDevWin\CIncludes\AEDataModel.h"\
	"..\..\QTDevWin\CIncludes\Aliases.h"\
	"..\..\QTDevWin\CIncludes\AppleEvents.h"\
	"..\..\QTDevWin\CIncludes\AppleTalk.h"\
	"..\..\QTDevWin\CIncludes\CodeFragments.h"\
	"..\..\QTDevWin\CIncludes\Collections.h"\
	"..\..\QTDevWin\CIncludes\Components.h"\
	"..\..\QTDevWin\CIncludes\ConditionalMacros.h"\
	"..\..\QTDevWin\CIncludes\Controls.h"\
	"..\..\QTDevWin\CIncludes\DateTimeUtils.h"\
	"..\..\QTDevWin\CIncludes\Dialogs.h"\
	"..\..\QTDevWin\CIncludes\Drag.h"\
	"..\..\QTDevWin\CIncludes\Endian.h"\
	"..\..\QTDevWin\CIncludes\Errors.h"\
	"..\..\QTDevWin\CIncludes\Events.h"\
	"..\..\QTDevWin\CIncludes\Files.h"\
	"..\..\QTDevWin\CIncludes\Finder.h"\
	"..\..\QTDevWin\CIncludes\Icons.h"\
	"..\..\QTDevWin\CIncludes\ImageCompression.h"\
	"..\..\QTDevWin\CIncludes\MacMemory.h"\
	"..\..\QTDevWin\CIncludes\MacTypes.h"\
	"..\..\QTDevWin\CIncludes\MacWindows.h"\
	"..\..\QTDevWin\CIncludes\Menus.h"\
	"..\..\QTDevWin\CIncludes\MixedMode.h"\
	"..\..\QTDevWin\CIncludes\Movies.h"\
	"..\..\QTDevWin\CIncludes\MoviesFormat.h"\
	"..\..\QTDevWin\CIncludes\Notification.h"\
	"..\..\QTDevWin\CIncludes\OSUtils.h"\
	"..\..\QTDevWin\CIncludes\Patches.h"\
	"..\..\QTDevWin\CIncludes\Processes.h"\
	"..\..\QTDevWin\CIncludes\QDOffscreen.h"\
	"..\..\QTDevWin\CIncludes\Quickdraw.h"\
	"..\..\QTDevWin\CIncludes\QuickdrawText.h"\
	"..\..\QTDevWin\CIncludes\Resources.h"\
	"..\..\QTDevWin\CIncludes\Sound.h"\
	"..\..\QTDevWin\CIncludes\StandardFile.h"\
	"..\..\QTDevWin\CIncludes\TextCommon.h"\
	"..\..\QTDevWin\CIncludes\TextEdit.h"\
	"..\..\QTDevWin\ComponentIncludes\ComponentDispatchHelper.c"\
	"..\..\QTDevWin\ComponentIncludes\Components.k.h"\
	"..\..\QTDevWin\ComponentIncludes\Sound.k.h"\
	".\uLawCodec.h"\
	".\ulawcodecdispatch.h"\
	
CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /Zi /Od /X /I "..\..\QTDevWin\CIncludes"\
 /I "..\..\QTDevWin\ComponentIncludes" /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\uLawCompressor.obj" : $(SOURCE) $(DEP_CPP_ULAWC) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\uLawDecompressor.c

!IF  "$(CFG)" == "uLawCodec. - Win32 Release"

DEP_CPP_ULAWD=\
	"..\..\QTDevWin\CIncludes\AEDataModel.h"\
	"..\..\QTDevWin\CIncludes\Aliases.h"\
	"..\..\QTDevWin\CIncludes\AppleEvents.h"\
	"..\..\QTDevWin\CIncludes\AppleTalk.h"\
	"..\..\QTDevWin\CIncludes\CodeFragments.h"\
	"..\..\QTDevWin\CIncludes\Collections.h"\
	"..\..\QTDevWin\CIncludes\Components.h"\
	"..\..\QTDevWin\CIncludes\ConditionalMacros.h"\
	"..\..\QTDevWin\CIncludes\Controls.h"\
	"..\..\QTDevWin\CIncludes\DateTimeUtils.h"\
	"..\..\QTDevWin\CIncludes\Dialogs.h"\
	"..\..\QTDevWin\CIncludes\Drag.h"\
	"..\..\QTDevWin\CIncludes\Endian.h"\
	"..\..\QTDevWin\CIncludes\Errors.h"\
	"..\..\QTDevWin\CIncludes\Events.h"\
	"..\..\QTDevWin\CIncludes\Files.h"\
	"..\..\QTDevWin\CIncludes\Finder.h"\
	"..\..\QTDevWin\CIncludes\Icons.h"\
	"..\..\QTDevWin\CIncludes\ImageCompression.h"\
	"..\..\QTDevWin\CIncludes\MacMemory.h"\
	"..\..\QTDevWin\CIncludes\MacTypes.h"\
	"..\..\QTDevWin\CIncludes\MacWindows.h"\
	"..\..\QTDevWin\CIncludes\Menus.h"\
	"..\..\QTDevWin\CIncludes\MixedMode.h"\
	"..\..\QTDevWin\CIncludes\Movies.h"\
	"..\..\QTDevWin\CIncludes\MoviesFormat.h"\
	"..\..\QTDevWin\CIncludes\Notification.h"\
	"..\..\QTDevWin\CIncludes\OSUtils.h"\
	"..\..\QTDevWin\CIncludes\Patches.h"\
	"..\..\QTDevWin\CIncludes\Processes.h"\
	"..\..\QTDevWin\CIncludes\QDOffscreen.h"\
	"..\..\QTDevWin\CIncludes\QTML.h"\
	"..\..\QTDevWin\CIncludes\Quickdraw.h"\
	"..\..\QTDevWin\CIncludes\QuickdrawText.h"\
	"..\..\QTDevWin\CIncludes\Resources.h"\
	"..\..\QTDevWin\CIncludes\Sound.h"\
	"..\..\QTDevWin\CIncludes\StandardFile.h"\
	"..\..\QTDevWin\CIncludes\TextCommon.h"\
	"..\..\QTDevWin\CIncludes\TextEdit.h"\
	"..\..\QTDevWin\ComponentIncludes\ComponentDispatchHelper.c"\
	"..\..\QTDevWin\ComponentIncludes\Components.k.h"\
	"..\..\QTDevWin\ComponentIncludes\Sound.k.h"\
	".\uLawCodec.h"\
	".\ulawcodecdispatch.h"\
	
CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /X /I "..\..\QTDevWin\CIncludes" /I\
 "..\..\QTDevWin\ComponentIncludes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\uLawDecompressor.obj" : $(SOURCE) $(DEP_CPP_ULAWD) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "uLawCodec. - Win32 Debug"

DEP_CPP_ULAWD=\
	"..\..\QTDevWin\CIncludes\AEDataModel.h"\
	"..\..\QTDevWin\CIncludes\Aliases.h"\
	"..\..\QTDevWin\CIncludes\AppleEvents.h"\
	"..\..\QTDevWin\CIncludes\AppleTalk.h"\
	"..\..\QTDevWin\CIncludes\CodeFragments.h"\
	"..\..\QTDevWin\CIncludes\Collections.h"\
	"..\..\QTDevWin\CIncludes\Components.h"\
	"..\..\QTDevWin\CIncludes\ConditionalMacros.h"\
	"..\..\QTDevWin\CIncludes\Controls.h"\
	"..\..\QTDevWin\CIncludes\DateTimeUtils.h"\
	"..\..\QTDevWin\CIncludes\Dialogs.h"\
	"..\..\QTDevWin\CIncludes\Drag.h"\
	"..\..\QTDevWin\CIncludes\Endian.h"\
	"..\..\QTDevWin\CIncludes\Errors.h"\
	"..\..\QTDevWin\CIncludes\Events.h"\
	"..\..\QTDevWin\CIncludes\Files.h"\
	"..\..\QTDevWin\CIncludes\Finder.h"\
	"..\..\QTDevWin\CIncludes\Icons.h"\
	"..\..\QTDevWin\CIncludes\ImageCompression.h"\
	"..\..\QTDevWin\CIncludes\MacMemory.h"\
	"..\..\QTDevWin\CIncludes\MacTypes.h"\
	"..\..\QTDevWin\CIncludes\MacWindows.h"\
	"..\..\QTDevWin\CIncludes\Menus.h"\
	"..\..\QTDevWin\CIncludes\MixedMode.h"\
	"..\..\QTDevWin\CIncludes\Movies.h"\
	"..\..\QTDevWin\CIncludes\MoviesFormat.h"\
	"..\..\QTDevWin\CIncludes\Notification.h"\
	"..\..\QTDevWin\CIncludes\OSUtils.h"\
	"..\..\QTDevWin\CIncludes\Patches.h"\
	"..\..\QTDevWin\CIncludes\Processes.h"\
	"..\..\QTDevWin\CIncludes\QDOffscreen.h"\
	"..\..\QTDevWin\CIncludes\QTML.h"\
	"..\..\QTDevWin\CIncludes\Quickdraw.h"\
	"..\..\QTDevWin\CIncludes\QuickdrawText.h"\
	"..\..\QTDevWin\CIncludes\Resources.h"\
	"..\..\QTDevWin\CIncludes\Sound.h"\
	"..\..\QTDevWin\CIncludes\StandardFile.h"\
	"..\..\QTDevWin\CIncludes\TextCommon.h"\
	"..\..\QTDevWin\CIncludes\TextEdit.h"\
	"..\..\QTDevWin\ComponentIncludes\ComponentDispatchHelper.c"\
	"..\..\QTDevWin\ComponentIncludes\Components.k.h"\
	"..\..\QTDevWin\ComponentIncludes\Sound.k.h"\
	".\uLawCodec.h"\
	".\ulawcodecdispatch.h"\
	
CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /Zi /Od /X /I "..\..\QTDevWin\CIncludes"\
 /I "..\..\QTDevWin\ComponentIncludes" /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /Fp"$(INTDIR)\uLawCodec.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\uLawDecompressor.obj" : $(SOURCE) $(DEP_CPP_ULAWD) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

