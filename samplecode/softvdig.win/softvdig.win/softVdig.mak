# Microsoft Developer Studio Generated NMAKE File, Based on softVdig.dsp
!IF "$(CFG)" == ""
CFG=softVdig - Win32 Release
!MESSAGE No configuration specified. Defaulting to softVdig - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "softVdig - Win32 Release" && "$(CFG)" !=\
 "softVdig - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "softVdig.mak" CFG="softVdig - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "softVdig - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "softVdig - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "softVdig - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\.\Release
TargetName=SoftVdig
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\SoftVdig.qtx"

!ELSE 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\SoftVdig.qtx"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\softVdig.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\SoftVdig.exp"
	-@erase "$(OUTDIR)\SoftVdig.ilk"
	-@erase "$(OUTDIR)\SoftVdig.lib"
	-@erase "$(OUTDIR)\SoftVdig.qtx"
	-@erase "$(OutDir)\resource.frk\$(TargetName)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /D "WIN32" /D\
 "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\softVdig.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\softVdig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libc.lib ..\..\QTDevWin\Libraries\qtmlclient.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\SoftVdig.pdb"\
 /machine:I386 /nodefaultlib /def:".\softVdig.def" /out:"$(OUTDIR)\SoftVdig.qtx"\
 /implib:"$(OUTDIR)\SoftVdig.lib" 
DEF_FILE= \
	".\softVdig.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\softVdig.obj"

"$(OUTDIR)\SoftVdig.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "softVdig - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\.\Debug
TargetName=SoftVdig
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\SoftVdig.qtx"

!ELSE 

ALL : "$(OutDir)\resource.frk\$(TargetName)" "$(OUTDIR)\SoftVdig.qtx"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\softVdig.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\SoftVdig.exp"
	-@erase "$(OUTDIR)\SoftVdig.ilk"
	-@erase "$(OUTDIR)\SoftVdig.lib"
	-@erase "$(OUTDIR)\SoftVdig.pdb"
	-@erase "$(OUTDIR)\SoftVdig.qtx"
	-@erase "$(OutDir)\resource.frk\$(TargetName)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\QTDevWin\CIncludes" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\softVdig.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\softVdig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libcd.lib ..\..\QTDevWin\Libraries\qtmlclient.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\SoftVdig.pdb" /debug\
 /machine:I386 /nodefaultlib /def:".\softVdig.def" /out:"$(OUTDIR)\SoftVdig.qtx"\
 /implib:"$(OUTDIR)\SoftVdig.lib" 
DEF_FILE= \
	".\softVdig.def"
LINK32_OBJS= \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\softVdig.obj"

"$(OUTDIR)\SoftVdig.qtx" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "softVdig - Win32 Release" || "$(CFG)" ==\
 "softVdig - Win32 Debug"
SOURCE=.\dllmain.c

!IF  "$(CFG)" == "softVdig - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /I "..\..\QTDevWin\CIncludes" /D "WIN32"\
 /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\softVdig.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "softVdig - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\QTDevWin\CIncludes" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\softVdig.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\softVdig.c

!IF  "$(CFG)" == "softVdig - Win32 Release"

DEP_CPP_SOFTV=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\packages.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\toolutils.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\softVdig.h"\
	

"$(INTDIR)\softVdig.obj" : $(SOURCE) $(DEP_CPP_SOFTV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "softVdig - Win32 Debug"

DEP_CPP_SOFTV=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\packages.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\toolutils.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\softVdig.h"\
	

"$(INTDIR)\softVdig.obj" : $(SOURCE) $(DEP_CPP_SOFTV) "$(INTDIR)"


!ENDIF 

SOURCE=.\softVdig.r

!IF  "$(CFG)" == "softVdig - Win32 Release"

OutDir=.\.\Release
TargetPath=.\Release\SoftVdig.qtx
TargetName=SoftVdig
InputPath=.\softVdig.r

"$(OutDir)\resource.frk\$(TargetName)"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez -d WARHOL_NOASM=0 -d            WARHOL_DECKS=0 -d\
         WARHOL_MIDI=0 -d WARHOL_QTI=0 -i  ..\..\RInclude -i\
               ..\..\QTDevWin\RIncludes\      -o  $(TargetPath) $(InputPath)

!ELSEIF  "$(CFG)" == "softVdig - Win32 Debug"

OutDir=.\.\Debug
TargetPath=.\Debug\SoftVdig.qtx
TargetName=SoftVdig
InputPath=.\softVdig.r

"$(OutDir)\resource.frk\$(TargetName)"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez -d WARHOL_NOASM=0 -d            WARHOL_DECKS=0 -d\
         WARHOL_MIDI=0 -d WARHOL_QTI=0 -i  ..\..\RInclude -i\
               ..\..\QTDevWin\RIncludes\      -o  $(TargetPath) $(InputPath)

!ENDIF 


!ENDIF 

