# Microsoft Developer Studio Generated NMAKE File, Based on SimplePlayerMFC.dsp
!IF "$(CFG)" == ""
CFG=SimplePlayerMFC - Win32 Release
!MESSAGE No configuration specified. Defaulting to SimplePlayerMFC - Win32\
 Release.
!ENDIF 

!IF "$(CFG)" != "SimplePlayerMFC - Win32 Release" && "$(CFG)" !=\
 "SimplePlayerMFC - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SimplePlayerMFC.mak" CFG="SimplePlayerMFC - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimplePlayerMFC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SimplePlayerMFC - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : ".\SimplePlayerMFC.exe"

!ELSE 

ALL : ".\SimplePlayerMFC.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SimplePlayerMFC.obj"
	-@erase "$(INTDIR)\SimplePlayerMFC.pch"
	-@erase "$(INTDIR)\SimplePlayerMFC.res"
	-@erase "$(INTDIR)\SimplePlayerMFCDoc.obj"
	-@erase "$(INTDIR)\SimplePlayerMFCView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase ".\SimplePlayerMFC.exe"
	-@erase ".\SimplePlayerMFC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\SimplePlayerMFC.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SimplePlayerMFC.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimplePlayerMFC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SimplePlayerMFC.pdb"\
 /machine:I386 /nodefaultlib:"LIBCD" /out:".\SimplePlayerMFC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SimplePlayerMFC.obj" \
	"$(INTDIR)\SimplePlayerMFC.res" \
	"$(INTDIR)\SimplePlayerMFCDoc.obj" \
	"$(INTDIR)\SimplePlayerMFCView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\SimplePlayerMFC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : ".\SimplePlayerMFC.exe"

!ELSE 

ALL : ".\SimplePlayerMFC.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SimplePlayerMFC.obj"
	-@erase "$(INTDIR)\SimplePlayerMFC.pch"
	-@erase "$(INTDIR)\SimplePlayerMFC.res"
	-@erase "$(INTDIR)\SimplePlayerMFCDoc.obj"
	-@erase "$(INTDIR)\SimplePlayerMFCView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\SimplePlayerMFC.pdb"
	-@erase ".\SimplePlayerMFC.exe"
	-@erase ".\SimplePlayerMFC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\SimplePlayerMFC.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SimplePlayerMFC.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimplePlayerMFC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SimplePlayerMFC.pdb" /debug\
 /machine:I386 /nodefaultlib:"LIBCD" /out:".\SimplePlayerMFC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SimplePlayerMFC.obj" \
	"$(INTDIR)\SimplePlayerMFC.res" \
	"$(INTDIR)\SimplePlayerMFCDoc.obj" \
	"$(INTDIR)\SimplePlayerMFCView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\SimplePlayerMFC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "SimplePlayerMFC - Win32 Release" || "$(CFG)" ==\
 "SimplePlayerMFC - Win32 Debug"
SOURCE=..\QTClasses\CQuickTime.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_CQUIC=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	
CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\CQuickTime.obj" : $(SOURCE) $(DEP_CPP_CQUIC) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_CQUIC=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	
CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\QTDevWin\CIncludes"\
 /I "..\qtclasses" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /D pascal= /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\CQuickTime.obj" : $(SOURCE) $(DEP_CPP_CQUIC) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_MAINF=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\MainFrm.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_MAINF=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\MainFrm.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ENDIF 

SOURCE=.\SimplePlayerMFC.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_SIMPL=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\MainFrm.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFC.obj" : $(SOURCE) $(DEP_CPP_SIMPL) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_SIMPL=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\MainFrm.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFC.obj" : $(SOURCE) $(DEP_CPP_SIMPL) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ENDIF 

SOURCE=.\SimplePlayerMFC.rc
DEP_RSC_SIMPLE=\
	".\res\SimplePlayerMFC.ico"\
	".\res\SimplePlayerMFC.rc2"\
	".\res\SimplePlayerMFCDoc.ico"\
	

"$(INTDIR)\SimplePlayerMFC.res" : $(SOURCE) $(DEP_RSC_SIMPLE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\SimplePlayerMFCDoc.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_SIMPLEP=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFCDoc.obj" : $(SOURCE) $(DEP_CPP_SIMPLEP) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_SIMPLEP=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFCDoc.obj" : $(SOURCE) $(DEP_CPP_SIMPLEP) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ENDIF 

SOURCE=.\SimplePlayerMFCView.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_SIMPLEPL=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFCView.obj" : $(SOURCE) $(DEP_CPP_SIMPLEPL) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_SIMPLEPL=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	"..\qtclasses\CQuickTime.h"\
	".\SimplePlayerMFC.h"\
	".\SimplePlayerMFCDoc.h"\
	".\SimplePlayerMFCView.h"\
	

"$(INTDIR)\SimplePlayerMFCView.obj" : $(SOURCE) $(DEP_CPP_SIMPLEPL) "$(INTDIR)"\
 "$(INTDIR)\SimplePlayerMFC.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "SimplePlayerMFC - Win32 Release"

DEP_CPP_STDAF=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	".\StdAfx.h"\
	
CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\SimplePlayerMFC.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SimplePlayerMFC.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SimplePlayerMFC - Win32 Debug"

DEP_CPP_STDAF=\
	"..\..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\..\qtdevwin\cincludes\collections.h"\
	"..\..\..\qtdevwin\cincludes\components.h"\
	"..\..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\..\qtdevwin\cincludes\controls.h"\
	"..\..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\..\qtdevwin\cincludes\drag.h"\
	"..\..\..\qtdevwin\cincludes\endian.h"\
	"..\..\..\qtdevwin\cincludes\events.h"\
	"..\..\..\qtdevwin\cincludes\files.h"\
	"..\..\..\qtdevwin\cincludes\finder.h"\
	"..\..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\..\qtdevwin\cincludes\icons.h"\
	"..\..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\..\qtdevwin\cincludes\menus.h"\
	"..\..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\..\qtdevwin\cincludes\movies.h"\
	"..\..\..\qtdevwin\cincludes\notification.h"\
	"..\..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\..\qtdevwin\cincludes\patches.h"\
	"..\..\..\qtdevwin\cincludes\processes.h"\
	"..\..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\..\qtdevwin\cincludes\utcutils.h"\
	".\StdAfx.h"\
	
CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\QTDevWin\CIncludes"\
 /I "..\qtclasses" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS"\
 /D pascal= /Fp"$(INTDIR)\SimplePlayerMFC.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SimplePlayerMFC.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

