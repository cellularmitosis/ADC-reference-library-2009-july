# Microsoft Developer Studio Project File - Name="SndEqualizer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SndEqualizer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoundPlayer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoundPlayer.mak" CFG="SndEqualizer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SndEqualizer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SndEqualizer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SndEqualizer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\\" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Common Files" /I ".\Application Files" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c /Tp
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\QTDevWin\Libraries"
# SUBTRACT LINK32 /incremental:yes /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
TargetName=SoundPlayer
SOURCE="$(InputPath)"
PostBuild_Desc=Adding Mac-style resources...
PostBuild_Cmds=attrib -R $(OutDir)\$(TargetName).exe	..\..\QTDevWin\Tools\RezWack -d $(OutDir)\$(TargetName).exe -r     $(OutDir)\$(TargetName).qtr -o $(OutDir)\Temp$(TargetName).exe	attrib -R     $(OutDir)\Temp$(TargetName).exe	attrib -R $(OutDir)\$(TargetName).exe	del     $(OutDir)\$(TargetName).exe	ren $(OutDir)\Temp$(TargetName).exe     $(TargetName).exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SndEqualizer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Common Files" /I ".\Application Files" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmtd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib /pdbtype:sept /libpath:"..\..\QTDevWin\Libraries"
# SUBTRACT LINK32 /incremental:no
# Begin Special Build Tool
OutDir=.\Debug
TargetName=SoundPlayer
SOURCE="$(InputPath)"
PostBuild_Desc=Adding Mac-style resources...
PostBuild_Cmds=attrib -R $(OutDir)\$(TargetName).exe	..\..\QTDevWin\Tools\RezWack -d $(OutDir)\$(TargetName).exe -r     $(OutDir)\$(TargetName).qtr -o $(OutDir)\Temp$(TargetName).exe	attrib -R     $(OutDir)\Temp$(TargetName).exe	attrib -R $(OutDir)\$(TargetName).exe	del     $(OutDir)\$(TargetName).exe	ren $(OutDir)\Temp$(TargetName).exe     $(TargetName).exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SndEqualizer - Win32 Release"
# Name "SndEqualizer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\Application Files\ComApplication.c"
# End Source File
# Begin Source File

SOURCE=".\Common Files\ComFramework.c"
# End Source File
# Begin Source File

SOURCE=.\MacApplication.r

!IF  "$(CFG)" == "SndEqualizer - Win32 Release"

# Begin Custom Build
TargetDir=.\Release
TargetName=SoundPlayer
InputPath=.\MacApplication.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i      ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "SndEqualizer - Win32 Debug"

# Begin Custom Build
TargetDir=.\Debug
TargetName=SoundPlayer
InputPath=.\MacApplication.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i      ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PlaySoundFillBuffer.c
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTUtilities.c"
# End Source File
# Begin Source File

SOURCE=".\Application Files\SndEqualizer.rc"
# End Source File
# Begin Source File

SOURCE=".\Common Files\WinFramework.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AtomInfo.h
# End Source File
# Begin Source File

SOURCE=".\Application Files\ComApplication.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\ComFramework.h"
# End Source File
# Begin Source File

SOURCE=".\Application Files\ComResource.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\QTUtilities.h"
# End Source File
# Begin Source File

SOURCE=.\SoundPlayer.h
# End Source File
# Begin Source File

SOURCE=".\Common Files\WinFramework.h"
# End Source File
# Begin Source File

SOURCE=".\Common Files\WinPrefix.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\Application Files\movie.ico"
# End Source File
# Begin Source File

SOURCE=".\Application Files\SndEqualizer.ico"
# End Source File
# End Group
# End Target
# End Project
