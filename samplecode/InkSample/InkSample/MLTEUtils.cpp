/*
    File: MLTEUtils.cpp
        
    Contains: Definition of convenience functions for common setup needed before calling MLTE API.
    
    Version: InkSample 1.1

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
        ("Apple") in consideration of your agreement to the following terms, and your
        use, installation, modification or redistribution of this Apple software
        constitutes acceptance of these terms.  If you do not agree with these terms,
        please do not use, install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and subject
        to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
        
    Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
*/

#include <iostream>
#include "MLTEUtils.h"

#ifndef MLTE_API_IN_HITOOLBOX_133
#define MLTE_API_IN_HITOOLBOX_133 1
#endif

static OSStatus MLTEInit();

Point&
HIPointToQDPoint( const HIPoint& inPoint, Point& outPoint )
{
    outPoint.h = (SInt16)inPoint.x;
    outPoint.v = (SInt16)inPoint.y;
    return outPoint;
}

TXNObject
MLTECreateObject( WindowRef window, Rect& bounds, Boolean readOnly )
{
    OSStatus status = paramErr;
    TXNObject mlteObj = NULL;
    TXNFrameID unusedID;
    
    static TXNFrameOptions frameOptions = kTXNAlwaysWrapAtViewEdgeMask
                                          | kTXNWantVScrollBarMask;
    if( readOnly )
        frameOptions |= kTXNReadOnlyMask;
                       
    TXNControlTag iControlTags[2] = { kTXNDoFontSubstitution, kTXNVisibilityTag };
    TXNControlData iControlData[2] = { {false}, {false} };
    
    status = MLTEInit(); // will init MLTE once per app context
    
    require_noerr( status, MLTE_Initialize_err );
    
    status = TXNNewObject( NULL /*fsspec*/,
                            window, /*window*/
                            &bounds/*mlteRect*/,
                            frameOptions /*mlteFrameOptions*/,
                            kTXNTextEditStyleFrameType, /*required*/
                            kTXNUnicodeTextFile, /*outputDataType*/
                            kTXNSystemDefaultEncoding,
                            &mlteObj,
                            &unusedID,
                            NULL /*txnObjRefcon*/);

    require_noerr( status, MLTE_Initialize_err );
    
    status = ::TXNSetTXNObjectControls( mlteObj, false, 2,
                                        iControlTags, iControlData );
    
MLTE_Initialize_err:

    return mlteObj;
}
    
// -----------------------------------------------------------------------------
// SetMLTEObjectVisibility [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
MLTESetObjectVisibility( TXNObject mlteObj, Boolean vis )
{
    TXNControlTag iControlTags[1] = { kTXNVisibilityTag };
    TXNControlData iControlData[1] = {{ vis }};
    return ::TXNSetTXNObjectControls( mlteObj, false, 2,
                                        iControlTags, iControlData );
}

// -----------------------------------------------------------------------------
// MLTESetCFStringToObject [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
MLTESetCFStringToObject( TXNObject mlteObj, CFStringRef cfString,
                         TXNOffset start, TXNOffset end )
{
    OSStatus status = paramErr;
    CFIndex len;
    ByteCount size;
    UniChar* charBuffer = NULL;
    CFRange range;
    
    // get the length of the string (# of UniChars)
    len = CFStringGetLength( cfString );
    
    range.location = 0;
    range.length = len;
    
    // allocation size, 2 bytes per UniChar
    size = len * 2;
    // allocate buffer
    charBuffer = (UniChar *) malloc( size );
    
    // Get the string, and set it to the mlte object
    if ( charBuffer != NULL )
    {
        ::CFStringGetCharacters(cfString, range, charBuffer);
        status = ::TXNSetData( mlteObj, kTXNUnicodeTextData,
                                charBuffer, size, start, end );
        // clean up
        free( charBuffer );
    }
    return status;
}

// -----------------------------------------------------------------------------
// MLTESetStringToObject [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
MLTESetStringToObject( TXNObject mlteObject, const Ptr pData, SInt32 dataSize,
                        TXNOffset startOffset, TXNOffset endOffset )
{
    OSStatus status = paramErr;
	
    require( mlteObject != NULL && pData != NULL, SetData_Fail );

    status = ::TXNSetData( mlteObject, kTXNTextData, pData, dataSize, 
                           startOffset, endOffset );
            
    SetData_Fail:
#if MY_DEBUG
    if( status != noErr )
        printf("Error while setting data to TXNObject from buffer : %li", status );
#endif

    return status;
}

// -----------------------------------------------------------------------------
// MLTEProcessHICommand [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
MLTEProcessHICommand(TXNObject mlteObj, const HICommand& hiCommand)
{
    OSStatus status = eventNotHandledErr;
    switch( hiCommand.commandID )
    {
        case kHICommandCut:
            ::TXNCut(  mlteObj );
            status = noErr;
            break;
        case kHICommandCopy:
            ::TXNCopy(  mlteObj );
            status = noErr;
            break;
        case kHICommandPaste:
            ::TXNConvertFromPublicScrap();
            ::TXNPaste(  mlteObj );
            status = noErr;
            break;
        case kHICommandClear:
            ::TXNClear(  mlteObj );
            status = noErr;
            break;
        case kHICommandSelectAll:
            status = ::TXNSetSelection( mlteObj, kTXNStartOffset, kTXNEndOffset );
            break;
        case kHICommandUndo:
        {
            TXNActionKey actionKey;
            if ( ::TXNCanUndo(  mlteObj, &actionKey ) )
                ::TXNUndo(  mlteObj );
            status = noErr;
            break;
        }
    }
    return status;
}

// -----------------------------------------------------------------------------
// MLTEGlobalHIPointToOffset [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
MLTEGlobalHIPointToOffset( TXNObject mlteObj, HIPoint& iHIPoint, TXNOffset& oOffset )
{
    Point qdPoint;
    CGrafPtr objPort, savePort;
    HIViewRef view;
    WindowRef window;

    #if MLTE_API_IN_HITOOLBOX_133
    // Get the mlte object's window/port
    window = TXNGetWindowRef( mlteObj );
    objPort = GetWindowPort( window );

    // Convert HIPoint to QD point
    qdPoint.h = (SInt16)iHIPoint.x;
    qdPoint.v = (SInt16)iHIPoint.y;

    // Convert from global coordinates to local coordinates
    GetPort( &savePort );
    SetPort( objPort );
    GlobalToLocal( &qdPoint );
    SetPort( savePort );

    // Convert QD point back to HIPoint
    iHIPoint.x = (float)qdPoint.h;
    iHIPoint.y = (float)qdPoint.v;

    // Convert from window to view coordinates
    view = HIViewGetRoot( window );
    HIViewConvertPoint( &iHIPoint, NULL, view );

    // Now finally pass the point to MLTE to determine the offset
    return TXNHIPointToOffset( mlteObj, &iHIPoint, &oOffset );
    #else
    return paramErr;
    #endif
}

// -----------------------------------------------------------------------------
// MLTEInit [PUBLIC] - only initialize MLTE one time per application context
// -----------------------------------------------------------------------------

static OSStatus MLTEInit()
{
    static Boolean sMLTEWasInitialized = false;
    
    OSStatus status = noErr;
    
    // only call TXNInitTextension once per app context
    // note that displaying NavigationServices dialog may init MLTE with
    // Nav specific settings if TXNInitTextesion was not called 
    // by client app code before that.
    if( sMLTEWasInitialized == false )
    {
	const ItemCount kfontDescCount = 1;
	TXNMacOSPreferredFontDescription fontDesc;
	
	const Str255 kDefaultFontName = "\pMarker Felt";
                
	SInt16	qdFontID;
	GetFNum( kDefaultFontName, &qdFontID );
        
	fontDesc.fontID = qdFontID;
	fontDesc.pointSize = ff(32);
	fontDesc.fontStyle = kTXNDontCareTypeStyle;
	
	fontDesc.encoding = kTextEncodingMacRoman;

	TXNMacOSPreferredFontDescription fontDescArray[kfontDescCount] = { fontDesc };
	
	const ItemCount fontDescCount= sizeof( fontDescArray ) / 
                                            sizeof(TXNMacOSPreferredFontDescription);
        
        ::QDDisplayWaitCursor( true );
	status = ::TXNInitTextension( fontDescArray, fontDescCount, kTXNWantGraphicsMask );
        ::QDDisplayWaitCursor( false );
        
	if( status == noErr )
            sMLTEWasInitialized = true;	
        
    } else
        status = noErr;
        
    return status;
}