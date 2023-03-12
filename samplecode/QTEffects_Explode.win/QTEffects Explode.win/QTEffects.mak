# Microsoft Developer Studio Generated NMAKE File, Based on QTEffects.dsp
!IF "$(CFG)" == ""
CFG=QTEffects - Win32 Debug
!MESSAGE No configuration specified. Defaulting to QTEffects - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "QTEffects - Win32 Release" && "$(CFG)" !=\
 "QTEffects - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QTEffects.mak" CFG="QTEffects - Win32 Debug"
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

!IF  "$(CFG)" == "QTEffects - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\QTEffects.exe"

!ELSE 

ALL : "$(OUTDIR)\QTEffects.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\QTEffects.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\QTEffects.exe"
	-@erase "$(OUTDIR)\QTEffects.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTEffects.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libc.lib qtmlclient.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)\QTEffects.pdb" /machine:I386 /nodefaultlib\
 /out:"$(OUTDIR)\QTEffects.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\QTEffects.obj"

"$(OUTDIR)\QTEffects.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=$(InputPath)
PostBuild_Desc=Adding Macintosh resources to our application file...
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTEffects.exe"
   MyRezWack_Release.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "QTEffects - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\QTEffects.exe" "$(OUTDIR)\QTEffects.bsc"

!ELSE 

ALL : "$(OUTDIR)\QTEffects.exe" "$(OUTDIR)\QTEffects.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\QTEffects.obj"
	-@erase "$(INTDIR)\QTEffects.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\QTEffects.bsc"
	-@erase "$(OUTDIR)\QTEffects.exe"
	-@erase "$(OUTDIR)\QTEffects.ilk"
	-@erase "$(OUTDIR)\QTEffects.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QTEffects.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\QTEffects.sbr"

"$(OUTDIR)\QTEffects.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib libcd.lib qtmlclient.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)\QTEffects.pdb" /debug /machine:I386\
 /nodefaultlib /out:"$(OUTDIR)\QTEffects.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\QTEffects.obj"

"$(OUTDIR)\QTEffects.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=$(InputPath)
PostBuild_Desc=Adding Macintosh resources to our application file...
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\QTEffects.exe" "$(OUTDIR)\QTEffects.bsc"
   MyRezWack_Debug.bat
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(CFG)" == "QTEffects - Win32 Release" || "$(CFG)" ==\
 "QTEffects - Win32 Debug"
SOURCE=.\main.c

!IF  "$(CFG)" == "QTEffects - Win32 Release"

DEP_CPP_MAIN_=\
	".\main.h"\
	".\qteffects.h"\
	{$(INCLUDE)}"aedatamodel.h"\
	{$(INCLUDE)}"aliases.h"\
	{$(INCLUDE)}"appleevents.h"\
	{$(INCLUDE)}"appletalk.h"\
	{$(INCLUDE)}"codefragments.h"\
	{$(INCLUDE)}"collections.h"\
	{$(INCLUDE)}"components.h"\
	{$(INCLUDE)}"conditionalmacros.h"\
	{$(INCLUDE)}"controls.h"\
	{$(INCLUDE)}"datetimeutils.h"\
	{$(INCLUDE)}"dialogs.h"\
	{$(INCLUDE)}"drag.h"\
	{$(INCLUDE)}"endian.h"\
	{$(INCLUDE)}"errors.h"\
	{$(INCLUDE)}"events.h"\
	{$(INCLUDE)}"files.h"\
	{$(INCLUDE)}"filetypesandcreators.h"\
	{$(INCLUDE)}"finder.h"\
	{$(INCLUDE)}"fixmath.h"\
	{$(INCLUDE)}"fonts.h"\
	{$(INCLUDE)}"gestalt.h"\
	{$(INCLUDE)}"gxmath.h"\
	{$(INCLUDE)}"gxtypes.h"\
	{$(INCLUDE)}"icons.h"\
	{$(INCLUDE)}"imagecodec.h"\
	{$(INCLUDE)}"imagecompression.h"\
	{$(INCLUDE)}"intlresources.h"\
	{$(INCLUDE)}"macmemory.h"\
	{$(INCLUDE)}"mactypes.h"\
	{$(INCLUDE)}"macwindows.h"\
	{$(INCLUDE)}"mediahandlers.h"\
	{$(INCLUDE)}"menus.h"\
	{$(INCLUDE)}"mixedmode.h"\
	{$(INCLUDE)}"movies.h"\
	{$(INCLUDE)}"moviesformat.h"\
	{$(INCLUDE)}"notification.h"\
	{$(INCLUDE)}"numberformatting.h"\
	{$(INCLUDE)}"osutils.h"\
	{$(INCLUDE)}"patches.h"\
	{$(INCLUDE)}"printing.h"\
	{$(INCLUDE)}"processes.h"\
	{$(INCLUDE)}"qdoffscreen.h"\
	{$(INCLUDE)}"qtml.h"\
	{$(INCLUDE)}"quickdraw.h"\
	{$(INCLUDE)}"quickdrawtext.h"\
	{$(INCLUDE)}"quicktimecomponents.h"\
	{$(INCLUDE)}"quicktimemusic.h"\
	{$(INCLUDE)}"resources.h"\
	{$(INCLUDE)}"script.h"\
	{$(INCLUDE)}"sound.h"\
	{$(INCLUDE)}"standardfile.h"\
	{$(INCLUDE)}"stringcompare.h"\
	{$(INCLUDE)}"textcommon.h"\
	{$(INCLUDE)}"textedit.h"\
	{$(INCLUDE)}"textutils.h"\
	{$(INCLUDE)}"traps.h"\
	{$(INCLUDE)}"video.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "QTEffects - Win32 Debug"

DEP_CPP_MAIN_=\
	".\main.h"\
	".\qteffects.h"\
	{$(INCLUDE)}"aedatamodel.h"\
	{$(INCLUDE)}"aliases.h"\
	{$(INCLUDE)}"appleevents.h"\
	{$(INCLUDE)}"appletalk.h"\
	{$(INCLUDE)}"codefragments.h"\
	{$(INCLUDE)}"collections.h"\
	{$(INCLUDE)}"components.h"\
	{$(INCLUDE)}"conditionalmacros.h"\
	{$(INCLUDE)}"controls.h"\
	{$(INCLUDE)}"datetimeutils.h"\
	{$(INCLUDE)}"dialogs.h"\
	{$(INCLUDE)}"drag.h"\
	{$(INCLUDE)}"endian.h"\
	{$(INCLUDE)}"errors.h"\
	{$(INCLUDE)}"events.h"\
	{$(INCLUDE)}"files.h"\
	{$(INCLUDE)}"filetypesandcreators.h"\
	{$(INCLUDE)}"finder.h"\
	{$(INCLUDE)}"fixmath.h"\
	{$(INCLUDE)}"fonts.h"\
	{$(INCLUDE)}"gestalt.h"\
	{$(INCLUDE)}"gxmath.h"\
	{$(INCLUDE)}"gxtypes.h"\
	{$(INCLUDE)}"icons.h"\
	{$(INCLUDE)}"imagecodec.h"\
	{$(INCLUDE)}"imagecompression.h"\
	{$(INCLUDE)}"intlresources.h"\
	{$(INCLUDE)}"macmemory.h"\
	{$(INCLUDE)}"mactypes.h"\
	{$(INCLUDE)}"macwindows.h"\
	{$(INCLUDE)}"mediahandlers.h"\
	{$(INCLUDE)}"menus.h"\
	{$(INCLUDE)}"mixedmode.h"\
	{$(INCLUDE)}"movies.h"\
	{$(INCLUDE)}"moviesformat.h"\
	{$(INCLUDE)}"notification.h"\
	{$(INCLUDE)}"numberformatting.h"\
	{$(INCLUDE)}"osutils.h"\
	{$(INCLUDE)}"patches.h"\
	{$(INCLUDE)}"printing.h"\
	{$(INCLUDE)}"processes.h"\
	{$(INCLUDE)}"qdoffscreen.h"\
	{$(INCLUDE)}"qtml.h"\
	{$(INCLUDE)}"quickdraw.h"\
	{$(INCLUDE)}"quickdrawtext.h"\
	{$(INCLUDE)}"quicktimecomponents.h"\
	{$(INCLUDE)}"quicktimemusic.h"\
	{$(INCLUDE)}"resources.h"\
	{$(INCLUDE)}"script.h"\
	{$(INCLUDE)}"sound.h"\
	{$(INCLUDE)}"standardfile.h"\
	{$(INCLUDE)}"stringcompare.h"\
	{$(INCLUDE)}"textcommon.h"\
	{$(INCLUDE)}"textedit.h"\
	{$(INCLUDE)}"textutils.h"\
	{$(INCLUDE)}"traps.h"\
	{$(INCLUDE)}"video.h"\
	

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\QTEffects.c

!IF  "$(CFG)" == "QTEffects - Win32 Release"

DEP_CPP_QTEFF=\
	".\qteffects.h"\
	{$(INCLUDE)}"aedatamodel.h"\
	{$(INCLUDE)}"aliases.h"\
	{$(INCLUDE)}"appleevents.h"\
	{$(INCLUDE)}"appletalk.h"\
	{$(INCLUDE)}"codefragments.h"\
	{$(INCLUDE)}"collections.h"\
	{$(INCLUDE)}"components.h"\
	{$(INCLUDE)}"conditionalmacros.h"\
	{$(INCLUDE)}"controls.h"\
	{$(INCLUDE)}"datetimeutils.h"\
	{$(INCLUDE)}"dialogs.h"\
	{$(INCLUDE)}"drag.h"\
	{$(INCLUDE)}"endian.h"\
	{$(INCLUDE)}"errors.h"\
	{$(INCLUDE)}"events.h"\
	{$(INCLUDE)}"files.h"\
	{$(INCLUDE)}"filetypesandcreators.h"\
	{$(INCLUDE)}"finder.h"\
	{$(INCLUDE)}"fixmath.h"\
	{$(INCLUDE)}"gestalt.h"\
	{$(INCLUDE)}"gxmath.h"\
	{$(INCLUDE)}"gxtypes.h"\
	{$(INCLUDE)}"icons.h"\
	{$(INCLUDE)}"imagecodec.h"\
	{$(INCLUDE)}"imagecompression.h"\
	{$(INCLUDE)}"intlresources.h"\
	{$(INCLUDE)}"macmemory.h"\
	{$(INCLUDE)}"mactypes.h"\
	{$(INCLUDE)}"macwindows.h"\
	{$(INCLUDE)}"mediahandlers.h"\
	{$(INCLUDE)}"menus.h"\
	{$(INCLUDE)}"mixedmode.h"\
	{$(INCLUDE)}"movies.h"\
	{$(INCLUDE)}"moviesformat.h"\
	{$(INCLUDE)}"notification.h"\
	{$(INCLUDE)}"numberformatting.h"\
	{$(INCLUDE)}"osutils.h"\
	{$(INCLUDE)}"patches.h"\
	{$(INCLUDE)}"printing.h"\
	{$(INCLUDE)}"processes.h"\
	{$(INCLUDE)}"qdoffscreen.h"\
	{$(INCLUDE)}"qtml.h"\
	{$(INCLUDE)}"quickdraw.h"\
	{$(INCLUDE)}"quickdrawtext.h"\
	{$(INCLUDE)}"quicktimecomponents.h"\
	{$(INCLUDE)}"quicktimemusic.h"\
	{$(INCLUDE)}"resources.h"\
	{$(INCLUDE)}"script.h"\
	{$(INCLUDE)}"sound.h"\
	{$(INCLUDE)}"standardfile.h"\
	{$(INCLUDE)}"stringcompare.h"\
	{$(INCLUDE)}"textcommon.h"\
	{$(INCLUDE)}"textedit.h"\
	{$(INCLUDE)}"textutils.h"\
	{$(INCLUDE)}"traps.h"\
	{$(INCLUDE)}"video.h"\
	

"$(INTDIR)\QTEffects.obj" : $(SOURCE) $(DEP_CPP_QTEFF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "QTEffects - Win32 Debug"

DEP_CPP_QTEFF=\
	".\qteffects.h"\
	{$(INCLUDE)}"aedatamodel.h"\
	{$(INCLUDE)}"aliases.h"\
	{$(INCLUDE)}"appleevents.h"\
	{$(INCLUDE)}"appletalk.h"\
	{$(INCLUDE)}"codefragments.h"\
	{$(INCLUDE)}"collections.h"\
	{$(INCLUDE)}"components.h"\
	{$(INCLUDE)}"conditionalmacros.h"\
	{$(INCLUDE)}"controls.h"\
	{$(INCLUDE)}"datetimeutils.h"\
	{$(INCLUDE)}"dialogs.h"\
	{$(INCLUDE)}"drag.h"\
	{$(INCLUDE)}"endian.h"\
	{$(INCLUDE)}"errors.h"\
	{$(INCLUDE)}"events.h"\
	{$(INCLUDE)}"files.h"\
	{$(INCLUDE)}"filetypesandcreators.h"\
	{$(INCLUDE)}"finder.h"\
	{$(INCLUDE)}"fixmath.h"\
	{$(INCLUDE)}"gestalt.h"\
	{$(INCLUDE)}"gxmath.h"\
	{$(INCLUDE)}"gxtypes.h"\
	{$(INCLUDE)}"icons.h"\
	{$(INCLUDE)}"imagecodec.h"\
	{$(INCLUDE)}"imagecompression.h"\
	{$(INCLUDE)}"intlresources.h"\
	{$(INCLUDE)}"macmemory.h"\
	{$(INCLUDE)}"mactypes.h"\
	{$(INCLUDE)}"macwindows.h"\
	{$(INCLUDE)}"mediahandlers.h"\
	{$(INCLUDE)}"menus.h"\
	{$(INCLUDE)}"mixedmode.h"\
	{$(INCLUDE)}"movies.h"\
	{$(INCLUDE)}"moviesformat.h"\
	{$(INCLUDE)}"notification.h"\
	{$(INCLUDE)}"numberformatting.h"\
	{$(INCLUDE)}"osutils.h"\
	{$(INCLUDE)}"patches.h"\
	{$(INCLUDE)}"printing.h"\
	{$(INCLUDE)}"processes.h"\
	{$(INCLUDE)}"qdoffscreen.h"\
	{$(INCLUDE)}"qtml.h"\
	{$(INCLUDE)}"quickdraw.h"\
	{$(INCLUDE)}"quickdrawtext.h"\
	{$(INCLUDE)}"quicktimecomponents.h"\
	{$(INCLUDE)}"quicktimemusic.h"\
	{$(INCLUDE)}"resources.h"\
	{$(INCLUDE)}"script.h"\
	{$(INCLUDE)}"sound.h"\
	{$(INCLUDE)}"standardfile.h"\
	{$(INCLUDE)}"stringcompare.h"\
	{$(INCLUDE)}"textcommon.h"\
	{$(INCLUDE)}"textedit.h"\
	{$(INCLUDE)}"textutils.h"\
	{$(INCLUDE)}"traps.h"\
	{$(INCLUDE)}"video.h"\
	

"$(INTDIR)\QTEffects.obj"	"$(INTDIR)\QTEffects.sbr" : $(SOURCE)\
 $(DEP_CPP_QTEFF) "$(INTDIR)"


!ENDIF 


!ENDIF 

