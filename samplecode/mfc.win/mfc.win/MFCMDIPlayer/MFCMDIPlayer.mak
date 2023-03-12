# Microsoft Developer Studio Generated NMAKE File, Based on MFCMDIPlayer.dsp
!IF "$(CFG)" == ""
CFG=MFCMDIPlayer - Win32 Release
!MESSAGE No configuration specified. Defaulting to MFCMDIPlayer - Win32\
 Release.
!ENDIF 

!IF "$(CFG)" != "MFCMDIPlayer - Win32 Release" && "$(CFG)" !=\
 "MFCMDIPlayer - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MFCMDIPlayer.mak" CFG="MFCMDIPlayer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MFCMDIPlayer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MFCMDIPlayer - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : ".\MFCMDIPlayer.exe"

!ELSE 

ALL : ".\MFCMDIPlayer.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MFCMDIPlayer.obj"
	-@erase "$(INTDIR)\MFCMDIPlayer.pch"
	-@erase "$(INTDIR)\MFCMDIPlayer.res"
	-@erase "$(INTDIR)\MFCMDIPlayerDoc.obj"
	-@erase "$(INTDIR)\MFCMDIPlayerView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase ".\MFCMDIPlayer.exe"
	-@erase ".\MFCMDIPlayer.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\MFCMDIPlayer.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\MFCMDIPlayer.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MFCMDIPlayer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\MFCMDIPlayer.pdb"\
 /machine:I386 /out:".\MFCMDIPlayer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MFCMDIPlayer.obj" \
	"$(INTDIR)\MFCMDIPlayer.res" \
	"$(INTDIR)\MFCMDIPlayerDoc.obj" \
	"$(INTDIR)\MFCMDIPlayerView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\MFCMDIPlayer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : ".\MFCMDIPlayer.exe"

!ELSE 

ALL : ".\MFCMDIPlayer.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\CQuickTime.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MFCMDIPlayer.obj"
	-@erase "$(INTDIR)\MFCMDIPlayer.pch"
	-@erase "$(INTDIR)\MFCMDIPlayer.res"
	-@erase "$(INTDIR)\MFCMDIPlayerDoc.obj"
	-@erase "$(INTDIR)\MFCMDIPlayerView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\MFCMDIPlayer.pdb"
	-@erase ".\MFCMDIPlayer.exe"
	-@erase ".\MFCMDIPlayer.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\QTDevWin\CIncludes" /I\
 "..\qtclasses" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D\
 pascal= /Fp"$(INTDIR)\MFCMDIPlayer.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\MFCMDIPlayer.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MFCMDIPlayer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\QTDevWin\Libraries\qtmlclient.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\MFCMDIPlayer.pdb" /debug\
 /machine:I386 /out:".\MFCMDIPlayer.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\CQuickTime.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MFCMDIPlayer.obj" \
	"$(INTDIR)\MFCMDIPlayer.res" \
	"$(INTDIR)\MFCMDIPlayerDoc.obj" \
	"$(INTDIR)\MFCMDIPlayerView.obj" \
	"$(INTDIR)\StdAfx.obj"

".\MFCMDIPlayer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "MFCMDIPlayer - Win32 Release" || "$(CFG)" ==\
 "MFCMDIPlayer - Win32 Debug"
SOURCE=.\ChildFrm.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

DEP_CPP_CHILD=\
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
	".\ChildFrm.h"\
	".\MFCMDIPlayer.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

DEP_CPP_CHILD=\
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
	".\ChildFrm.h"\
	".\MFCMDIPlayer.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ENDIF 

SOURCE=..\QTClasses\CQuickTime.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

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


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

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

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

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
	".\MainFrm.h"\
	".\MFCMDIPlayer.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

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
	".\MainFrm.h"\
	".\MFCMDIPlayer.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ENDIF 

SOURCE=.\MFCMDIPlayer.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

DEP_CPP_MFCMD=\
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
	".\ChildFrm.h"\
	".\MainFrm.h"\
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayer.obj" : $(SOURCE) $(DEP_CPP_MFCMD) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

DEP_CPP_MFCMD=\
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
	".\ChildFrm.h"\
	".\MainFrm.h"\
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayer.obj" : $(SOURCE) $(DEP_CPP_MFCMD) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ENDIF 

SOURCE=.\MFCMDIPlayer.rc
DEP_RSC_MFCMDI=\
	".\res\MFCMDIPlayer.ico"\
	".\res\MFCMDIPlayer.rc2"\
	".\res\MFCMDIPlayerDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\MFCMDIPlayer.res" : $(SOURCE) $(DEP_RSC_MFCMDI) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\MFCMDIPlayerDoc.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

DEP_CPP_MFCMDIP=\
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
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayerDoc.obj" : $(SOURCE) $(DEP_CPP_MFCMDIP) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

DEP_CPP_MFCMDIP=\
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
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayerDoc.obj" : $(SOURCE) $(DEP_CPP_MFCMDIP) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ENDIF 

SOURCE=.\MFCMDIPlayerView.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

DEP_CPP_MFCMDIPL=\
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
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayerView.obj" : $(SOURCE) $(DEP_CPP_MFCMDIPL) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

DEP_CPP_MFCMDIPL=\
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
	".\MFCMDIPlayer.h"\
	".\MFCMDIPlayerDoc.h"\
	".\MFCMDIPlayerView.h"\
	

"$(INTDIR)\MFCMDIPlayerView.obj" : $(SOURCE) $(DEP_CPP_MFCMDIPL) "$(INTDIR)"\
 "$(INTDIR)\MFCMDIPlayer.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "MFCMDIPlayer - Win32 Release"

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
 pascal= /Fp"$(INTDIR)\MFCMDIPlayer.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\MFCMDIPlayer.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "MFCMDIPlayer - Win32 Debug"

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
 /D pascal= /Fp"$(INTDIR)\MFCMDIPlayer.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\MFCMDIPlayer.pch" : $(SOURCE)\
 $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

