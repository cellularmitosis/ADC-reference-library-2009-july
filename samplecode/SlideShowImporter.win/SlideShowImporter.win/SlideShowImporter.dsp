# Microsoft Developer Studio Project File - Name="SlideShowImporter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SlideShowImporter - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SlideShowImporter.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SlideShowImporter.mak" CFG="SlideShowImporter - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SlideShowImporter - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SlideShowImporter - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SlideShowImporter - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SLIDESHOWIMPORTER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\ComponentIncludes" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Includes" /I ".\PrefixIncludes" /I ".\Resources" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SLIDESHOWIMPORTER_EXPORTS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"Release/SlideShowImport.qtx"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
OutDir=.\Release
TargetName=SlideShowImport
SOURCE="$(InputPath)"
PostBuild_Cmds=attrib -R $(OutDir)\$(TargetName).qtx    	..\..\QTDevWin\Tools\RezWack -d $(OutDir)\$(TargetName).qtx -r     $(OutDir)\$(TargetName).qtr -o $(OutDir)\Temp$(TargetName).qtx	attrib -R     $(OutDir)\Temp$(TargetName).qtx	attrib -R $(OutDir)\$(TargetName).qtx	del     $(OutDir)\$(TargetName).qtx	ren $(OutDir)\Temp$(TargetName).qtx     $(TargetName).qtx
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SlideShowImporter - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SLIDESHOWIMPORTER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\ComponentIncludes" /I "..\..\QTDevWin\CIncludes" /I "..\..\QTDevWin\RIncludes" /I "..\..\QTDevWin\ComponentIncludes" /I ".\Includes" /I ".\PrefixIncludes" /I ".\Resources" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SLIDESHOWIMPORTER_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 qtmlclient.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"Debug/SlideShowImport.qtx"
# SUBTRACT LINK32 /debug /nodefaultlib
# Begin Special Build Tool
OutDir=.\Debug
TargetName=SlideShowImport
SOURCE="$(InputPath)"
PostBuild_Cmds=attrib -R $(OutDir)\$(TargetName).qtx    	..\..\QTDevWin\Tools\RezWack -d $(OutDir)\$(TargetName).qtx -r     $(OutDir)\$(TargetName).qtr -o $(OutDir)\Temp$(TargetName).qtx	attrib -R     $(OutDir)\Temp$(TargetName).qtx	attrib -R $(OutDir)\$(TargetName).qtx	del     $(OutDir)\$(TargetName).qtx	ren $(OutDir)\Temp$(TargetName).qtx     $(TargetName).qtx
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SlideShowImporter - Win32 Release"
# Name "SlideShowImporter - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\Dllmain.c
# End Source File
# Begin Source File

SOURCE=.\source\SlideShowImport.c
# End Source File
# Begin Source File

SOURCE=.\SlideShowImporter.def
# End Source File
# Begin Source File

SOURCE=.\SlideShowImporterWin.r

!IF  "$(CFG)" == "SlideShowImporter - Win32 Release"

# Begin Custom Build
TargetDir=.\Release
TargetName=SlideShowImport
InputPath=.\SlideShowImporterWin.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i   ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -i .\source -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "SlideShowImporter - Win32 Debug"

# Begin Custom Build
TargetDir=.\Debug
TargetName=SlideShowImport
InputPath=.\SlideShowImporterWin.r

"$(TargetDir)\$(TargetName).qtr" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\QTDevWin\Tools\Rez.exe -i ..\..\QTDevWin\CIncludes -i   ..\..\QTDevWin\RIncludes -i .\Includes -i .\Resources -i .\source -o $(TargetDir)\$(TargetName).qtr <  $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\SlideShowImportVersion.h
# End Source File
# End Group
# End Target
# End Project
