# Microsoft Developer Studio Project File - Name="Dimmer2EffectWindows" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Dimmer2EffectWindows - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Dimmer2Effect.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dimmer2Effect.mak" CFG="Dimmer2EffectWindows - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dimmer2EffectWindows - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Dimmer2EffectWindows - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /I ".\EffectFramework" /I ".\Effect" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "TARGET_OS_WIN32" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"libcmtd.lib" /out:".\Release\Dimmer2EffectWindows.qtx" /libpath:"..\..\QTDevWin\Libraries"
# SUBTRACT LINK32 /verbose /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
ProjDir=.
TargetName=Dimmer2EffectWindows
SOURCE="$(InputPath)"
PostBuild_Desc=Embedding Mac-style resources into our extension...
PostBuild_Cmds=attrib -R "$(OutDir)\$(TargetName).qtx"	RezWack -f -d "$(OutDir)\$(TargetName).qtx" -r "$(OutDir)\$(TargetName).qtr" -o "$(ProjDir)\$(TargetName).qtx"	attrib -R "$(ProjDir)\$(TargetName).qtx"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX- /ZI /Od /I ".\Effect" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "TARGET_OS_WIN32" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:".\Debug\Dimmer2EffectWindowsDebug.qtx" /libpath:"..\..\QTDevWin\Libraries"
# Begin Special Build Tool
OutDir=.\Debug
ProjDir=.
TargetName=Dimmer2EffectWindowsDebug
SOURCE="$(InputPath)"
PostBuild_Desc=Embedding Mac-style resources into our extension...
PostBuild_Cmds=attrib -R "$(OutDir)\$(TargetName).qtx"	RezWack -f -d "$(OutDir)\$(TargetName).qtx" -r "$(OutDir)\$(TargetName).qtr" -o "$(ProjDir)\$(TargetName).qtx"	attrib -R "$(ProjDir)\$(TargetName).qtx"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Dimmer2EffectWindows - Win32 Release"
# Name "Dimmer2EffectWindows - Win32 Debug"
# Begin Group "EffectFramework"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EffectFramework\BltMacros.h
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\Effect.c
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\Effect.r

!IF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Release"

!ELSEIF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EffectFramework\EffectDefinitions.h
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\EffectDispatch.h
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\EffectUtilities.c
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\EffectUtilities.h
# End Source File
# End Group
# Begin Group "Effect"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Effect\EffectFilter16.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Effect\EffectFilter32.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\Dimmer2Effect.def
# End Source File
# Begin Source File

SOURCE=.\Dllmain.c
# End Source File
# Begin Source File

SOURCE=.\EffectFramework\EffectRezWin.r

!IF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Release"

# Begin Custom Build - Compile Resources
TargetDir=.\Release
TargetName=Dimmer2EffectWindows
InputPath=.\EffectFramework\EffectRezWin.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rez.exe -rd -i "C:\Program Files\Microsoft Visual Studio\VC98\QTDevWin\RIncludes" -i .\EffectFramework -o "$(TargetDir)\$(TargetName).qtr" <  "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Dimmer2EffectWindows - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Compile Resources
TargetDir=.\Debug
TargetName=Dimmer2EffectWindowsDebug
InputPath=.\EffectFramework\EffectRezWin.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rez.exe -p -rd -i "C:\Program Files\Microsoft Visual Studio\VC98\QTDevWin\RIncludes" -i .\EffectFramework -o "$(TargetDir)\$(TargetName).qtr" <  "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
