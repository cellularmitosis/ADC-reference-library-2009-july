# Microsoft Developer Studio Generated NMAKE File, Based on SimpleEditMFC.dsp
!IF "$(CFG)" == ""
CFG=SimpleEditMFC - Win32 Release
!MESSAGE No configuration specified. Defaulting to SimpleEditMFC - Win32\
 Release.
!ENDIF 

!IF "$(CFG)" != "SimpleEditMFC - Win32 Release" && "$(CFG)" !=\
 "SimpleEditMFC - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SimpleEditMFC.mak" CFG="SimpleEditMFC - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimpleEditMFC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SimpleEditMFC - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : ".\SimpleEditMFC.exe"

!ELSE 

ALL : ".\SimpleEditMFC.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SimpleEditMFC.obj"
	-@erase "$(INTDIR)\SimpleEditMFC.pch"
	-@erase "$(INTDIR)\SimpleEditMFC.res"
	-@erase "$(INTDIR)\SimpleEditMFCDoc.obj"
	-@erase "$(INTDIR)\SimpleEditMFCView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase ".\SimpleEditMFC.exe"
	-@erase ".\SimpleEditMFC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\SimpleEditMFC.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SimpleEditMFC.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimpleEditMFC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SimpleEditMFC.pdb"\
 /machine:I386 /nodefaultlib:"LIBCD" /out:".\SimpleEditMFC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SimpleEditMFC.obj" \
	"$(INTDIR)\SimpleEditMFC.res" \
	"$(INTDIR)\SimpleEditMFCDoc.obj" \
	"$(INTDIR)\SimpleEditMFCView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\SimpleEditMFC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : ".\SimpleEditMFC.exe"

!ELSE 

ALL : ".\SimpleEditMFC.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SimpleEditMFC.obj"
	-@erase "$(INTDIR)\SimpleEditMFC.pch"
	-@erase "$(INTDIR)\SimpleEditMFC.res"
	-@erase "$(INTDIR)\SimpleEditMFCDoc.obj"
	-@erase "$(INTDIR)\SimpleEditMFCView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\SimpleEditMFC.pdb"
	-@erase ".\SimpleEditMFC.exe"
	-@erase ".\SimpleEditMFC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\SimpleEditMFC.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\SimpleEditMFC.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimpleEditMFC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SimpleEditMFC.pdb" /debug\
 /machine:I386 /nodefaultlib:"LIBCD" /out:".\SimpleEditMFC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SimpleEditMFC.obj" \
	"$(INTDIR)\SimpleEditMFC.res" \
	"$(INTDIR)\SimpleEditMFCDoc.obj" \
	"$(INTDIR)\SimpleEditMFCView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\SimpleEditMFC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "SimpleEditMFC - Win32 Release" || "$(CFG)" ==\
 "SimpleEditMFC - Win32 Debug"
SOURCE=..\QTClasses\CQuickTime.cpp

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

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


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

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

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ENDIF 

SOURCE=.\SimpleEditMFC.cpp

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFC.obj" : $(SOURCE) $(DEP_CPP_SIMPL) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFC.obj" : $(SOURCE) $(DEP_CPP_SIMPL) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ENDIF 

SOURCE=.\SimpleEditMFC.rc
DEP_RSC_SIMPLE=\
	".\res\SimpleEditMFC.ico"\
	".\res\SimpleEditMFC.rc2"\
	".\res\SimpleEditMFCDoc.ico"\
	

"$(INTDIR)\SimpleEditMFC.res" : $(SOURCE) $(DEP_RSC_SIMPLE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\SimpleEditMFCDoc.cpp

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

DEP_CPP_SIMPLEE=\
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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFCDoc.obj" : $(SOURCE) $(DEP_CPP_SIMPLEE) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

DEP_CPP_SIMPLEE=\
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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFCDoc.obj" : $(SOURCE) $(DEP_CPP_SIMPLEE) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ENDIF 

SOURCE=.\SimpleEditMFCView.cpp

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

DEP_CPP_SIMPLEED=\
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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFCView.obj" : $(SOURCE) $(DEP_CPP_SIMPLEED) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

DEP_CPP_SIMPLEED=\
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
	".\SimpleEditMFC.h"\
	".\SimpleEditMFCDoc.h"\
	".\SimpleEditMFCView.h"\
	

"$(INTDIR)\SimpleEditMFCView.obj" : $(SOURCE) $(DEP_CPP_SIMPLEED) "$(INTDIR)"\
 "$(INTDIR)\SimpleEditMFC.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "SimpleEditMFC - Win32 Release"

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
 pascal= /Fp"$(INTDIR)\SimpleEditMFC.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SimpleEditMFC.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SimpleEditMFC - Win32 Debug"

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
 /D pascal= /Fp"$(INTDIR)\SimpleEditMFC.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SimpleEditMFC.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

