/*
	File:		main.c

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include "globals.h"
#include "window.h"
#include "print.h"
#include "fontmenu.h"
#include "atsui.h"
#include "main.h"


// Main entry point.  Sets things up, then runs the event loop
//
int main(int argc, char* argv[])
{
    char								startingFontName[] = "Geneva";
    int									startingFontSize = 48;
    ATSUFontID                          font;
    OSStatus                            err = noErr;

    // Set up the menubar and main window
    err = SetupMenuAndWindow();
    require_noerr( err, CantDoSetup );
    
    // Make sure we are using the right CG and ATSUI calls
    CheckAPIAvailability();

    // ********** Create the ATSUI data and draw it for the first time **********
    //
    verify_noerr( ATSUFindFontFromName(startingFontName, strlen(startingFontName), kFontFullName, kFontNoPlatform, kFontNoScript, kFontNoLanguage, &font) );
    verify( FindAndSelectFont(font) );
    SetATSUIStuffFont(font);
    SetATSUIStuffFontSize(Long2Fix(startingFontSize));
    SetUpATSUIStuff();
    DrawATSUIStuff(GetWindowPort(gWindow));

    // Call the event loop
    RunApplicationEventLoop();

CantDoSetup:
    return err;
}


// Checks to see if:
//    - QDBeginCGContext / QDEndCGContext are available (vs. CreateCGContextForPort)
//    - ATSUI Direct Access APIs are available (vs. GlyphInfoArray APIs)
//
// Sets global flags to indicate the result.
//
void CheckAPIAvailability(void)
{
    long response;

    // These globals were already initialized to false in globals.c,
    // so we only need to change their value in the true case.

    // The QDBeginCGContext / QDEndCGContext APIs were introduced in 0x0310 (Quickdraw version from Mac OS X 10.1)
    verify_noerr( Gestalt(gestaltQuickdrawVersion, &response) );
    if ( (response & 0x0000FFFF) >= 0x00000310 ) gNewCG = true;

    // The Direct Access ATSUI APIs have a constant that can be used with the ATSUI features gestalt selector
    verify_noerr( Gestalt(gestaltATSUFeatures, &response) );
    if ( response & gestaltATSUDirectAccess ) gHaveDirectAccess = true;
    
    // NOTE
    // ----
    // Never check the overall system version for a specific feature.  Always check the specific
    // gestalt selector for that component.  gestaltSystemVersion is not for specific APIs.
}


// Creates the menu bar and window, then installs proper event handlers for them
//
OSStatus SetupMenuAndWindow(void)
{
    IBNibRef                            nibRef;
    EventHandlerUPP                     handlerUPP;
    EventTypeSpec                       eventType;
    OSStatus                            err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );

    // Install the font menu
    err = InstallFontMenu(kFontMenuID);
    require_noerr( err, CantCreateFontMenu );

    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gWindow);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // The window was created hidden, so show it.
    ShowWindow(gWindow);

    // Install a handler to quit the application when the window is closed
    eventType.eventClass = kEventClassWindow;
    eventType.eventKind  = kEventWindowClose;
    handlerUPP = NewEventHandlerUPP(DoWindowClose);			// DoWindowClose() is defined in window.c
    verify_noerr( InstallWindowEventHandler(gWindow, handlerUPP, 1, &eventType, NULL, NULL) );

    // Install a handler to draw the contents of the window
    eventType.eventClass = kEventClassWindow;
    eventType.eventKind  = kEventWindowBoundsChanged;
    handlerUPP = NewEventHandlerUPP(DoWindowBoundsChanged);	// DoWindowBoundsChanged() is defined in window.c
    verify_noerr( InstallWindowEventHandler(gWindow, handlerUPP, 1, &eventType, NULL, NULL) );

    // Install a handler for command events
    eventType.eventClass = kEventClassCommand;
    eventType.eventKind  = kEventCommandProcess;
    handlerUPP = NewEventHandlerUPP(DoCommandEvent);		// DoCommandEvent() is defined below
    verify_noerr( InstallApplicationEventHandler(NewEventHandlerUPP(DoCommandEvent), 1, &eventType, NULL, NULL) );

    // Set up the global print session
    verify_noerr( InitializePrinting() );

CantCreateWindow:
CantCreateFontMenu:
CantSetMenuBar:
CantGetNibRef:
        return err;
}

// Handles command and menu events
//
pascal OSStatus DoCommandEvent(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData)
{
    HICommand               theCommand;
    UInt32                  theCommandID;
    MenuRef                 theMenu;
    MenuItemIndex           theItem;
    FMFont                  font;
    OSStatus                status = eventNotHandledErr;
    Boolean                 needsRedrawing = false;

    
    // Get the HICommand from the event structure, then get the menu reference and item out of that
    verify_noerr( GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &theCommand) );
    theCommandID = theCommand.commandID;
    theMenu = theCommand.menu.menuRef;
    theItem = theCommand.menu.menuItemIndex;

    if ( GetMenuID(theMenu) >= kFontMenuID ) {			// Handle font menu
        font = SelectAndGetFont(theMenu, theItem);
        SetATSUIStuffFont(font);
        UpdateATSUIStyle();

        needsRedrawing = true;
        status = noErr;
    }
    else if ( (theCommandID >> 24) == 'Z' ) {			// Handle font size menu
        UInt32      numericalBits;
        char        string[5];
        
        // The last three bytes of the CommandID are an ASCII representation of the font size
        numericalBits = theCommandID & 0x00FFFFFF;
        memcpy(string, &numericalBits, sizeof(UInt32));
        string[0] = '0';
        string[4] = 0x00;
        SetATSUIStuffFontSize(Long2Fix(atoi(string)));
        UpdateATSUIStyle();

        // Update the menu
        verify_noerr( SetMenuCommandMark(NULL, gCurrentFontSizeCommandID, kMenuNoMark) );
        verify_noerr( SetMenuCommandMark(NULL, theCommandID, kMenuCheckMark) );
        gCurrentFontSizeCommandID = theCommandID;

        needsRedrawing = true;
        status = noErr;
    }
    else switch (theCommandID) {						// Handle other menu commands
        case 'OPT1':
            gUseCG = ( ! gUseCG );
            CheckMenuItem(theMenu, theItem, gUseCG);
            if ( gUseCG ) {
                DisableMenuCommand(NULL, 'OPT2'); // Disable AnimateQDSegments option if CG is used
            }
            else {
                EnableMenuCommand(NULL, 'OPT2');  // Enable AnimateQDSegments option if CG is not used
            }
            status = noErr;
            needsRedrawing = true;
            break;

        case 'OPT2':
            gAnimateQDSegments = ( ! gAnimateQDSegments );
            CheckMenuItem(theMenu, theItem, gAnimateQDSegments);
            status = noErr;
            needsRedrawing = true;
            break;

        case 'OPT3':
            gFilterDegenerates = ( ! gFilterDegenerates );
            CheckMenuItem(theMenu, theItem, gFilterDegenerates);
            status = noErr;
            needsRedrawing = true;
            break;

        case 'DBG1':
            gOutputCurveType = ( ! gOutputCurveType );
            CheckMenuItem(theMenu, theItem, gOutputCurveType);
            status = noErr;
            needsRedrawing = true;
            break;

        case 'DBG2':
            gOutputNumSegments = ( ! gOutputNumSegments );
            CheckMenuItem(theMenu, theItem, gOutputNumSegments);
            status = noErr;
            needsRedrawing = true;
            break;

        case 'DBG3':
            gOutputNumOps = ( ! gOutputNumOps );
            CheckMenuItem(theMenu, theItem, gOutputNumOps);
            status = noErr;
            needsRedrawing = true;
            break;

        case kHICommandPageSetup:
            verify_noerr( DoPageSetupDialog() );
            status = noErr;
            needsRedrawing = true;
            break;

        case kHICommandPrint:
            status = DoPrintDialog();
            if ( status == noErr ) {
                DoPrintLoop();
            }
            else if ( status != kPMCancel ) {
                check_string( (status == noErr), "DoPrintDialog returned an error" );
            }
            status = noErr;
            needsRedrawing = true;
            break;
    }

    // Redraw if necessary
    if (needsRedrawing) DrawATSUIStuff(GetWindowPort(gWindow));
    return status;
}