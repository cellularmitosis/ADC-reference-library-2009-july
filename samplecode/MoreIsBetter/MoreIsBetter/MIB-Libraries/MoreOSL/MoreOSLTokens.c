/*
	File:		MoreOSLTokens.c

	Contains:	Token definition for MoreOSL.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSLTokens.c,v $
Revision 1.5  2002/11/08 23:48:17         
Include our prototype early to flush out any missing dependencies. Adopt MoreAEDataModel. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.4  2001/11/07 15:54:29         
Tidy up headers, add CVS logs, update copyright.


         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>     20/3/00    Quinn   Added InitPropertyMOSLTokenFromOtherProperty.  Fixed unitialised
                                    "err" in CompareMOSLTokens.
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreOSLTokens.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <AppleEvents.h>
	#include <AERegistry.h>
#endif

// MIB Prototypes

#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreBBLog.h"
#include "MoreOSL.h"				// for error codes

/////////////////////////////////////////////////////////////////

#if MORE_DEBUG

	extern pascal void BBLogMOSLToken(ConstStr255Param tag, const MOSLToken *tok)
		// See comment in header file.
	{
		OSStatus err;
		AEDesc	 tmpDesc;

		assert(tag != NULL);
		assert(tok != NULL);
		
		err = AECreateDesc(tok->tokType, tok, sizeof(*tok), &tmpDesc);
		if (err == noErr) {
			BBLogDesc(tag, &tmpDesc);
			MoreAEDisposeDesc(&tmpDesc);
		}
		assert(err == noErr);
	}

#endif

extern pascal void InitObjectMOSLToken(MOSLToken *tok, DescType tokType, void *tokData)
	// See comment in header file.
{
	assert(tok != NULL);

	tok->tokType     = tokType;
	tok->tokObjType  = tokType;
	tok->tokPropName = (OSType) 0;
	tok->tokData     = tokData;
}
	
extern pascal void InitPropertyMOSLToken(MOSLToken *tok, DescType tokObjType, DescType tokPropName, void *tokData)
	// See comment in header file.
{
	assert(tok != NULL);

	tok->tokType     = typeProperty;
	tok->tokObjType  = tokObjType;
	tok->tokPropName = tokPropName;
	tok->tokData     = tokData;
}

extern pascal void     InitPropertyMOSLTokenFromContainer(MOSLToken *tok, const MOSLToken *containerTok, DescType tokPropName)
	// See comment in header file.
{
	assert(tok != NULL);
	assert(containerTok != NULL);
	assert(containerTok->tokType != typeProperty);

	*tok = *containerTok;
	tok->tokType     = typeProperty;
	tok->tokPropName = tokPropName;
}

extern pascal void     InitPropertyMOSLTokenFromOtherProperty(MOSLToken *tok, const MOSLToken *propertyTok, DescType tokPropName)
	// See comment in header file.
{
	assert(tok != NULL);
	assert(propertyTok != NULL);
	assert(propertyTok->tokType == typeProperty);

	*tok = *propertyTok;
	tok->tokType     = typeProperty;
	tok->tokPropName = tokPropName;
}

extern pascal OSStatus MOSLTokenToDesc(const MOSLToken *tok, AEDesc *desc)
	// See comment in header file.
{
	OSStatus err;

	assert(tok  != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	err = AECreateDesc(tok->tokType, tok, sizeof(*tok), desc);
	return err;
}

extern pascal void DescToMOSLToken(const AEDesc *tokDesc, MOSLToken *tok)
	// See comment in header file.
{
	OSStatus junk;

	assert(tok     != NULL);
	assert(tokDesc != NULL);

	if (tokDesc->descriptorType == typeNull) {
		InitObjectMOSLToken(tok, cApplication, NULL);
	} else {
		assert(AEGetDescDataSize(tokDesc) == sizeof(*tok));
		junk = AEGetDescData(tokDesc, tok, sizeof(*tok));
		assert(junk == noErr);
		assert(tokDesc->descriptorType == tok->tokType);
	}
}

extern pascal OSStatus CompareMOSLTokens(DescType oper, const MOSLToken *tok1, const MOSLToken *tok2, Boolean *result)
	// See comment in header file.
{
	OSStatus err;

	assert(tok1   != NULL);
	assert(tok2   != NULL);
	assert(result != NULL);

	err = noErr;	
	switch (oper) {
		case kAEEquals:
			*result = (tok1->tokObjType  == tok2->tokObjType      )
						&& (tok1->tokPropName == tok2->tokPropName)
						&& (tok1->tokData     == tok2->tokData    );
			break;
		case kAEGreaterThanEquals:
		case kAEGreaterThan:
		case kAELessThanEquals:
		case kAELessThan:
			err = kMOSLCantRelateObjectsErr;
			break;
		default:
			assert(false);
			err = kMOSLUnrecognisedOperatorErr;
			break;
	}
	return err;
}
