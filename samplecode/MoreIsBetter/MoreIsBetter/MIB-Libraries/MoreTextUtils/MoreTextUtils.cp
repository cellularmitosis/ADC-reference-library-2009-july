/*
	File:		MoreTextUtils.cp

	Contains:	Simple utilities for operating on text.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreTextUtils.cp,v $
Revision 1.12  2002/11/25 18:05:46         
Eliminate "implicit arithmetic conversion" warnings.

Revision 1.11  2002/11/09 00:12:19         
Convert nil to NULL. Convert MoreAssertQ to assert. Convert MoreAssert to MoreAssertPCG.

Revision 1.10  2001/11/07 15:55:43         
Tidy up headers, add CVS logs, update copyright.


         <9>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <8>     15/2/01    Quinn   Major changes to GetNewStringList to get it to build under
                                    Project Builder.
         <7>     11/4/00    Quinn   Added more 'STR#' routines with a new, consistent naming
                                    convention.  Updated header comment (blergh!).
         <6>     20/3/00    Quinn   Added MoreReplaceText and its support routines.  Added
                                    MoreStrLen.  Tidied up a little.
         <5>      1/3/00    Quinn   Minor Carbonation changes.
         <4>     21/4/99    Quinn   Added ValidStringListHandle.
         <3>     2/15/99    PCG     InlineGetHandleSize loses the Inline
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <2>    10/23/98    PCG     add GetStringFromLongDouble
         <1>     6/16/98    PCG     initial checkin
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Our Prototypes

#include "MoreTextUtils.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Resources.h>
	#include <PLStringFuncs.h>
	#include <Fonts.h>
#endif

// MIB Prototypes

#include "MoreMemory.h"
#include "MoreQuickDraw.h"

/////////////////////////////////////////////////////////////////

pascal OSErr NewStringPtr (ConstStr255Param init, UInt8 maxSize, StringPtr *result)
{
	OSErr err = noErr;

	if (!MoreAssertPCG (result && (!init || (*init <= maxSize))))
		err = paramErr;
	else
	{
		*result = StringPtr (NewPtr (1 + (maxSize ? maxSize : (init ? *init : 255))));

		if (!*result)
			err = MemError ( );
		else if (init && *init)
			PLstrcpy (*result,init);
		else
			**result = 0;
	}

	return err;
}

pascal OSErr GetNewStringList (short resID, tStringListP *newStringList)
{
	OSErr err = noErr;
    
    // I made substantial changes to this routine (to get it to compile under 
    // gcc, which barfed on the list element of tStringListP being declared 
    // as []) and did not test them (because I did not have time).  You should 
    // at least step through this routine in the debugger once before you trust 
    // my changes.
    // -- Quinn, 8 Feb 2001
    
    assert(false);

	Handle h = GetResource ('STR#',resID);

	if (!h)
	{
		err = ResError ( );
		if (!err) err = resNotFound;
	}
	else
	{
		Size handleSize = GetHandleSize (h);
		assert(MemError ( ) == noErr);

		if (handleSize < 2)
			err = paramErr;
		else
		{
			UInt16	stringCount		= ** (UInt16 **) h;
			Size	stringListSize	= (Size) (sizeof(**newStringList) - (sizeof(ConstStringPtr) * kVariableLengthArray) 
                                        + (stringCount * sizeof(ConstStringPtr))
                                        + (handleSize - sizeof(UInt16)));
    
            // Allocate enough space for the count, followed by the 
            // variable length list array, followed by the packed 
            // string list (everything from the 'STR#' handle except 
            // the leading UInt16 count).
            
			*newStringList = (tStringListP) NewPtr (stringListSize);

			if (!*newStringList)
				err = MemError ( );
			else
			{
				(*newStringList)->count = stringCount;

				if (stringCount)
				{
                    // Init stringScan by pointing it to the first byte after 
                    // the last pointer in the list array.
                    
					StringPtr stringScan = (StringPtr) (&(*newStringList)->list[stringCount]);

                    // Copy the 'STR#' string data (ie everything after the 
                    // leading UInt16 string count) into the pointer block we 
                    // allocated.
                    
					BlockMoveData (*h + sizeof(UInt16), stringScan, handleSize - (Size) sizeof(UInt16));
    
                    // For each string in the variable length list array, 
                    // initialise the list array element (ie a pointer 
                    // to a Str255) to point to the current string (pointed 
                    // to by stringScan), then move stringScan on to the next 
                    // string in the packed string data.
                    
					UInt16 index = 0;
					do
					{
						(*newStringList)->list [index] = stringScan;
						stringScan += *stringScan + 1;
					}
					while (++index < stringCount);
				}
			}
		}

		ReleaseResource (h);
		assert(ResError ( ) == noErr);
	}

	return err;
}

extern pascal OSStatus MoreStrListNew(Handle *strList)
	// See comment in header file.
{
	OSStatus err;

	assert(strList != NULL);
		
	*strList = NewHandleClear(sizeof(SInt16));
	err = MoreMemError(*strList);

	return err;
}

static OSStatus CheckAndPrepStrList(Handle strList, SInt8 *s)
	// Checks whether strList is something like a string list handle
	// and marks the handle as not purgeable.  Sets *s to the
	// old handle state so that the caller can restore it
	// when done.
	//
	// Also, if the handle has been purged, calls the Resource Manager
	// LoadResource routine to attempt to unpurge it.  If that 
	// fails, spits out an error.
{
	OSStatus err;
	
	assert(s != NULL);
	
	err = noErr;
	if (strList == NULL) {
		err = paramErr;
	}
	if (err == noErr && *strList == NULL) {
		
		// Ideally I would like to call HGetState and check for the
		// kHandleIsResourceBit bit before calling LoadResource.  But
		// HGetState returns an error if you call it on a purged handle,
		// so I�m forced to call LoadResource unilaterally.  If the
		// handle is not a resource, Resource Manager will fail the
		// LoadResource call.
		
		LoadResource(strList);

		// LoadResource doesn�t reliably set ResError (specifically, 
		// in the case where it runs out of memory), so we just recheck
		// for a purged handle.

		if (*strList == NULL) {
			err = ResError();
			if (err == noErr) {
				err = memFullErr;
			}
		}
	}
	if (err == noErr) {
		*s = HGetState(strList);			assert( MemError() == noErr);
		HNoPurge(strList);					assert( MemError() == noErr);
	}
	return err;
}

static ByteCount GetOffsetOfIndexedString(Handle strList, SInt16 index)
	// Returns the offset of the index'th string in the
	// string list handle.  index must be in the range [1..count].
{
	ByteCount offset;
	ByteCount maxOffset;
	SInt16    thisIndex;
	
	assert(MoreStrListValidate(strList));

	if ( MoreAssertPCG(index >= 1 && index <= **((SInt16 **) strList)) ) {
		thisIndex = 1;
		offset = sizeof(SInt16);
		maxOffset = (ByteCount) GetHandleSize(strList);
		while (thisIndex != index && offset < maxOffset) {
			offset += ((UInt8 *) *strList)[offset] + 1;
			thisIndex += 1;
		}
		if (offset >= maxOffset) {
			offset = 0;
		}
	} else {
		offset = 0;
	}
	
	return offset;
}

static OSStatus CheckAndPrepStrListAndCalcOffset(Handle strList, SInt8 *s, SInt16 index, ByteCount *offset)
	// Does the same as CheckAndPrepStrList but also checks to see
	// whether index is within bounds (ie in [1..count]) and, if it
	// is, returns the offset of the index�th string in the handle.
{
	OSStatus err;

	assert(offset != NULL);
		
	err = CheckAndPrepStrList(strList, s);
	if (err == noErr) {
		if (index < 1 || index > **((SInt16 **) strList)) {
			err = paramErr;
		}
		if (err == noErr) {
			*offset = GetOffsetOfIndexedString(strList, index);
			if (*offset == 0) {
				err = paramErr;
			}
		}
		if (err != noErr) {
			HSetState(strList, *s);
		}
	}
	return err;
}

extern pascal OSStatus MoreStrListEmpty(Handle strList)
	// See comment in header file.
{
	OSStatus err;
	SInt8    s;
	SInt16   tmpSInt16;
	
	// Don�t call validate because strList doesn�t have to be a valid
	// string list in order for us to empty it.
	
	err = CheckAndPrepStrList(strList, &s);
	if (err == noErr) {
		tmpSInt16 = 0;
		err = PtrToXHand(&tmpSInt16, strList, sizeof(tmpSInt16));
		
		HSetState(strList, s);
	}
	assert(err != noErr || MoreStrListValidate(strList));
	return err;
}

extern pascal SInt16   MoreStrListCount(Handle strList)
	// See comment in header file.
{
	OSStatus err;
	SInt8    s;
	SInt16   result;
	
	assert(MoreStrListValidate(strList));

	result = 0;
	err = CheckAndPrepStrList(strList, &s);
	if (err == noErr) {
		result = **((SInt16 **) strList);
		HSetState(strList, s);
	}
	assert(err == noErr);		// nowhere to place the error in non-debug build
	return result;
}

extern pascal OSStatus MoreStrListGetIndexed(Handle strList, SInt16 index, Str255 str)
	// See comment in header file.
{
	OSStatus  err;
	SInt8     s;
	ByteCount offset;
	
	assert(MoreStrListValidate(strList));
	assert(str != NULL);
	
	err = CheckAndPrepStrListAndCalcOffset(strList, &s, index, &offset);
	if (err == noErr) {
		BlockMoveData(*strList + offset, str, ((UInt8 *) *strList)[offset] + 1);
		
		HSetState(strList, s);
	}
	return err;
}

extern pascal OSStatus MoreStrListSetIndexed(Handle strList, SInt16 index, ConstStr255Param str)
	// See comment in header file.
{
	OSStatus  err;
	SInt8     s;
	ByteCount offset;
	
	assert(MoreStrListValidate(strList));
	assert(str != NULL);
	
	err = CheckAndPrepStrListAndCalcOffset(strList, &s, index, &offset);
	if (err == noErr) {
		// Munger Explained:
		// At offset bytes into the strList, search for an N byte
		// string (where N is length of the string at offset)
		// with any contents (NULL) and replace with str.
		(void) Munger(strList, (long) offset, NULL, ((UInt8 *) *strList)[offset] + 1, str, str[0] + 1);
		err = MemError();
		
		HSetState(strList, s);
	}
	assert(MoreStrListValidate(strList));		// Even if we fail, the result should still be well-formed.
	return err;
}

extern pascal OSStatus MoreStrListDeleteIndexed(Handle strList, SInt16 index)
	// See comment in header file.
{
	OSStatus  err;
	SInt8     s;
	ByteCount offset;
	
	assert(MoreStrListValidate(strList));
	
	err = CheckAndPrepStrListAndCalcOffset(strList, &s, index, &offset);
	if (err == noErr) {
		// Munger Explained:
		// At offset bytes into the strList, search for an N byte
		// string (where N is length of the string at offset) with
		// any contents (NULL) and replace with nothing ((Ptr) -1, 0).
		(void) Munger(strList, (long) offset, NULL, ((UInt8 *) *strList)[offset] + 1, (Ptr) -1, 0);
		err = MemError();

		if (err == noErr) {
			**((SInt16 **) strList) -= 1;
		}
		
		HSetState(strList, s);
	}
	assert(MoreStrListValidate(strList));		// Even if we fail, the result should still be well-formed.
	return err;
}

extern pascal OSStatus MoreStrListInsertAtIndex(Handle strList, SInt16 index, ConstStr255Param str)
	// See comment in header file.
{
	OSStatus  err;
	SInt8     s;
	ByteCount offset;
	
	assert(MoreStrListValidate(strList));
	assert(str != NULL);
	
	err = CheckAndPrepStrList(strList, &s);
	if (err == noErr) {
		
		// If we�re adding to the end, call the append routine.
		// We have to do this check inside the CheckAndPreStrList
		// because the strList might be purged and we need to read
		// the string count from it.  Calling MoreStrListAppend (as
		// opposed to some internal routine) isn�t a problem because
		// the only real effect from CheckAndPreStrList is to mark
		// the handle as non-purgeable, which is idempotent.

		if (index == ((**((SInt16 **) strList) + 1))) {
			err = MoreStrListAppend(strList, str);
		} else {
		
			// Now we know we�re inserting in the middle.  Do the
			// bounds check on index and then find the offset of
			// the current index'th string.
			
			if (index < 1 || index > **((SInt16 **) strList)) {
				err = paramErr;
			}
			if (err == noErr) {
				offset = GetOffsetOfIndexedString(strList, index);
				if (offset == 0) {
					err = paramErr;
				}
			}
			
			// Insert the new string in the handle and then up the string
			// count.
			
			if (err == noErr) {
				// Munger Explained:
				// At offset bytes into the strList, search for nothing
				// (NULL, 0) and replace with str (str, str[0] + 1).
				(void) Munger(strList, (long) offset, NULL, 0, str, str[0] + 1);
				err = MemError();
			}
			if (err == noErr) {
				**((SInt16 **) strList) += 1;
			}
		}
		
		HSetState(strList, s);
	}
	assert(MoreStrListValidate(strList));		// Even if we fail, the result should still be well-formed.
	return err;
}

extern pascal OSStatus MoreStrListAppend(Handle strList, ConstStr255Param str)
	// See comment in header file.
{
	OSStatus  err;
	SInt8     s;

	assert(MoreStrListValidate(strList));
	assert(str != NULL);
	
	err = CheckAndPrepStrList(strList, &s);
	if (err == noErr) {
		err = PtrAndHand(str, strList, str[0] + 1);
		if (err == noErr) {
			**((SInt16 **) strList) += 1;
		}
		
		HSetState(strList, s);
	}
	assert(MoreStrListValidate(strList));		// Even if we fail, the result should still be well-formed.
	return err;
}

extern pascal Boolean  MoreStrListValidate(Handle strList)
	// See comment in header file.
{
	Boolean result;
	UInt16 stringCount;
	UInt16 stringIndex;
	UInt8* cursor;
	UInt8* bound;
	ByteCount strListLength;
	SInt8  s;

	if ( CheckAndPrepStrList(strList, &s) != noErr) {
		result = false;
	} else {
		assert(strList != NULL);			// because otherwise CheckAndPreStrList would have errored
		assert(*strList != NULL);			// ditto
		
		result = true;
		if ( GetHandleSize(strList) < (Size) sizeof(UInt16) ) {
			result = false;
		}
		if (result) {
			stringCount = *((UInt16 *) *strList);
			
			strListLength = (ByteCount) GetHandleSize(strList);
			
			// From here down we have to make sure we do nothing to move
			// until we're done with cursor and bound.
			
			cursor = ((UInt8 *)*strList) + sizeof(UInt16);
			bound  = ((UInt8 *)*strList) + strListLength;
			for (stringIndex = 0; stringIndex < stringCount; stringIndex++) {
				if ( cursor < bound ) {
					cursor += *cursor + 1;
				} else {
					result = false;
					break;
				}
			}
			
			if (result) {
				if ( cursor != bound ) {
					result = false;
				}
			}
		}
		HSetState(strList, s);
	}
	
	return result;
}

#if 0

// This routine has been retired.  See comment in header file for my rationale.
// -- Quinn, 8 Feb 2001

pascal OSErr GetPascalStringFromLongDouble (long double d, SInt8 precision, StringPtr buf)
{
	OSErr err = noErr;

	*buf = 0;

	//	If client requests more precision than is available, bitch.

	if (!MoreAssertPCG (precision < SIGDIGLEN))
		err = paramErr;
	else
	{
		// 	If precision is less than 0, client is requesting
		//	as much precision as is available.

		if (precision < 0)
			precision = SIGDIGLEN;

		//	Try normal notation, and if that overflows,
		//	fall back to scientific notation (bleah!).

		decimal dec;
		decform df = { 1, 0, precision };

		num2dec (&df,d,&dec);

		if (dec.sig.text [0] == '?')
		{
			df.style	= 0;
			df.digits	= SIGDIGLEN;

			num2dec (&df,d,&dec);
		}

		if (*(dec.sig.text) == '0')
			PLstrcpy (buf,"\p0");
		else if (*(dec.sig.text) == 'I')
			PLstrcpy (buf,"\p[INF]");
		else if (*(dec.sig.text) == 'N')
			PLstrcpy (buf,"\p[NaN]");
		else
		{
			//	Finally, convert it to a pascal string...

			dec2str (&df,&dec,(Ptr)buf);
			CopyCStringToPascal( (char *) buf, buf );

			//	...and trim trailing zeroes.

//			if (df.style == 1 && dec.exp < 0)
//				while (buf [*buf] == '0')
//					*buf -= 1;
		}
	}

	return err;
}

#endif

// gDummyPort is a pointer to a dummy GrafPort whose font is always
// the system font.  By setting thePort to this port, we can then safely
// call Script Manager that get script information from the current port.

static GrafPtr gDummyPort;

extern pascal OSStatus InitMoreTextUtils(void)
	// See comment in header.
{
	OSStatus err;
	GrafPtr  oldPort;

	// Preserve the current port.
		
	GetPort(&oldPort);
	
	// Create gDummyPort.
	
	gDummyPort = (GrafPtr) CreateNewPort();
	err = MoreMemError(gDummyPort);
	if (err == noErr) {
	
		// CreateNewPort, if it follows the semantics of the old
		// OpenPort routine, should set thePort to the new port.
		// This assert ensures that behaviour, because the following
		// call to TextFont relies on it.
		
		#if MORE_DEBUG
			{
				GrafPtr junkPort;
				
				GetPort(&junkPort);
				assert(junkPort == gDummyPort);
			}
		#endif

		// Set the dummy port�s font to the system font.
				
		TextFont(systemFont);
	}
	
	SetPort(oldPort);
	
	return err;
}

extern pascal void TermMoreTextUtils(void)
	// See comment in header.
{
	if (gDummyPort != NULL) {
		DisposePort( (CGrafPtr) gDummyPort);
		gDummyPort = NULL;
	}
}

extern pascal void MoreReplaceText(Str255 baseText, ConstStr255Param substitutionText, ConstStr15Param key)
	// See comment in header.
{
	OSStatus err;
	GrafPtr  oldPort;
	Handle   baseTextH;
	Handle   substitutionTextH;
	Size     newLength;
	UInt8 	 *tmpKey;

	assert(baseText != NULL);
	assert(substitutionText != NULL);
	assert(key != NULL);
	assert(key[0] <= 15);
	
	GetPort(&oldPort);
	
	baseTextH = NULL;
	substitutionTextH = NULL;

	// Copy the strings into handles, call ReplaceText, then
	// copy the result back into baseText (making sure that
	// there is at most 255 bytes in the resulting string).
		
	err = noErr;
	if (gDummyPort == NULL) {
		err = InitMoreTextUtils();
	}
	if (err == noErr) {
		SetPort(gDummyPort);

		err = PtrToHand(&baseText[1], &baseTextH, baseText[0]);
	}
	if (err == noErr) {
		err = PtrToHand(&substitutionText[1], &substitutionTextH, substitutionText[0]);
	}
	if (err == noErr) {
		tmpKey = (UInt8 *) key;
		err = ReplaceText(baseTextH, substitutionTextH, tmpKey);
		if (err >= 0) {
			newLength = GetHandleSize(baseTextH);
			if (newLength > 255) {
				newLength = 255;
			}
			baseText[0] = (UInt8) newLength;
			BlockMoveData(*baseTextH, &baseText[1], newLength);
			err = noErr;
		}
	}
	
	// Clean up.

	if (baseTextH != NULL) {
		DisposeHandle(baseTextH);
		assert(MemError() == noErr);
	}	
	if (substitutionTextH != NULL) {
		DisposeHandle(substitutionTextH);
		assert(MemError() == noErr);
	}	
	SetPort(oldPort);
	
	assert(err == noErr);
}

extern pascal ByteCount MoreStrLen(const char *str)
	// See comment in header.
{
	ByteCount result;
	
	result = 0;
	while ( *str != 0 ) {
		result += 1;
		str += 1;
	}
	return result;
}
