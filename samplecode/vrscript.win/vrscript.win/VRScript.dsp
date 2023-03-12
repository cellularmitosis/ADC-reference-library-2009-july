# Microsoft Developer Studio Project File - Name="VRScript" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=VRScript - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VRScript.mak".
!MESSAGE 
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

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "VRScript - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Od /I "..\..\QTDevWin\CIncludes" /I "." /I ".\Feature Files" /I ".\Application Files" /I ".\Common Files" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QD3D_AVAIL" /D "_EXCEPTION_DEFINED" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\..\QTDevWin\Libraries\qtmlclient.lib ..\..\QTDevWin\Libraries\qd3d.lib ..\..\QTDevWin\Libraries\qtvr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /ML /W3 /Gm /GX /Zi /Od /I "..\..\QTDevWin\CIncludes" /I "." /I ".\Feature Files" /I ".\Application Files" /I ".\Common Files" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "QD3D_AVAIL" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ..\..\QTDevWin\Libraries\qtmlclient.lib ..\..\QTDevWin\Libraries\qd3d.lib ..\..\QTDevWin\Libraries\qtvr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib version.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "VRScript - Win32 Release"
# Name "VRScript - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=".\Application Files\ComApplication.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\ComFramework.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\FileUtilities.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTUtilities.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTVRUtilities.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\URLUtilities.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VR3DObjects.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VR3DTexture.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRActions.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VREffects.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRHash.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRMovies.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRPicture.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRPreferences.c"
# End Source File
# Begin Source File

SOURCE=.\VRScript.c
# End Source File
# Begin Source File

SOURCE=".\Application Files\VRScript.rc"

!IF  "$(CFG)" == "VRScript - Win32 Release"

# ADD BASE RSC /l 0x409 /i "Application Files"
# ADD RSC /l 0x409 /i "Application Files" /i ".\Application Files"

!ELSEIF  "$(CFG)" == "VRScript - Win32 Debug"

# ADD BASE RSC /l 0x409 /i "Application Files"
# ADD RSC /l 0x409 /i "Application Files" /i ".\Application Files"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRSound.c"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRSprites.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\WinFramework.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=".\Application Files\ComApplication.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\FileUtilities.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTUtilities.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTVRUtilities.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VR3DObjects.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VR3DTexture.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VREffects.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRMovies.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRPicture.h"
# End Source File
# Begin Source File

SOURCE=.\VRScript.h
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRSound.h"
# End Source File
# Begin Source File

SOURCE=".\Feature Files\VRSprites.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\WinFramework.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
