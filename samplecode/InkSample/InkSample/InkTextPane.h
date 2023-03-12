/*
    File: InkTextPane.h
        
    Contains: Declaration of InkTextPane class which manages content in a single tab control
    content area, and registration function for HIView callback.
    
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

#ifndef InkSample_INKTEXTPANE
#define InkSample_INKTEXTPANE

#include <Carbon/Carbon.h>

// -----------------------------------------------------------------------------
// Control Types
// -----------------------------------------------------------------------------

extern CFStringRef gInkTextPaneClassID;

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------

OSStatus InkTextPaneRegister( void );

class InkTextPane
{
public:
    InkTextPane( HIViewRef parentInstance );
    virtual ~InkTextPane( void );
    
    virtual OSStatus Initialize( EventRef inEvent );
    virtual OSStatus ControlInitialize( EventRef inEvent );
    
    virtual OSStatus Draw( EventRef inEvent );
    virtual OSStatus Activate( EventRef inEvent );
    virtual OSStatus Deactivate( EventRef inEvent );
    
    Boolean IsParentVisible() { return ::IsControlVisible( fParentInstance ) ; }
    Boolean IsMyWindowActive() { return ( fHIViewWindow != NULL ) && IsWindowActive( fHIViewWindow ); };
    WindowRef GetWindow() { return fHIViewWindow; };
    
    // init static class globals
    static void StaticInit();
    
    // The InkTextPane uses a text view to display results of ink
    // input and gestures.  The text view interface is abstracted
    // by using only this API to interact with the text view.

    OSStatus TextViewCreate( WindowRef window, Rect& bounds );
    void TextViewSetVisibility( Boolean vis );
    void TextViewDraw();
    void TextViewFocus( Boolean focus );
    OSStatus TextViewAppendCFString( CFStringRef cfString );
    OSStatus TextViewAppendString( char* string, CFIndex len );
    OSStatus TextViewReplaceCFString( CFStringRef cfString, UInt32 start = kTXNUseCurrentSelection, UInt32 end = kTXNUseCurrentSelection );
    OSStatus TextViewReplaceString( char* str, CFIndex len, UInt32 start = kTXNUseCurrentSelection, UInt32 end = kTXNUseCurrentSelection );
    OSStatus TextViewProcessHICommand( const HICommand& cmd );
    OSStatus TextViewKeyDown( UniChar uChar );
    OSStatus TextViewSetOffsetWithHIPoint( HIPoint pt );
    OSStatus TextViewSetOffsetWithQDPoint( Point pt );
    OSStatus TextViewSetSelectionOffsets( UInt32 start, UInt32 end );
    OSStatus TextViewJoin( HIRect bounds );
    
    OSStatus TextViewHIPointToOffset( HIPoint& pt, UInt32& offset );
    OSStatus TextViewQDPointToOffset( Point& pt, UInt32& offset );

    // Use some UI to indicate what type of Ink Event
    // was handled by the ink handlers.
    OSStatus UISignalInkEvent( UInt32 signalInkEventKind );
    
    void DebugDrawPoint( HIPoint hiPt );
    void DebugDrawMLTEOffsetToPoint( TXNOffset start, TXNOffset end );
    
protected:

    // CARBON EVENT HANDLER RELATED METHODS
    virtual OSStatus InstallTextInputHandlersOnTarget( EventTargetRef targetRef );
    virtual OSStatus InstallInkHandlersOnTarget( EventTargetRef targetRef );
    virtual OSStatus InstallMenuHandlersOnTarget( EventTargetRef targetRef );
    
    // STATIC
    static OSStatus MenuEventHandler( EventHandlerCallRef inHandlerRef,
                                      EventRef inEvent, void *userData );
                                      
    static OSStatus TextInputHandler( EventHandlerCallRef inHandlerRef,
                                      EventRef inEvent, void *userData );
                        
    static OSStatus InkHandler( EventHandlerCallRef inCallRef,
                                EventRef inEvent, void* inUserData );
    
    virtual OSStatus ProcessHICommand( const HICommand& cmd );
    virtual OSStatus HandleInkTextEvent( EventHandlerCallRef inCallRef, EventRef inEvent );
    virtual OSStatus HandleGestureEvent( EventHandlerCallRef inCallRef, EventRef inEvent );
    
    HIViewRef fParentInstance; // this is the userpane control in the window that
                               // specifies our bounds

    WindowRef fHIViewWindow;
    WindowRef fWindowRef;
    
    EventHandlerRef fMenuEventHandler;
    EventHandlerRef fTextInputHandler;
    EventHandlerRef fInkEventHandler;
    
    Boolean fAnnotateInkInput; // put markup around ink text, to prove it came from ink handler
    
private:
    TXNObject fMLTEObj;
};

#endif // InkSample_INKTEXTPANE