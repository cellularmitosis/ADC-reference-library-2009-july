/*
    File:	 BIMInputEvents.c

    Description: Functions for sending input events to the target application.

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

           	 2001/04/18	SC	Added routines to handle events sent by the server's
                                        send event palette.
                                        Updated Carbon event names.
		 2000/07/28	SC	Changed to include Carbon.h
		 2000/06/01	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BIM.h"
#include "BIMClientServer.h"
#include "BIMInputEvents.h"

/************************************************************************************************
*
*  BIMUpdateActiveInputArea
*
*  Create and sends an "update active input area" event to our client application. We use the
*  Carbon event manager to create a Carbon event, add the appropriate parameters and send it
*  using SendTextInputEvent.
*
*	In	inSessionHandle		A reference to the active session.
*		inFix			If TRUE, we are fixing the entire input area. Otherwise
*					we are just updating the contents.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSErr BIMUpdateActiveInputArea( BIMSessionHandle inSessionHandle, Boolean inFix )
{
    OSErr error;
    long fixLength;
    EventRef event;
    ComponentInstance componentInstance;
    ScriptLanguageRecord scriptLanguageRecord;
    TextRange pinRange;
    TextRangeArrayPtr hiliteRangePtr;
    TextRangeArrayPtr updateRangePtr;

    if (inFix)
        fixLength = (*inSessionHandle)->fInputBufferCount * 2;
    else
        fixLength = 0;

    hiliteRangePtr = nil;
    updateRangePtr = nil;


    //  Create a new Text Input event (a Carbon event of class "kEventClassTextInput" and kind
    //  "kEventTextInputUpdateActiveInputArea").

    error = CreateEvent( NULL, kEventClassTextInput, kEventTextInputUpdateActiveInputArea,
                         GetCurrentEventTime(), kEventAttributeUserEvent, &event );

    //  The first required parameter is a reference to our text service component instance. Add
    //  it to the event.
    
    if( error == noErr ) {
        componentInstance = (*inSessionHandle)->fComponentInstance;
        error = SetEventParameter( event, kEventParamTextInputSendComponentInstance,
                                   typeComponentInstance, sizeof( ComponentInstance ),
                                   &componentInstance );
    }

    //	The second parameter we need is the script information record. This tells what script
    //  and language the text is in.

    if( error == noErr ) {
        scriptLanguageRecord.fScript = smRoman;			// smJapanese
        scriptLanguageRecord.fLanguage = langEnglish;		// langJapanese
        error = SetEventParameter( event, kEventParamTextInputSendSLRec,
                                   typeIntlWritingCode, sizeof( ScriptLanguageRecord ),
                                   &scriptLanguageRecord );
    }

    //  Now put all of the conversion text into the event. Note that some applications will
    //  always replace all of the active inline area text with what's passed to them, so we have
    //  to pass the entire handle, even if all we're doing is moving the insertion point. Bummer.

    if( error == noErr )
        error = SetEventParameter( event, kEventParamTextInputSendText,
                                   typeUnicodeText,
                                   (*inSessionHandle)->fInputBufferCount * sizeof( UniChar ),
                                   (*inSessionHandle)->fInputBuffer );

    //  The fix length, or the length of text that should be dumped from the inline hole when
    //  we're finished.

    if( error == noErr )
        error = SetEventParameter( event, kEventParamTextInputSendFixLen,
                                   typeLongInteger, sizeof( long ),
                                   &fixLength );

    //  If the text hasn't been changed, then we want to pass an update handle that contains no
    //  update regions, otherwise our handle will say that everything has changed.

    if( error == noErr ) {
        updateRangePtr = (TextRangeArrayPtr) NewPtrClear( sizeof( short ) + sizeof( TextRange ) * 2 );
        if (updateRangePtr) {
            updateRangePtr->fNumOfRanges = 2;
            updateRangePtr->fRange [0].fStart = 0;
            updateRangePtr->fRange [0].fEnd = (*inSessionHandle)->fLastUpdateLength * sizeof( UniChar );
            updateRangePtr->fRange [1].fHiliteStyle = 0;
            updateRangePtr->fRange [1].fStart = 0;
            updateRangePtr->fRange [1].fEnd = (*inSessionHandle)->fInputBufferCount * sizeof( UniChar );
            updateRangePtr->fRange [1].fHiliteStyle = 0;

            (*inSessionHandle)->fLastUpdateLength = (*inSessionHandle)->fInputBufferCount;
        }
        else
            error = memFullErr;
    }

    if( error == noErr )
        error = SetEventParameter( event, kEventParamTextInputSendUpdateRng,
                                   typeTextRangeArray, sizeof( short ) + sizeof( TextRange ) * 2,
                                   updateRangePtr );

    //	Specify the hilite range.

    if( error == noErr ) {
        hiliteRangePtr = (TextRangeArrayPtr) NewPtrClear (sizeof( short ) + sizeof( TextRange ) * 2);
        if( hiliteRangePtr ) {
            hiliteRangePtr->fNumOfRanges = 2;
            hiliteRangePtr->fRange [0].fStart = 0;
            hiliteRangePtr->fRange [0].fEnd = (*inSessionHandle)->fInputBufferCount * sizeof( UniChar );
            if (inFix)
                hiliteRangePtr->fRange [0].fHiliteStyle = kConvertedText;
            else
                hiliteRangePtr->fRange [0].fHiliteStyle = kRawText;
            hiliteRangePtr->fRange [1].fStart = (*inSessionHandle)->fInputBufferCount * sizeof( UniChar );
            hiliteRangePtr->fRange [1].fEnd = (*inSessionHandle)->fInputBufferCount * sizeof( UniChar );
            hiliteRangePtr->fRange [1].fHiliteStyle = kCaretPosition;
        }
        else
            error = memFullErr;
    }
    if( error == noErr )
        error = SetEventParameter( event, kEventParamTextInputSendHiliteRng,
                                   typeTextRangeArray, sizeof( short ) + sizeof( TextRange ) * 2,
                                   hiliteRangePtr );

    //	We don't have any clause information.

    pinRange.fStart = 0;
    pinRange.fEnd = (*inSessionHandle)->fInputBufferCount * sizeof( UniChar );
    if( error == noErr )
        error = SetEventParameter( event, kEventParamTextInputSendPinRng,
                                   typeTextRange, sizeof( TextRange ),
                                   &pinRange );

    // The event is all ready to go, so send it off.

    if( error == noErr )
        error = SendTextInputEvent( event );

    //	Dispose of everything.

    DisposePtr( (Ptr) hiliteRangePtr );
    DisposePtr( (Ptr) updateRangePtr );
    return error;
}

/************************************************************************************************
*
*  BIMHandleOffsetToPos
*
*  Called when the user clicks the "Send" button in the server process debug palette. We use the
*  given parameters to generate and send a kOffsetToPos event to our client
*  application and extract the results to be sent back to the server.
*
*	In	inSessionHandle			A reference to the active session.
*		offsetToPosParams		Parameters for the event.
*
*	Out	offsetToPosParams		Returned parameters.
*
************************************************************************************************/

void BIMHandleOffsetToPos( BIMSessionHandle inSessionHandle,
                           struct OffsetToPosParams *offsetToPosParams )
{
    OSStatus		status;
    ComponentInstance	componentInstance;
    EventRef		event;
    
    event = NULL;

    //  Create a new Text Input event (a Carbon event of class "kEventClassTextInput" and kind
    //  "kEventTextInputOffsetToPos").

    status = CreateEvent( NULL, kEventClassTextInput, kEventTextInputOffsetToPos,
                          GetCurrentEventTime(), kEventAttributeUserEvent, &event );

    //  The first required parameter is a reference to our text service component instance.

    if( status == noErr ) {
        componentInstance = (*inSessionHandle)->fComponentInstance;
        status = SetEventParameter( event, kEventParamTextInputSendComponentInstance,
                                    typeComponentInstance, sizeof( ComponentInstance ),
                                    &componentInstance );
    }

    //  Add the required refCon parameter.
    
    if( status == noErr )
        status = SetEventParameter( event, kEventParamTextInputSendRefCon, typeLongInteger,
                                    sizeof( long ), &offsetToPosParams->fRefCon );

    //  Add the required text offset parameter.
    
    if( status == noErr )
        status = SetEventParameter( event, kEventParamTextInputSendTextOffset, typeLongInteger,
                                    sizeof( long ), &offsetToPosParams->fTextOffset );

    //  Add the optional script/language parameter.
    
    if( status == noErr )
        status = SetEventParameter( event, kEventParamTextInputSendSLRec, typeIntlWritingCode,
                                    sizeof( ScriptLanguageRecord ), &offsetToPosParams->fSLRec );

    //  Add the optional leading edge parameter.
    
    if( status == noErr )
        status = SetEventParameter( event, kEventParamTextInputSendLeadingEdge, typeBoolean,
                                    sizeof( Boolean ), &offsetToPosParams->fLeadingEdge );

    //  The event is all ready to go, so send it off.

    if( status == noErr )
        status = SendTextInputEvent( event );

    //  Extract the required point parameter.

    if( status == noErr )
        status = GetEventParameter( event, kEventParamTextInputReplyPoint, typeQDPoint, NULL,
                                    sizeof( Point ), NULL, &offsetToPosParams->fReplyPoint );
                                    
    //  Extract the optional script/language parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplySLRec, typeIntlWritingCode,
                                    NULL, sizeof( ScriptLanguageRecord ), NULL,
                                    &offsetToPosParams->fReplySLRec );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplySLRecSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplySLRecSpecified = FALSE;
    }
        
    //  Extract the optional font parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplyFont, typeLongInteger,
                                    NULL, sizeof( long ), NULL,
                                    &offsetToPosParams->fReplyFont );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplyFontSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplyFontSpecified = FALSE;
    }
                                    
    //  Extract the optional point size parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplyPointSize, typeFixed,
                                    NULL, sizeof( Fixed ), NULL,
                                    &offsetToPosParams->fReplyPointSize );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplyPointSizeSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplyPointSizeSpecified = FALSE;
    }
                                    
    //  Extract the optional line height parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplyLineHeight,
                                    typeShortInteger, NULL, sizeof( short ), NULL,
                                    &offsetToPosParams->fReplyLineHeight );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplyLineHeightSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplyLineHeightSpecified = FALSE;
    }
                                    
    //  Extract the optional line ascent parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplyLineAscent,
                                    typeShortInteger, NULL, sizeof( short ), NULL,
                                    &offsetToPosParams->fReplyLineAscent );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplyLineAscentSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplyLineAscentSpecified = FALSE;
    }
                                    
    //  Extract the optional text angle parameter.

    if( status == noErr ) {
        status = GetEventParameter( event, kEventParamTextInputReplyTextAngle, typeFixed,
                                    NULL, sizeof( Fixed ), NULL,
                                    &offsetToPosParams->fReplyTextAngle );
        if( status == eventParameterNotFoundErr ) {
            status = noErr;
            offsetToPosParams->fReplyTextAngleSpecified = TRUE;
        }
        else
            offsetToPosParams->fReplyTextAngleSpecified = FALSE;
    }

    //	Dispose of everything.

    if( event )
        ReleaseEvent( event );

    offsetToPosParams->fReplyResultCode = status;
}
