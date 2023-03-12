# Microsoft Developer Studio Generated NMAKE File, Based on VRScript.dsp
!IF "$(CFG)" == ""
CFG=VRScript - Win32 Release
!MESSAGE No configuration specified. Defaulting to VRScript - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "VRScript - Win32 Release" && "$(CFG)" !=\
 "VRScript - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VRScript.mak" CFG="VRScript - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VRScript - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "VRScript - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "VRScript - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\VRScript.exe"

!ELSE 

ALL : "$(OUTDIR)\VRScript.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\FileUtilities.obj"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\QTVRUtilities.obj"
	-@erase "$(INTDIR)\URLUtilities.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\VR3DObjects.obj"
	-@erase "$(INTDIR)\VR3DTexture.obj"
	-@erase "$(INTDIR)\VRActions.obj"
	-@erase "$(INTDIR)\VREffects.obj"
	-@erase "$(INTDIR)\VRHash.obj"
	-@erase "$(INTDIR)\VRMovies.obj"
	-@erase "$(INTDIR)\VRPicture.obj"
	-@erase "$(INTDIR)\VRPreferences.obj"
	-@erase "$(INTDIR)\VRScript.obj"
	-@erase "$(INTDIR)\VRScript.res"
	-@erase "$(INTDIR)\VRSound.obj"
	-@erase "$(INTDIR)\VRSprites.obj"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\VRScript.exe"
	-@erase "$(OUTDIR)\VRScript.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /Od /I "..\..\QTDevWin\CIncludes" /I "." /I\
 ".\Feature Files" /I ".\Application Files" /I ".\Common Files" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "QD3D_AVAIL" /D "_EXCEPTION_DEFINED" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VRScript.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VRScript.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\QTDevWin\Libraries\qtmlclient.lib\
 ..\..\QTDevWin\Libraries\qd3d.lib ..\..\QTDevWin\Libraries\qtvr.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\VRScript.pdb" /machine:I386\
 /out:"$(OUTDIR)\VRScript.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\FileUtilities.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\QTVRUtilities.obj" \
	"$(INTDIR)\URLUtilities.obj" \
	"$(INTDIR)\VR3DObjects.obj" \
	"$(INTDIR)\VR3DTexture.obj" \
	"$(INTDIR)\VRActions.obj" \
	"$(INTDIR)\VREffects.obj" \
	"$(INTDIR)\VRHash.obj" \
	"$(INTDIR)\VRMovies.obj" \
	"$(INTDIR)\VRPicture.obj" \
	"$(INTDIR)\VRPreferences.obj" \
	"$(INTDIR)\VRScript.obj" \
	"$(INTDIR)\VRScript.res" \
	"$(INTDIR)\VRSound.obj" \
	"$(INTDIR)\VRSprites.obj" \
	"$(INTDIR)\WinFramework.obj"

"$(OUTDIR)\VRScript.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\VRScript.exe"

!ELSE 

ALL : "$(OUTDIR)\VRScript.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ComApplication.obj"
	-@erase "$(INTDIR)\ComFramework.obj"
	-@erase "$(INTDIR)\FileUtilities.obj"
	-@erase "$(INTDIR)\QTUtilities.obj"
	-@erase "$(INTDIR)\QTVRUtilities.obj"
	-@erase "$(INTDIR)\URLUtilities.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\VR3DObjects.obj"
	-@erase "$(INTDIR)\VR3DTexture.obj"
	-@erase "$(INTDIR)\VRActions.obj"
	-@erase "$(INTDIR)\VREffects.obj"
	-@erase "$(INTDIR)\VRHash.obj"
	-@erase "$(INTDIR)\VRMovies.obj"
	-@erase "$(INTDIR)\VRPicture.obj"
	-@erase "$(INTDIR)\VRPreferences.obj"
	-@erase "$(INTDIR)\VRScript.obj"
	-@erase "$(INTDIR)\VRScript.res"
	-@erase "$(INTDIR)\VRSound.obj"
	-@erase "$(INTDIR)\VRSprites.obj"
	-@erase "$(INTDIR)\WinFramework.obj"
	-@erase "$(OUTDIR)\VRScript.exe"
	-@erase "$(OUTDIR)\VRScript.ilk"
	-@erase "$(OUTDIR)\VRScript.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /Gm /GX /Zi /Od /I "..\..\QTDevWin\CIncludes" /I "."\
 /I ".\Feature Files" /I ".\Application Files" /I ".\Common Files" /D "_DEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "QD3D_AVAIL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\VRScript.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\VRScript.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\QTDevWin\Libraries\qtmlclient.lib\
 ..\..\QTDevWin\Libraries\qd3d.lib ..\..\QTDevWin\Libraries\qtvr.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib\
 /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\VRScript.pdb"\
 /debug /machine:I386 /out:"$(OUTDIR)\VRScript.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ComApplication.obj" \
	"$(INTDIR)\ComFramework.obj" \
	"$(INTDIR)\FileUtilities.obj" \
	"$(INTDIR)\QTUtilities.obj" \
	"$(INTDIR)\QTVRUtilities.obj" \
	"$(INTDIR)\URLUtilities.obj" \
	"$(INTDIR)\VR3DObjects.obj" \
	"$(INTDIR)\VR3DTexture.obj" \
	"$(INTDIR)\VRActions.obj" \
	"$(INTDIR)\VREffects.obj" \
	"$(INTDIR)\VRHash.obj" \
	"$(INTDIR)\VRMovies.obj" \
	"$(INTDIR)\VRPicture.obj" \
	"$(INTDIR)\VRPreferences.obj" \
	"$(INTDIR)\VRScript.obj" \
	"$(INTDIR)\VRScript.res" \
	"$(INTDIR)\VRSound.obj" \
	"$(INTDIR)\VRSprites.obj" \
	"$(INTDIR)\WinFramework.obj"

"$(OUTDIR)\VRScript.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "VRScript - Win32 Release" || "$(CFG)" ==\
 "VRScript - Win32 Debug"
SOURCE=".\Application Files\ComApplication.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_COMAP=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_COMAP=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\ComApplication.obj" : $(SOURCE) $(DEP_CPP_COMAP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_COMAP=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\ComApplication.obj" : $(SOURCE) $(DEP_CPP_COMAP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\ComFramework.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_COMFR=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\QTUtilities.h"\
	".\common files\winprefix.h"\
	

"$(INTDIR)\ComFramework.obj" : $(SOURCE) $(DEP_CPP_COMFR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_COMFR=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\QTUtilities.h"\
	".\common files\winprefix.h"\
	

"$(INTDIR)\ComFramework.obj" : $(SOURCE) $(DEP_CPP_COMFR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\FileUtilities.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_FILEU=\
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
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	".\Common Files\FileUtilities.h"\
	".\common files\urlutilities.h"\
	

"$(INTDIR)\FileUtilities.obj" : $(SOURCE) $(DEP_CPP_FILEU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_FILEU=\
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
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	".\Common Files\FileUtilities.h"\
	".\common files\urlutilities.h"\
	

"$(INTDIR)\FileUtilities.obj" : $(SOURCE) $(DEP_CPP_FILEU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\QTUtilities.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_QTUTI=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Common Files\QTUtilities.h"\
	
NODEP_CPP_QTUTI=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) $(DEP_CPP_QTUTI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_QTUTI=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Common Files\QTUtilities.h"\
	

"$(INTDIR)\QTUtilities.obj" : $(SOURCE) $(DEP_CPP_QTUTI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\QTVRUtilities.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_QTVRU=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\winprefix.h"\
	
NODEP_CPP_QTVRU=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\QTVRUtilities.obj" : $(SOURCE) $(DEP_CPP_QTVRU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_QTVRU=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\winprefix.h"\
	

"$(INTDIR)\QTVRUtilities.obj" : $(SOURCE) $(DEP_CPP_QTVRU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\URLUtilities.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_URLUT=\
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
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	".\common files\urlutilities.h"\
	
NODEP_CPP_URLUT=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\URLUtilities.obj" : $(SOURCE) $(DEP_CPP_URLUT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_URLUT=\
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
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	".\common files\urlutilities.h"\
	

"$(INTDIR)\URLUtilities.obj" : $(SOURCE) $(DEP_CPP_URLUT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VR3DObjects.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VR3DO=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VR3DO=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VR3DObjects.obj" : $(SOURCE) $(DEP_CPP_VR3DO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VR3DO=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VR3DObjects.obj" : $(SOURCE) $(DEP_CPP_VR3DO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VR3DTexture.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VR3DT=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VR3DT=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VR3DTexture.obj" : $(SOURCE) $(DEP_CPP_VR3DT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VR3DT=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VR3DTexture.obj" : $(SOURCE) $(DEP_CPP_VR3DT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRActions.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRACT=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\QTDevWin\CIncludes\Folders.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRACT=\
	".\Application Files\MacFramework.h"\
	".\feature files\MacFramework.h"\
	".\MacFramework.h"\
	

"$(INTDIR)\VRActions.obj" : $(SOURCE) $(DEP_CPP_VRACT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRACT=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRActions.obj" : $(SOURCE) $(DEP_CPP_VRACT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VREffects.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VREFF=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VREFF=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VREffects.obj" : $(SOURCE) $(DEP_CPP_VREFF) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VREFF=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VREffects.obj" : $(SOURCE) $(DEP_CPP_VREFF) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRHash.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRHAS=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRHAS=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRHash.obj" : $(SOURCE) $(DEP_CPP_VRHAS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRHAS=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRHash.obj" : $(SOURCE) $(DEP_CPP_VRHAS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRMovies.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRMOV=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRMOV=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRMovies.obj" : $(SOURCE) $(DEP_CPP_VRMOV) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRMOV=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRMovies.obj" : $(SOURCE) $(DEP_CPP_VRMOV) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRPicture.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRPIC=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRPicture.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRPIC=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRPicture.obj" : $(SOURCE) $(DEP_CPP_VRPIC) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRPIC=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRPicture.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRPicture.obj" : $(SOURCE) $(DEP_CPP_VRPIC) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRPreferences.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRPRE=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\QTDevWin\CIncludes\Folders.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRPRE=\
	".\Application Files\MacFramework.h"\
	".\feature files\MacFramework.h"\
	".\MacFramework.h"\
	

"$(INTDIR)\VRPreferences.obj" : $(SOURCE) $(DEP_CPP_VRPRE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRPRE=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRPreferences.obj" : $(SOURCE) $(DEP_CPP_VRPRE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\VRScript.c

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRSCR=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRPicture.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRSCR=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRScript.obj" : $(SOURCE) $(DEP_CPP_VRSCR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRSCR=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRPicture.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRScript.obj" : $(SOURCE) $(DEP_CPP_VRSCR) "$(INTDIR)"


!ENDIF 

SOURCE=".\Application Files\VRScript.rc"
DEP_RSC_VRSCRI=\
	".\Application Files\ComResource.h"\
	".\Application Files\Movie.ico"\
	".\Application Files\VRScript.ico"\
	

!IF  "$(CFG)" == "VRScript - Win32 Release"


"$(INTDIR)\VRScript.res" : $(SOURCE) $(DEP_RSC_VRSCRI) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\VRScript.res" /i "Application Files" /i\
 ".\Application Files" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"


"$(INTDIR)\VRScript.res" : $(SOURCE) $(DEP_RSC_VRSCRI) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\VRScript.res" /i "Application Files" /i\
 ".\Application Files" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRSound.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRSOU=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRSOU=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRSound.obj" : $(SOURCE) $(DEP_CPP_VRSOU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRSOU=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRSound.obj" : $(SOURCE) $(DEP_CPP_VRSOU) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Feature Files\VRSprites.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_VRSPR=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundcomponents.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	
NODEP_CPP_VRSPR=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\VRSprites.obj" : $(SOURCE) $(DEP_CPP_VRSPR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_VRSPR=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\gxmath.h"\
	"..\..\qtdevwin\cincludes\gxtypes.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecodec.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qd3d.h"\
	"..\..\qtdevwin\cincludes\qd3dacceleration.h"\
	"..\..\qtdevwin\cincludes\qd3dcamera.h"\
	"..\..\qtdevwin\cincludes\qd3ddrawcontext.h"\
	"..\..\qtdevwin\cincludes\qd3derrors.h"\
	"..\..\qtdevwin\cincludes\qd3dgeometry.h"\
	"..\..\qtdevwin\cincludes\qd3dgroup.h"\
	"..\..\qtdevwin\cincludes\qd3dio.h"\
	"..\..\qtdevwin\cincludes\qd3dlight.h"\
	"..\..\qtdevwin\cincludes\qd3dmath.h"\
	"..\..\qtdevwin\cincludes\qd3drenderer.h"\
	"..\..\qtdevwin\cincludes\qd3dset.h"\
	"..\..\qtdevwin\cincludes\qd3dshader.h"\
	"..\..\qtdevwin\cincludes\qd3dstorage.h"\
	"..\..\qtdevwin\cincludes\qd3dstyle.h"\
	"..\..\qtdevwin\cincludes\qd3dtransform.h"\
	"..\..\qtdevwin\cincludes\qd3dview.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\quicktimevrformat.h"\
	"..\..\qtdevwin\cincludes\rave.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\soundsprocket.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComApplication.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\FileUtilities.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\QTVRUtilities.h"\
	".\common files\urlutilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	".\Feature Files\VR3DObjects.h"\
	".\Feature Files\VR3DTexture.h"\
	".\feature files\vractions.h"\
	".\Feature Files\VREffects.h"\
	".\feature files\vrhash.h"\
	".\Feature Files\VRMovies.h"\
	".\feature files\vrpreferences.h"\
	".\Feature Files\VRSound.h"\
	".\Feature Files\VRSprites.h"\
	".\VRScript.h"\
	

"$(INTDIR)\VRSprites.obj" : $(SOURCE) $(DEP_CPP_VRSPR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=".\Common Files\WinFramework.c"

!IF  "$(CFG)" == "VRScript - Win32 Release"

DEP_CPP_WINFR=\
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
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	
NODEP_CPP_WINFR=\
	"..\..\qtdevwin\cincludes\errors.h"\
	

"$(INTDIR)\WinFramework.obj" : $(SOURCE) $(DEP_CPP_WINFR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

DEP_CPP_WINFR=\
	"..\..\qtdevwin\cincludes\aedatamodel.h"\
	"..\..\qtdevwin\cincludes\aliases.h"\
	"..\..\qtdevwin\cincludes\appearance.h"\
	"..\..\qtdevwin\cincludes\appleevents.h"\
	"..\..\qtdevwin\cincludes\appletalk.h"\
	"..\..\qtdevwin\cincludes\codefragments.h"\
	"..\..\qtdevwin\cincludes\collections.h"\
	"..\..\qtdevwin\cincludes\components.h"\
	"..\..\qtdevwin\cincludes\conditionalmacros.h"\
	"..\..\qtdevwin\cincludes\controldefinitions.h"\
	"..\..\qtdevwin\cincludes\controls.h"\
	"..\..\qtdevwin\cincludes\datetimeutils.h"\
	"..\..\qtdevwin\cincludes\dialogs.h"\
	"..\..\qtdevwin\cincludes\drag.h"\
	"..\..\qtdevwin\cincludes\endian.h"\
	"..\..\qtdevwin\cincludes\events.h"\
	"..\..\qtdevwin\cincludes\files.h"\
	"..\..\qtdevwin\cincludes\filetypesandcreators.h"\
	"..\..\qtdevwin\cincludes\finder.h"\
	"..\..\qtdevwin\cincludes\fixmath.h"\
	"..\..\qtdevwin\cincludes\fonts.h"\
	"..\..\qtdevwin\cincludes\gestalt.h"\
	"..\..\qtdevwin\cincludes\icons.h"\
	"..\..\qtdevwin\cincludes\imagecompression.h"\
	"..\..\qtdevwin\cincludes\intlresources.h"\
	"..\..\qtdevwin\cincludes\lists.h"\
	"..\..\qtdevwin\cincludes\macerrors.h"\
	"..\..\qtdevwin\cincludes\machelp.h"\
	"..\..\qtdevwin\cincludes\macmemory.h"\
	"..\..\qtdevwin\cincludes\mactypes.h"\
	"..\..\qtdevwin\cincludes\macwindows.h"\
	"..\..\qtdevwin\cincludes\mediahandlers.h"\
	"..\..\qtdevwin\cincludes\menus.h"\
	"..\..\qtdevwin\cincludes\mixedmode.h"\
	"..\..\qtdevwin\cincludes\movies.h"\
	"..\..\qtdevwin\cincludes\moviesformat.h"\
	"..\..\qtdevwin\cincludes\navigation.h"\
	"..\..\qtdevwin\cincludes\notification.h"\
	"..\..\qtdevwin\cincludes\numberformatting.h"\
	"..\..\qtdevwin\cincludes\osutils.h"\
	"..\..\qtdevwin\cincludes\patches.h"\
	"..\..\qtdevwin\cincludes\printing.h"\
	"..\..\qtdevwin\cincludes\processes.h"\
	"..\..\qtdevwin\cincludes\qdoffscreen.h"\
	"..\..\qtdevwin\cincludes\qtml.h"\
	"..\..\qtdevwin\cincludes\qtsmovie.h"\
	"..\..\qtdevwin\cincludes\quickdraw.h"\
	"..\..\qtdevwin\cincludes\quickdrawtext.h"\
	"..\..\qtdevwin\cincludes\quicktimecomponents.h"\
	"..\..\qtdevwin\cincludes\quicktimemusic.h"\
	"..\..\qtdevwin\cincludes\quicktimestreaming.h"\
	"..\..\qtdevwin\cincludes\quicktimevr.h"\
	"..\..\qtdevwin\cincludes\resources.h"\
	"..\..\qtdevwin\cincludes\scrap.h"\
	"..\..\qtdevwin\cincludes\script.h"\
	"..\..\qtdevwin\cincludes\sound.h"\
	"..\..\qtdevwin\cincludes\standardfile.h"\
	"..\..\qtdevwin\cincludes\stringcompare.h"\
	"..\..\qtdevwin\cincludes\textcommon.h"\
	"..\..\qtdevwin\cincludes\textedit.h"\
	"..\..\qtdevwin\cincludes\textutils.h"\
	"..\..\qtdevwin\cincludes\translation.h"\
	"..\..\qtdevwin\cincludes\translationextensions.h"\
	"..\..\qtdevwin\cincludes\traps.h"\
	"..\..\qtdevwin\cincludes\utcutils.h"\
	"..\..\qtdevwin\cincludes\video.h"\
	".\Application Files\ComResource.h"\
	".\common files\comframework.h"\
	".\Common Files\QTUtilities.h"\
	".\Common Files\WinFramework.h"\
	".\common files\winprefix.h"\
	

"$(INTDIR)\WinFramework.obj" : $(SOURCE) $(DEP_CPP_WINFR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

