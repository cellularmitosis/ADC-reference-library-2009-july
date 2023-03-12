/*
	File:		main.c
	
	Contains:	Carbon application code to call through to the Cocoa NSSpelling routines.
				The code calls the Cocoa NSSpelling wrapper functions.

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

	Copyright © 2003 - 2004 Apple Computer, Inc., All Rights Reserved
*/


#include <Carbon/Carbon.h>
#include "SpellCheck.h"

//	Additional info associated with each window stored off the refCon
struct WindowinfoStruct
{
	CFRange		range;
};
typedef struct WindowinfoStruct WindowinfoStruct;


static pascal OSStatus MainWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData );
static void	SetMisspelledWord( WindowRef window, CFStringRef stringToSpellCheck, CFRange *range );
static CFStringRef	GetControlItemTextFromControlID( WindowRef window, OSType signature, SInt32 id );
static pascal OSStatus MySimpleDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue );
static pascal void MySimpleNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message );
static void	ReplaceMisspelledWord( WindowRef window, CFStringRef newWord );
static void	ContinueChecking( WindowRef window );


int main( int argc, char* argv[] )
{
    IBNibRef 				nibRef;
    WindowRef 				window;
    OSStatus				err;
    DataBrowserCallbacks	dbCallbacks;
    ControlRef				control;
    WindowinfoStruct		*windowInfo;
    ControlID				controlID		= { 'Brow', 0 };
    static const EventTypeSpec	windowEvents[] =
    {
		{ kEventClassCommand, kEventCommandProcess },
		{ kEventClassWindow, kEventWindowClose }
    };

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    InitializeCocoa();

    //	We pass "window" as the userData to make it available from our event handler
    err	= InstallWindowEventHandler( window, NewEventHandlerUPP( MainWindowEventHandlerProc ), GetEventTypeCount(windowEvents), windowEvents, window, NULL );

    GetControlByID( window, &controlID, &control );
    dbCallbacks.version	= kDataBrowserLatestCallbacks;
    err	= InitDataBrowserCallbacks( &dbCallbacks );
    dbCallbacks.u.v1.itemDataCallback	= NewDataBrowserItemDataUPP( MySimpleDataCallback );
	dbCallbacks.u.v1.itemNotificationCallback	= NewDataBrowserItemNotificationUPP( MySimpleNotificationCallback );
    err	= SetDataBrowserCallbacks( control, &dbCallbacks );

    windowInfo	= (WindowinfoStruct*)NewPtrClear( sizeof(WindowinfoStruct) );
    SetWRefCon( window, (UInt32)windowInfo );
    
    ShowWindow( window );				// The window was created hidden so show it.
    
    RunApplicationEventLoop();			// Call the event loop

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return( err );
}



static pascal OSStatus MainWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData )
{
    HICommand			command;
    ControlRef			control;
    OSErr				err;
    Size				count;
    CFStringRef			stringToSpellCheck;
    CFStringRef			wordInQuestion;
    ControlID			controlID		= { 'Text', 0 };
    WindowRef			window			= (WindowRef) inUserData;
    UInt32				eventClass		= GetEventClass( inEvent );
    UInt32				eventKind		= GetEventKind( inEvent );
    WindowinfoStruct	*windowInfo		= (WindowinfoStruct*) GetWRefCon( window );
	
    switch ( eventClass )
    {
        case kEventClassCommand:
            if ( eventKind == kEventCommandProcess )
            {
                GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
                
                if ( command.commandID == 'Spel' )			//	Check Spelling Button was clicked
                {
                    GetControlByID( window, &controlID, &control );
                    err	= GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &stringToSpellCheck, &count );
                    if ( err == noErr )
                    {
						windowInfo->range	= CheckSpellingOfString( stringToSpellCheck, 0 );
						
						if ( windowInfo->range.length > 0 )
							SetMisspelledWord( window, stringToSpellCheck, &windowInfo->range );
						else
							windowInfo->range.location	= 0;
                    }
                }
                else if ( command.commandID == 'Ignr' )		//	Ignore Button was clicked
                {
					wordInQuestion	= GetControlItemTextFromControlID( window, 'Miss', 0 );
					IgnoreWord( wordInQuestion, 0 );
					CFRelease( wordInQuestion );
                    
                    ContinueChecking( window );
                }
                else if ( command.commandID == 'Rplc' )		//	Replace Button was clicked
                {
                    wordInQuestion	= GetControlItemTextFromControlID( window, 'NwWd', 0 );
                    ReplaceMisspelledWord( window, wordInQuestion );
                    ContinueChecking( window );
                }
            }
            break;
        
        case kEventClassWindow:
            if ( eventKind == kEventWindowClose )
            {
            }
            break;
    }
    
    return( eventNotHandledErr );			//	Use default handlers for things like kHICommandBringAllToFront
}


static void	ContinueChecking( WindowRef window )
{
    ControlRef			control;
    Size				count;
    CFStringRef			stringToSpellCheck;
    OSErr				err;
    ControlID			controlID		= { 'Text', 0 };
    WindowinfoStruct	*windowInfo		= (WindowinfoStruct*) GetWRefCon( window );
    
    GetControlByID( window, &controlID, &control );
    err	= GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &stringToSpellCheck, &count );
    if ( err == noErr )
    {
        int	lastLocation	= windowInfo->range.location;
        windowInfo->range	= CheckSpellingOfString( stringToSpellCheck, windowInfo->range.location + windowInfo->range.length );
        
        if ( (windowInfo->range.location > lastLocation) && (windowInfo->range.location < 32*1024) )
        {
            SetMisspelledWord( window, stringToSpellCheck, &windowInfo->range );
        }
        else
        {
            windowInfo->range.location	= windowInfo->range.length	= 0;
            SetMisspelledWord( window, stringToSpellCheck, &windowInfo->range );
        }
    }
}


//	MySimpleDataCallback is a simple data callback routine	that would be used in a browser control displaying a list
//	of generic document icons together with a list of checkboxes. The checkbox column is a mutable column.
static pascal OSStatus MySimpleDataCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue )
{
    OSStatus	err;
    CFStringRef	guess;
    CFArrayRef	array	= (CFArrayRef) GetControlReference( browser );

	//	Look at the property ID and decide what to do depending on what column we're dealing with...
    if ( property == 'Gues' )
    {
        guess	= CFArrayGetValueAtIndex( array, itemID-1 );

        err	= SetDataBrowserItemDataText( itemData, guess );
        if ( err != noErr ) goto Bail;
    }
    else
    {
        err	= errDataBrowserPropertyNotSupported;
        goto Bail;
    }

Bail:
    return( err );
}


static pascal void MySimpleNotificationCallback( ControlRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message )
{
    CFStringRef	guess;
    CFArrayRef	array		= (CFArrayRef) GetControlReference( browser );

    if ( message == kDataBrowserItemDoubleClicked )
    {
        guess	= CFArrayGetValueAtIndex( array, itemID-1 );

        //	Replace the misspelled word with the selection
        ReplaceMisspelledWord( GetControlOwner(browser), guess );
        (void) RemoveDataBrowserItems( browser, kDataBrowserNoItem, 0, NULL, kDataBrowserItemNoProperty );	//	Erase the guesses
        ContinueChecking( GetControlOwner(browser) );
    }
}


static void	SetMisspelledWord( WindowRef window, CFStringRef stringToSpellCheck, CFRange *range )
{
    CFStringRef			misspelledWord;
    static CFArrayRef	array;
    CFIndex				count;
    ControlRef			control;
    OSStatus			err;
    ControlID			controlID	= { 'Miss', 0 };
    
    misspelledWord	= CFStringCreateWithSubstring( kCFAllocatorDefault, stringToSpellCheck, *range );
    
    GetControlByID( window, &controlID, &control );
    (void) SetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &misspelledWord );
    Draw1Control( control );

    if ( CFStringGetLength(misspelledWord) > 0 )
    {
        controlID.signature	= 'Brow';
        GetControlByID( window, &controlID, &control );
    
        if ( array != NULL ) { CFRelease( (void*)array ); array = NULL; }
        array	= GuessesForWord( misspelledWord );
        count	= CFArrayGetCount( array );
        
        SetControlReference( control, (UInt32) array );
        err	= RemoveDataBrowserItems( control, kDataBrowserNoItem, 0, NULL, kDataBrowserItemNoProperty );
        AddDataBrowserItems( control, kDataBrowserNoItem, count, NULL, kDataBrowserItemNoProperty );
    }
    CFRelease( misspelledWord );
}

static CFStringRef	GetControlItemTextFromControlID( WindowRef window, OSType signature, SInt32 id )
{
    CFStringRef	misspelledWord;
    ControlRef	control;
    SInt32		dataSize;
    ControlID	controlID;
    
    controlID.signature	= signature;
    controlID.id		= id;

    GetControlByID( window, &controlID, &control );
    (void) GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), (Ptr)&misspelledWord, &dataSize );
    return( misspelledWord );
}


static void	ReplaceMisspelledWord( WindowRef window, CFStringRef newWord )
{
    ControlRef			control;
    OSStatus			err;
    Size				count;
    CFStringRef			theText;
    CFMutableStringRef	theNewText;
    CFRange				emptyRange		= { 0, 0 };
    ControlID			controlID		= { 'Text', 0 };
    WindowinfoStruct	*windowInfo		= (WindowinfoStruct*) GetWRefCon( window );
        
    GetControlByID( window, &controlID, &control );
    err	= GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &theText, &count );

    theNewText	= CFStringCreateMutableCopy( kCFAllocatorSystemDefault, 0, theText );
    CFStringReplace( theNewText, windowInfo->range, newWord );
    SetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), &theNewText );
    Draw1Control( control );
    CFRelease( theNewText );

    SetMisspelledWord( window, newWord, &emptyRange );

    windowInfo->range.length	= CFStringGetLength( newWord );
}








