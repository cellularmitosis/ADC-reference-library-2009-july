#
#	File:		LateImportTest.c
#
#	Contains:	MPW build instructions for the LateImportTest program.
#
#	Written by:	Quinn
#
#	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.
#
#	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
#				("Apple") in consideration of your agreement to the following terms, and your
#				use, installation, modification or redistribution of this Apple software
#				constitutes acceptance of these terms.  If you do not agree with these terms,
#				please do not use, install, modify or redistribute this Apple software.
#
#				In consideration of your agreement to abide by the following terms, and subject
#				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
#				copyrights in this original Apple software (the "Apple Software"), to use,
#				reproduce, modify and redistribute the Apple Software, with or without
#				modifications, in source and/or binary forms; provided that if you redistribute
#				the Apple Software in its entirety and without modifications, you must retain
#				this notice and the following text and disclaimers in all such redistributions of
#				the Apple Software.  Neither the name, trademarks, service marks or logos of
#				Apple Computer, Inc. may be used to endorse or promote products derived from the
#				Apple Software without specific prior written permission from Apple.  Except as
#				expressly stated in this notice, no other rights or licenses, express or implied,
#				are granted by Apple herein, including but not limited to any patent rights that
#				may be infringed by your derivative works or by other works in which the Apple
#				Software may be incorporated.
#
#				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
#				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
#				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
#				COMBINATION WITH YOUR PRODUCTS.
#
#				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
#				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
#				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
#				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
#				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
#				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	Change History (most recent first):
#
# $Log: LateImportTest.make,v $
# Revision 1.2  2001/11/07 15:50:15         
# Tidy up headers, add CVS logs, update copyright.
#
#
#        <1>     17/1/00    Quinn   First checked in.
#

#
#	{MoreIsBetter} must be set to the root of the MoreIsBetter source hierarchy.
#	{MIBInterfaces} must be set to the MoreIsBetter Universal Interfaces.
#	{MIBStubLibraries} must be set to the MoreIsBetter stub libraries.
#

#	Locate the CFMLateImport directory within MIB.

CFMLateImportDir = {MoreIsBetter}MIB-Libraries:MoreCodeFragments:CFMLateImport:

#	Locate the objects directory within MIB.

ObjectsDir = {MoreIsBetter}MIB-Libraries:MoreCodeFragments:CFMLateImport:Objects:

#	Set up some standard variables.

COpts = ¶
	-traceback 											# we *always* want traceback tables (-: ¶
	-i "{MIBInterfaces}" 								# for all the Universal Interfaces ¶
	-i "{MoreIsBetter}"MIB-Libraries: 					# for MoreSetup.h ¶
	-i "{MoreIsBetter}"MIB-Libraries:MoreInterfaceLib: 	# for MoreInterfaceLib.h ¶
	-i "{CFMLateImportDir}"				 				# for CFMLateImport.h ¶
	-i "{CFMLateImportDir}"LateImportTest:TestLibrary: 	# for TestLibrary.h ¶
	-i "{CIncludes}"									# Standard MPW ANSI C stuff

#	Start with the high-level target.

LateImportTest	Ä	¶
		"{CFMLateImportDir}"LateImportTest:LateImportTest-PPC-MPW
	
#	To build LateImportTest-PPC-MPW, we need to link the corresponding .o file
#	and Rez in SIOW.

"{CFMLateImportDir}"LateImportTest:LateImportTest-PPC-MPW ÄÄ ¶
		"{ObjectsDir}"LateImportTest.c.o ¶
		"{ObjectsDir}"CFMLateImport.c.o ¶
		"{ObjectsDir}"MoreInterfaceLib.c.o ¶
		"{PPCLibraries}"PPCSIOW.o ¶
		"{PPCLibraries}"PPCCRuntime.o ¶
		"{SharedLibraries}"StdCLib ¶
		"{MIBStubLibraries}"InterfaceLib ¶
		"{CFMLateImportDir}"LateImportTest:TestLibrary:TestLibrary.stub
	PPCLink ¶
		{deps} ¶
		-xm executable 									# build an application ¶
		-o {targ}	 									# named LateImportTest-PPC-MPW ¶
		-weaklib TestLibrary						 	# weak link with our stub library ¶
		-init MyCFragInitFunction						# specify the fragment initialization function ¶
		-packdata off									# CFMLateImport requires unpacked data sections

"{CFMLateImportDir}"LateImportTest:LateImportTest-PPC-MPW ÄÄ ¶
		"{RIncludes}"SIOW.r
	Rez {deps} -append -o {targ}

#	To build LateImportTest.c.o, we need to compile the corresponding .c file.

"{ObjectsDir}"LateImportTest.c.o Ä	¶
		"{CFMLateImportDir}"LateImportTest:LateImportTest.c
	MrC {deps} ¶
		{COpts} ¶
		-o "{ObjectsDir}"

"{CFMLateImportDir}"LateImportTest:LateImportTest.c Ä ¶
	"{CFMLateImportDir}"LateImportTest:TestLibrary:TestLibrary.h

#	To build CFMLateImport.c.o, we need to compile the corresponding .c file.

"{ObjectsDir}"CFMLateImport.c.o Ä	¶
		"{CFMLateImportDir}"CFMLateImport.c
	MrC {deps} ¶
		{COpts} ¶
		-o "{ObjectsDir}"

#	To build CFMLateImport.c.o, we need to compile the corresponding .c file.

"{ObjectsDir}"MoreInterfaceLib.c.o Ä	¶
		"{MoreIsBetter}"MIB-Libraries:MoreInterfaceLib:MoreInterfaceLib.c
	MrC {deps} ¶
		{COpts} ¶
		-o "{ObjectsDir}"
