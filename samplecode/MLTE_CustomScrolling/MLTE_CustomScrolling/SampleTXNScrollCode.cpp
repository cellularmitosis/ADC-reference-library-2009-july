/*
*	File:		SampleTXNScrollCode.cpp of MLTECustomScrolling
* 
*	Created:	2003/12/05
*
*	Contains:	Demo implementation of MLTE's custom scrolling functionality.
*	
*	Copyright:  Copyright © 2004 Apple Computer, Inc., All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
 
#include "SampleTXNScrollCode.h"

#ifndef _MLTE_SAMPLE_SCROLL_CODE_
#define _MLTE_SAMPLE_SCROLL_CODE_ 1

const SInt32 kMyINT32_MAX = 2147483647;

// global pointer to the MLTE callback
TXNScrollInfoUPP gScrollInfoUPP;

// global pointer to the control action proc
ControlActionUPP gControlActionUPP; 

// accessor to get a vertical scroller control ref out of an
// mlte user data item passed by the MLTE callback.
static ControlRef
MLTEUserDataGetVerticalScrollBar(SInt32);

// accessor to get a horizontal scroller control ref out of an
// mlte user data item passed by the MLTE callback.
static ControlRef
MLTEUserDataGetHorizontalScrollBar(SInt32);

// how tall is the view rect?
static UInt32
TextViewGetHeight( WindowRef window, OSType key );

// how wide is the view rect?
static UInt32
TextViewGetWidth( WindowRef window, OSType key );

// how tall is the text content?
static UInt32
TextViewGetTextHeight( WindowRef window, OSType key );

// what is the greatest width of the text content?
static UInt32
TextViewGetTextWidth( WindowRef window, OSType key );

// access cache values that hold the last know scroller values
SInt32
TextViewGetVertScrollCache( WindowRef window, OSType key );

SInt32
TextViewGetHorizScrollCache( WindowRef window, OSType key );

// setters to update the cache
void
TextViewSetVertScrollCache( WindowRef window, SInt32 value, OSType key );

void
TextViewSetHorizScrollCache( WindowRef window, SInt32 value, OSType key );

// tell the text view to scroll
static OSStatus
TextViewCustomScroll( WindowRef window, OSType orientation, TXNScrollUnit scrollUnit,
                      SInt32 textViewDelta, SInt32 cacheValue, OSType key );
                      
OSStatus
TextViewInvalidate( WindowRef window, OSType key );

OSStatus
TextViewDelete( WindowRef window, OSType key );

// associate control action proc with a control
static OSStatus
ControlRegisterActionProc( WindowRef window, ControlRef scrollControl,
                            TXNScrollBarOrientation orientation );

// Tell a control to update value and maximum
OSStatus
ControlUpdateValues( ControlRef scrollBar, SInt32 value, SInt32 max );

// Tracking a mouse down in a scroll control via callback
void
MyTrackActionProcedure(ControlRef controlRef, ControlPartCode partCode);

// MLTE returns data for updating the scroll controls in this callback
// whenever a change in the content of the TXNObject occurs that 
// requires updated scrollbars.
void
MyMLTEScrollInfoCallback( SInt32 value, SInt32 max,
                    TXNScrollBarOrientation orientation, SInt32 mlteUserData);

// Associate a tracking procedure UPP with each of a scroll control.
OSStatus
ControlRegisterActionProc( WindowRef window, ControlRef scrollControl,
                            TXNScrollBarOrientation orientation )
{
    OSStatus status = noErr;
    SInt32 max = 200;
    
    require( IsValidControlHandle( scrollControl ), BadControl );
    
    // Create the callback
    if( gControlActionUPP == NULL )
        gControlActionUPP = NewControlActionUPP( MyTrackActionProcedure );
    require( gControlActionUPP != NULL, NullControlActionUPP );
    
    // Associate the callback with the control
    SetControlAction( scrollControl, gControlActionUPP );
    
    // set the max value of the control based on the height of the text
    // in the text view
    if( orientation = kTXNVertical )
    {
        SInt32 height = (SInt32)TextViewGetTextHeight( window, kMyMLTEDataStructProperty );
        max = (  height <= kMyINT32_MAX ? height : kMyINT32_MAX );
    }
    else
    {
        SInt32 width = (SInt32)TextViewGetTextWidth( window, kMyMLTEDataStructProperty );
        max = (  width <= kMyINT32_MAX ? width : kMyINT32_MAX );
    }
    
    SetControl32BitMaximum( scrollControl, max );
    
    BadControl:
    NullControlActionUPP:
    return status;
}

// Tell a control to update its value & maximum.
OSStatus
ControlUpdateValues( ControlRef scrollBar, SInt32 value, SInt32 max )
{
    require( IsValidControlHandle( scrollBar ), BadControl );
    
    SetControl32BitValue( scrollBar, value );
    SetControl32BitMaximum( scrollBar, max );

    return noErr;
    
    BadControl:
    return paramErr;
}

// Tracking the mouse in a scroll control via callback
void
MyTrackActionProcedure( ControlRef controlRef, ControlPartCode partCode )
{
    OSStatus status;
    SInt32 value;
    SInt32 textViewDelta, controlDelta, controlMin, controlMax;
    TXNScrollUnit scrollUnits;
    ControlID controlID = {0,0};
    WindowRef window = NULL;

    // when user clicks on kControlPageUpPart or kControlPageDownPart
    // we need to know the viewRect height and width so we can update
    // the scrollbars with a delta of "one page" scrolled
    SInt32 viewHeight = 0;
    SInt32 viewWidth = 0;
    
    scrollUnits = kTXNScrollUnitsInPixels;
    textViewDelta = controlDelta = 0;
    
    // the controlID will tell us if it is a v or h scroller
    // based on the arbitrary signature we assigned it in the nib (Hscr or Vscr)
    status = GetControlID( controlRef, &controlID );
    require_noerr( status, CantGetControlID );
    
    // the window has cached scroll data we need, so we need to get the window
    window = GetControlOwner( controlRef );
    require( window != NULL, CantGetWindow );
  
    value = GetControl32BitValue( controlRef );
    
    controlMin = GetControl32BitMinimum( controlRef );
    controlMax = GetControl32BitMaximum( controlRef );
    
    // calc view width or height ( for page up/down/left/right )
    
    if( controlID.signature == kMyVerticalScrollBar )
        viewHeight = TextViewGetHeight( window, kMyMLTEDataStructProperty );
    else
        viewWidth = TextViewGetWidth( window, kMyMLTEDataStructProperty );
    
    // which part of the scroll control was hit?
    switch ( partCode )
    {
        case kControlDownButtonPart:
            textViewDelta = controlDelta = 1;
            scrollUnits = kTXNScrollUnitsInPixels; //kTXNScrollUnitsInLines;
            break;
        case kControlUpButtonPart:
            textViewDelta = controlDelta = -1;
            scrollUnits = kTXNScrollUnitsInPixels; //kTXNScrollUnitsInLines;
            break;
        case kControlPageUpPart:
            textViewDelta = -1; // units in viewRectHeight/Width(s)
            if( controlID.signature == kMyVerticalScrollBar )
                controlDelta = ( viewHeight <= kMyINT32_MAX ? viewHeight : kMyINT32_MAX );
            else
                controlDelta = ( viewWidth <= kMyINT32_MAX ? viewWidth : kMyINT32_MAX );
            scrollUnits = kTXNScrollUnitsInViewRects;
            controlDelta = -controlDelta;
            break;
        case kControlPageDownPart:
            textViewDelta = 1; // units in viewRectHeight/Width(s)
            if( controlID.signature == kMyVerticalScrollBar )
                controlDelta = ( viewHeight <= kMyINT32_MAX ? viewHeight : kMyINT32_MAX );
            else
                controlDelta = ( viewWidth <= kMyINT32_MAX ? viewWidth : kMyINT32_MAX );
            scrollUnits = kTXNScrollUnitsInViewRects;
            break;
        // we really do not want to do anything if the
        // kControlIndicatorPart is hit.  It should be
        // handled by the Live Tracking procedure instead.
        case kControlIndicatorPart:
        {
            SInt32 cacheValue = 0;
            scrollUnits = kTXNScrollUnitsInPixels;
            // Which scrollbar is getting called by this callback?
            if( controlID.signature == kMyVerticalScrollBar )
            {
                // we can find the degree of the change by comparing to
                // our last known value
                cacheValue = TextViewGetVertScrollCache( window, kMyMLTEDataStructProperty );
                textViewDelta = value - (cacheValue <= kMyINT32_MAX ? cacheValue : kMyINT32_MAX );
            }
            else // kMyHorizontalScrollBar
            {
                // we can find the degree of the change by comparing to
                // our last known value
                cacheValue = TextViewGetHorizScrollCache( window, kMyMLTEDataStructProperty );
                textViewDelta = value - (cacheValue <= kMyINT32_MAX ? cacheValue : kMyINT32_MAX );
            }
        }
            break;
            
        default:
            break;
    }
    if ( partCode != kControlIndicatorPart ) // only update the control for non-thumb hits
    {
        value += controlDelta;
        
        if( value < controlMin ) // don't go past max or min
            value = controlMin;
        if( value > controlMax )
            value = controlMax;
            
        SetControl32BitValue( controlRef, value );
    }

    status = TextViewCustomScroll( window, controlID.signature, scrollUnits,
                                   textViewDelta, value, kMyMLTEDataStructProperty );

    CantGetWindow:
    CantGetControlID:
    return;
}

// The scroll info proc / callback is MLTE's way to tell us that the state of
// the TXNObject has changed and therefore scrollbars should be updated.
// The callback provides for a refCon/userData item, but the only help
// we get for identifying which control to update is the TXNScrollBarOrientation
// parameter.  We need to find the correct scrollbar controls on our own.

// In this implementation, the control refs are stored in an array, and a pointer
// to the start of the array is stored in the mlteUserData item.  We provide
// accessor functions to get the scroll control refs out of the mlteUserData
// so that we could change the method of storing them without affecting this
// function.
void
MyMLTEScrollInfoCallback( SInt32 value, SInt32 max,
                    TXNScrollBarOrientation orientation, SInt32 mlteUserData)
{
    OSStatus status;
    ControlRef vScrollControlRef, hScrollControlRef;
    
    if( orientation == kTXNVertical )
    {
        vScrollControlRef = MLTEUserDataGetVerticalScrollBar( mlteUserData );
        if( vScrollControlRef != NULL )
            status = ControlUpdateValues( vScrollControlRef, value, max );
        TextViewSetVertScrollCache( GetControlOwner( vScrollControlRef ),
                                    value, kMyMLTEDataStructProperty );
    }
    else if ( orientation == kTXNHorizontal )
    {
        hScrollControlRef = MLTEUserDataGetHorizontalScrollBar( mlteUserData );
        if( hScrollControlRef != NULL )
            status = ControlUpdateValues( hScrollControlRef, value, max );
        TextViewSetHorizScrollCache( GetControlOwner( hScrollControlRef ),
                                     value, kMyMLTEDataStructProperty);
    }
}

// Accessors to get the scroll controls out of the mlteUserData are below.

// The scroll controls could be stored in some other manner
// this is just a simple and direct method
//
// Recall the mlte user data (refcon) was set up as follows:
//    ControlRef scrollArray[2];
//    scrollArray[0] = vertical scroller control ref;
//    scrollArray[1] = horizontal scroller control ref;

ControlRef
MLTEUserDataGetVerticalScrollBar( SInt32 mlteUserData )
{
    ControlRef* pControl = ( ControlRef* )mlteUserData;
    if( mlteUserData != NULL && IsValidControlHandle( *pControl ) )
        return *pControl;
    return 0;
};

ControlRef
MLTEUserDataGetHorizontalScrollBar( SInt32 mlteUserData )
{
    ControlRef* pControl = ( ControlRef* )mlteUserData;
    if( mlteUserData != NULL )
    {
        pControl = pControl+1;
        if( IsValidControlHandle( *pControl ) )
            return *pControl;
    }
    return 0;
};

// When scrolling in units of view rects (rather than pixels)
// we need to get the pixels in one view rect height / width, so
// we can update the scrollbar value with the correct value.

// ask the MLTE object how many pixels equal view rect height
UInt32
TextViewGetHeight( WindowRef window, OSType key )
{
    Rect viewRect = { 0,0,0,0 };
    TXNGetViewRect( WindowGetTXNObj( window, key ), &viewRect );
    return viewRect.bottom-viewRect.top;
}

// ask the MLTE object how many pixels equal view rect width
UInt32
TextViewGetWidth( WindowRef window, OSType key )
{
    Rect viewRect = { 0,0,0,0 };
    TXNGetViewRect( WindowGetTXNObj( window, key ), &viewRect );
    return viewRect.right-viewRect.left;
}

// When updating the maximum range of a scrollbar, we need to get the
// pixel dimensions of the text in the object, so that the vertical and
// horizontal range of the scrollbar corresponds to the text in the TXNObject

// ask the MLTE Object how many pixels tall the text is
UInt32
TextViewGetTextHeight( WindowRef window, OSType key )
{
    OSStatus status = paramErr;
    TXNLongRect textRect;
    
    require( window != NULL, CantGetRect );
    
    status = TXNGetRectBounds( WindowGetTXNObj( window, key ), NULL, NULL, &textRect );
    require_noerr( status, CantGetRect );

    return textRect.right - textRect.left;
    
    CantGetRect:
    return 0;
}


// ask the MLTE Object how many pixels wide the text is
UInt32
TextViewGetTextWidth( WindowRef window, OSType key )
{
    OSStatus status = paramErr;
    TXNLongRect textRect;
    
    require( window != NULL, CantGetRect );
    
    status = TXNGetRectBounds( WindowGetTXNObj( window, key ), NULL, NULL, &textRect );
    require_noerr( status, CantGetRect );

    return textRect.bottom - textRect.top;
    
    CantGetRect:
    return 0;
}

void
TextViewSetVertScrollCache( WindowRef window, SInt32 value, OSType key )
{
    OSStatus status = paramErr;
    MyMLTEData *pMyMLTEData = NULL;
    
    require( window != NULL, CantGetObj );
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    pMyMLTEData->fVertScrollCache = value;
    
    CantGetObj:
    return;
}

void
TextViewSetHorizScrollCache( WindowRef window, SInt32 value, OSType key )
{
    OSStatus status = paramErr;
    MyMLTEData *pMyMLTEData = NULL;
    
    require( window != NULL, CantGetObj );
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    pMyMLTEData->fHorizScrollCache = value;
    
    CantGetObj:
    return;
}

SInt32
TextViewGetVertScrollCache( WindowRef window, OSType key )
{
    OSStatus status = paramErr;
    MyMLTEData *pMyMLTEData = NULL;
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    return pMyMLTEData->fVertScrollCache;
    
    CantGetObj:
    return 0;
}

SInt32
TextViewGetHorizScrollCache( WindowRef window, OSType key )
{
    OSStatus status = paramErr;
    MyMLTEData *pMyMLTEData = NULL;
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    return pMyMLTEData->fHorizScrollCache;
    
    CantGetObj:
    return 0;
}

// When the user adjusts the scroll controls, tell the MLTE object to update
// the displayed text accordingly, and also save the control values so we
// can calculate deltas later.
OSStatus
TextViewCustomScroll( WindowRef window, OSType orientation, TXNScrollUnit scrollUnit,
                      SInt32 textViewDelta, SInt32 cacheValue, OSType key )
{
    OSStatus status = paramErr;
    SInt32 vDelta, hDelta;
    MyMLTEData *pMyMLTEData = NULL;
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    // We are only handling scrolling in one direction, but when we tell MLTE to
    // scroll, we pass data for both directions.  So set the delta for the correct
    // orientation, and the other orientation will have zero delta.
    
    if( orientation == kMyVerticalScrollBar )
    {
        vDelta = textViewDelta;
        hDelta = 0;
        // We will need to know the last known control value 
        // to calculate the delta on the next time around
        pMyMLTEData->fVertScrollCache = cacheValue;
    }
    else
    {
        hDelta = textViewDelta,
        vDelta = 0;
        // We will need to know the last known control value next time around
        // to calculate the delta on the next time around
        pMyMLTEData->fHorizScrollCache = cacheValue;
    }
    
    status = TXNScroll( pMyMLTEData->fTXNObj, scrollUnit, scrollUnit,
                                    &vDelta, &hDelta );
    CantGetObj:
    return status;
}

OSStatus
TextViewProcessHICommand( WindowRef window, OSType key, const HICommand& hiCommand)
{
    OSStatus status = eventNotHandledErr;
    TXNObject mlteObj = WindowGetTXNObj( window, key );
    require( mlteObj != NULL, CantGetObj );
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
    CantGetObj:
    return status;
}

OSStatus
TextViewDraw( WindowRef window, OSType key )
{
    OSStatus status = eventNotHandledErr;
    TXNObject mlteObj = WindowGetTXNObj( window, key );
    require( mlteObj != NULL, CantGetObj );
    TXNDraw( mlteObj, NULL );
    
    status = noErr;
    
    CantGetObj:
    return status;
}

OSStatus
TextViewInvalidate( WindowRef window, OSType key )
{
    OSStatus status = eventNotHandledErr;
    Rect viewRect;
    require_action( window != NULL, BadWindow, status = eventNotHandledErr );
    
    TXNGetViewRect( WindowGetTXNObj( window, key ), &viewRect );
    
    status = InvalWindowRect( window, &viewRect );
    
    BadWindow:
    return status;   
}

OSStatus
TextViewDelete( WindowRef window, OSType key )
{
    OSStatus status = eventNotHandledErr;
    MyMLTEData *pMyMLTEData = NULL;
    
    // recover our data from the window
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_action( status == noErr, CantGetObj, status = eventNotHandledErr );

    // clean up allocated memory
    TXNDeleteObject( pMyMLTEData->fTXNObj );
    DisposePtr( (char*)pMyMLTEData );
    
    status = RemoveWindowProperty( window, kMyPropertyCreator, key );
    CantGetObj:
    return status;
}

OSStatus
WindowStoreTXNObject( WindowRef window, OSType key, TXNObject txnObj )
{
    OSStatus status = paramErr;
    
    // allocate memory for a custom struct which holds the TXNObject and
    // important scroll cache data.  Don't forget to dispose of this memory
    // allocation when the window goes away!
    MyMLTEData *pMyMLTEData = NULL;
    require_action( window != NULL, BadWindow, status = paramErr );
    
    pMyMLTEData = (MyMLTEData*)NewPtr( sizeof( MyMLTEData ) );
    require_action( pMyMLTEData != NULL, MemoryError, status = memFullErr );
    
    pMyMLTEData->fTXNObj = txnObj;
    pMyMLTEData->fVertScrollCache = 0;
    pMyMLTEData->fHorizScrollCache = 0;
    
    // put a pointer to the struct into the window
    status = SetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), &pMyMLTEData );
    require_noerr( status, CantSetProperty );

    status = TXNAttachObjectToWindowRef( txnObj, window );
    
    BadWindow:
    CantSetProperty:
    MemoryError:
    return status;
}

TXNObject
WindowGetTXNObj( WindowRef window, OSType key )
{
    OSStatus status = paramErr;
    MyMLTEData *pMyMLTEData = NULL;
    status = GetWindowProperty( window, kMyPropertyCreator, key,
                        sizeof(Ptr), NULL, &pMyMLTEData );
    require_noerr( status, CantGetObj );
    
    return pMyMLTEData->fTXNObj;
    
    CantGetObj:
    return NULL; 
}

OSStatus
WindowRegisterTXNScrollProcs( WindowRef window, TXNObject txnObj )
{
    OSStatus status = memFullErr;
    ControlRef aScrollControl;
    ControlRef* pScrollControl;
    ControlID controlID = { kMyVerticalScrollBar, 0 };
    
    // allocate memory to hold scroll controls
    Ptr scrollControlArray = NewPtr( sizeof(ControlRef) * 2 ); // don't forget to dispose when finished
    require( scrollControlArray != NULL, MemoryError );
    
    // pointer to first ControlRef in array
    pScrollControl = (ControlRef*)scrollControlArray;
    
    // set up and store vertical scroll control
    status = GetControlByID( window, &controlID, &aScrollControl );
    require_noerr( status, CantSetupControl );
    status = ControlRegisterActionProc( window, aScrollControl, kTXNVertical );
    require_noerr( status, CantSetupControl );
                            
    *pScrollControl = aScrollControl;
    
    // advance to next ControlRef in array
    pScrollControl++;
    
    // set up and store horizontal scroll control
    controlID.signature = kMyHorizontalScrollBar;
    status = GetControlByID( window, &controlID, &aScrollControl);
    require_noerr( status, CantSetupControl );
    status = ControlRegisterActionProc( window, aScrollControl, kTXNHorizontal );
    require_noerr( status, CantSetupControl );
    *pScrollControl = aScrollControl;
    
    // The scroll info proc is MLTE's way to tell you that the content of
    // the TXNObject has changed and that the scrollbars should be updated
    // We need a way to find the controlRefs to our scroll controls while inside the callback.
    // One way to do that is just put a pointer to an array of controlRefs inside the userData
    // item for the callback.  (You could also put a pointer to your own struct/class instance)
    if( scrollControlArray != NULL )
    {
        gScrollInfoUPP = NewTXNScrollInfoUPP( MyMLTEScrollInfoCallback );
        require( gScrollInfoUPP != NULL, CantMakeScrollUPP );
        TXNRegisterScrollInfoProc( txnObj, gScrollInfoUPP, (SInt32)scrollControlArray/*userData*/);
    }
    
    MemoryError:
    return status;
    
    // clean up allocated memory if jumped to here
    CantSetupControl:
    CantMakeScrollUPP:
    DisposePtr( scrollControlArray );
    return status;
    
}

OSStatus
WindowDrawContent( WindowRef window )
{
    DrawControls( window );
    return TextViewDraw( window, kMyMLTEDataStructProperty );
}

OSStatus
WindowInvalidateContent( WindowRef window )
{
    TextViewInvalidate( window, kMyMLTEDataStructProperty );
    return eventNotHandledErr;
}

OSStatus
WindowClose( WindowRef window )
{
    TextViewDelete( window, kMyMLTEDataStructProperty );
    DisposeWindow( window );
    return noErr;
}

#endif //_MLTE_SAMPLE_SCROLL_CODE_
