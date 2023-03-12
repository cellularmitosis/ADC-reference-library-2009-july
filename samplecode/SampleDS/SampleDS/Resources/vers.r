/*
	File:		vers.r
	
	Description:	Sample Data Source resource file

	Author:	Image Capture Engineering

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):

	   <1>	 	6/17/03	WN			first file

*/

#include "Carbon.r"

#define Version_Major		0x01
#define Version_Minor		0x09
#define Version_Revision	0x00
#define Version_Stage		betaStage
#define Version_Build		0x02

#if Version_Stage == developStage
	#define Version_Stage_Short	'd'
	#define Version_String	$$Format("%x.%x.%x%c%x", Version_Major, Version_Minor,\
		Version_Revision, Version_Stage_Short, Version_Build)

#elif Version_Stage == alphaStage
	#define Version_Stage_Short	'a'
	#define Version_String	$$Format("%x.%x.%x%c%x", Version_Major, Version_Minor,\
		Version_Revision, Version_Stage_Short, Version_Build)

#elif Version_Stage == betaStage
	#define Version_Stage_Short	'b'
	#define Version_String	$$Format("%x.%x.%x%c%x", Version_Major, Version_Minor,\
		Version_Revision, Version_Stage_Short, Version_Build)

#elif Version_Stage == finalStage
	#define Version_String	$$Format("%x.%x.%x", Version_Major, Version_Minor)

#endif

resource 'vers' (1, purgeable)
{
	Version_Major,
	(Version_Minor << 4) | Version_Revision,
	Version_Stage,
	Version_Build,
	verUS,
	Version_String,
	Version_String " TWAIN Sample Data Source"
};

resource 'vers' (2, purgeable)
{
	Version_Major,
	(Version_Minor << 4) | Version_Revision,
	Version_Stage,
	Version_Build,
	verUS,
	Version_String,
	"(part of TWAIN 1.9)"
};

resource 'STR ' (123)
{
	"hello"
};