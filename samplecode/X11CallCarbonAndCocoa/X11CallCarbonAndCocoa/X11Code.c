/*
	File:		CFCode.h
	
	Description:	This file contains all the X11 source which is used to put up a GUI and then call Carbon
                        Cocoa and Core Foundation.
                        
	Author:		Chad Jones 

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
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
*/

#include "X11Code.h"
#include "CFCode.h"
#include "CarbonCode.h"
#include "CocoaCode.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <stdlib.h>
#include <stdio.h>

XtAppContext 	gApplicationContext;
Display* 	gXWindowsDisplay = NULL;
Widget	 	gX11MainWindow;
const char* 	gApplicationName = "DTSX11CallCarbonCocoaApplication";
const char* 	gApplicationClass = "DTSX11CallCarbonCocoaApplicationClass";
Widget		gContentWindow, gWindowHeaderLabel, gClickButtonMessage, gBeepFromCarbonButton, gBeepFromCocoaButton;
Widget 		gDialogFromCarbonButton, gDialogFromCFButton, gDialogFromCocoaButton, gQuitButton;

int InitializeX11(int Argc, char* Argv[])
{
    const char* defaultXWindowsDisplayConnectionString = ":0.0";

    //Initalizing X toolkit.
    XtToolkitInitialize();
    
    //Getting the application context.
    gApplicationContext = XtCreateApplicationContext();

    /* Here we are opening a display within X11.  Note this actually estabilishes our connection with the
     * X11.  
     * First Argument: Application context obtained from a previous call of XtCreateApplicationContext
     * Second Argument: The display connection string.  This string is used in connecting to the display.
     *    Here we pass null since we want to use the display which is default for system (usually defined
     *    in the DISPLAY envionment variable.
     * Third Argument: The application name to be used for this applicaiton.  Here is this the 
     *    string we defined.  Note the application name is the name of the instance of the application.
     * Forth Argument: The application class to be used for this application.  Note the application class
     *    is the name for the generic class for this type of application.  
     * Fifth argument: Options to use when opening the display.  Here we have none so pass NULL.
     * Sixth Argument: The number of options contained in the fifth argument.  Here we have no arguments
     *    so pass NULL.
     * Seventh Argument: argc of the application we are running within (passed to us from main).
     * Eight Argument: argv of the application we are running within (passed to us from main).
     */
    gXWindowsDisplay = XtOpenDisplay(gApplicationContext, NULL, gApplicationName, 
                                    gApplicationClass, NULL, 0, &Argc, Argv);
                         
    //On error XtOpenDisplay will return NULL.  if that occurs then we will retry with the 
    //default display connection string on MacOSX ":0.0".  If this doesn't work then we just give up compeltely
    //and return NULL indicating error.
    if (gXWindowsDisplay == NULL)
    {
        printf("First try was unable to connect to X11.  Attempting connection again with display string: \":0.0\"\n");
        fflush(stdout);
        /* Redoing earlier call but this time with our default display connection string.  Hopefully if X11 is running
         * this call will be successful.  If unsuccessful its likely because X11 isn't running at all.
         */
        gXWindowsDisplay = XtOpenDisplay(gApplicationContext, defaultXWindowsDisplayConnectionString, 
                                        gApplicationName, gApplicationClass, NULL, 0, 
                                        &Argc, Argv);
    }

    //If still can't get dislay then give up here
    if (gXWindowsDisplay == NULL) 
    {
        XtDestroyApplicationContext (gApplicationContext); //getting rid of the application context.
        printf("Unable to connect to X11 on second attempt.  Giving up and quitting...\n");fflush(stdout);
        return(1); //failed return
    }
    else //successfully got XWindows display
    {
        /* Creating the shell which will hold our main window
         * First Argument: The name of the application which 
         * First Argument: The application name to be used for this applicaiton.  Here is this the 
         *    string we defined.  Note the application name is the name of the instance of the application.
         * Second Argument: The application class to be used for this application.  Note the application class
         *    is the name for the generic class for this type of application.  
         * Third Argument: The type of shell we are creating.  Here we want an appliaction so pass applicationShellWidgetClass.
         * Forth Argument: The display we are using.  Here we pass the XWindows display we just obtained.
         * Fifth Argument: Additional Arguments to specify resources of the shell.  Here we don't want to specify additional
         *    resources so pass NULL
         * Sixth Argument: The number of arguments passed in Fifth argument.  We pass no arguments so pass NULL.
         * Return Value: The main window for our X11 application.
         */
        gX11MainWindow = XtAppCreateShell(gApplicationName, gApplicationClass, 
                                    applicationShellWidgetClass,
                                    gXWindowsDisplay, NULL, 0);
    }
    
    return(0); //if got this far were successful
}

// The callback function for when we want to produce a system beep from carbon. 
void BeepFromCarbonCallBack (Widget w, XtPointer client_data, XtPointer call_data)
{
    //Calling the carbon native function.
    BeepFromCarbon();
}

// The callback function for when we want to display a dialog from Carbon 
void DialogFromCarbonCallBack (Widget w, XtPointer client_data, XtPointer call_data)
{
    /* Calling the carbon native function.  Here we are even passing the strings that will be
     * displayed in the dialog
     */
    DisplayDialogFromCarbon("This is Carbon saying Hello X11!", "HELLO!");
}

// The callback function for when we want to display a dialog from Core Foundation 
void DialogFromCFCallBack (Widget w, XtPointer client_data, XtPointer call_data)
{
    /* Calling the CoreFoundation native function.  Here we are even passing the strings that will be
     * displayed in the dialog
     */
    DisplayCoreFoundationDialog("This is Core Foundation saying Hello X11!", "HELLO!");
}

// The callback function for when we want to produce a system beep from cocoa. 
void BeepFromCocoaCallBack (Widget w, XtPointer client_data, XtPointer call_data)
{
    //Calling the cocoa native function.
    BeepFromCocoa();
}

// The callback function for when we want to display a dialog from Cocoa 
void DialogFromCocoaCallBack (Widget w, XtPointer client_data, XtPointer call_data)
{
    /* Calling the Cocoa native function.  Here we are even passing the strings that will be
     * displayed in the dialog
     */
    DisplayDialogFromCocoa("This is Cocoa saying Hello X11!", "HELLO!");
}

// The callback function for when we want to quit X11
void QuitX11Callback (Widget w, XtPointer client_data, XtPointer call_data)
{
    /* Quitting the X11 event loop here.  When we return from this callback we will return to whoever
     * started the X11 event loop.  In this case we will return to main() for quitting the application
     */
    XtAppSetExitFlag(gApplicationContext);

    //Remove all callbacks on the buttons here.
    
    XtRemoveCallback (gBeepFromCarbonButton, XtNcallback, BeepFromCarbonCallBack, NULL);
    XtRemoveCallback (gDialogFromCarbonButton, XtNcallback, DialogFromCarbonCallBack, NULL);
    XtRemoveCallback (gDialogFromCFButton, XtNcallback, DialogFromCFCallBack, NULL);
    XtRemoveCallback (gDialogFromCocoaButton, XtNcallback, DialogFromCocoaCallBack, NULL);
    XtRemoveCallback (gQuitButton, XtNcallback, QuitX11Callback, NULL);
    
    /* destroy all widgets here.
     * First argument: The widget to destroy
     */
    XtDestroyWidget(gWindowHeaderLabel);
    XtDestroyWidget(gClickButtonMessage);
    XtDestroyWidget(gBeepFromCarbonButton);
    XtDestroyWidget(gDialogFromCarbonButton);
    XtDestroyWidget(gDialogFromCFButton);
    XtDestroyWidget(gDialogFromCocoaButton);
    XtDestroyWidget(gQuitButton);
    XtDestroyWidget(gContentWindow);

    //Closing the X11 display
    XtCloseDisplay(gXWindowsDisplay);

    //To cleanup X11 we need to destroy the application context.
    XtDestroyApplicationContext (gApplicationContext);
    
    printf("\nX11 Application is Quitting\n");fflush(stdout);
    
}

int DisplayInitialX11GUI()
{
    XFontStruct* headerLabelFont;
    XFontStruct* regularLabelFont;
    XFontStruct* buttonFont;

    /* Here we are loading the fonts for use in the window we will display.  We actually
     * load three different fonts here.  Each font loading call uses the following description:
     * First Argument: The display we are connected to.  This is the XWindows display we found
     *    when initalizing X11.
     * Second Argument: The name of the font to load.  Here we use a named font
     *    which will suit our purposes.  Note you can get a list of fonts by typing
     *    xlsfonts at the XWindows terminal.
     * Return Value: The font structure which is used to describe the font.  
     *    On failure this return value will be NULL.
     */
    headerLabelFont = XLoadQueryFont(gXWindowsDisplay, "r24");
    regularLabelFont = XLoadQueryFont(gXWindowsDisplay, "r14");
    buttonFont = XLoadQueryFont(gXWindowsDisplay, "9x15");

    if ((headerLabelFont == NULL) || (regularLabelFont == NULL) || (buttonFont == NULL))
    {	
        //can't load a font so give up
        return(1);
    }

    /* Here we are creating the box widget which will hold all other
     * items in the window.  
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want the box class
     * Third Argument: The parent widget.  Here the parent is the topmost
     *    main window of the program
     * Forth Argument: NULL indicating we don't need to set any additional resources.
     * Return Value: The widget representing the box which was created.
     */
    gContentWindow = XtVaCreateManagedWidget("gContentWindow", boxWidgetClass, gX11MainWindow,NULL);

    /* Now creating a label which uses the font we just loaded.  This label will
     * be the header on our window.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a label
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the label to have.  In this case the label font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the label and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the label to have.  In this case a message describing the application.
     * Eigth Argument: XtNborderWidth indicating we want to set the width of the border around the label and the
     *    width is in the next argument.
     * Ninth Argument: Width of the border.  Here we pass zero because we don't want a visible border.
     * Tenth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the label which was created.
     */
    gWindowHeaderLabel = XtVaCreateManagedWidget ("gWindowHeaderLabel", labelWidgetClass, gContentWindow, 
                                                    XtNfont, headerLabelFont, XtNlabel, 
                                                    "X11 Call Carbon and Cocoa",  
                                                    XtNborderWidth, 0, NULL);

    /* Now creating a label which will be used to indicate that the user should click a button below.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a label
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the label to have.  In this case the label font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the label and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the label to have.  In this case a message telling the user what to do.
     * Eigth Argument: XtNborderWidth indicating we want to set the width of the border around the label and the
     *    width is in the next argument.
     * Ninth Argument: Width of the border.  Here we pass zero because we don't want a visible border.
     * Tenth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the label which was created.
     */
    gClickButtonMessage = XtVaCreateManagedWidget ("gClickButtonMessage", labelWidgetClass, gContentWindow, 
                                                    XtNfont, regularLabelFont, XtNlabel, 
                                                    "Click a button below to perform a native MacOSX call:",  
                                                    XtNborderWidth, 0, NULL);

    /* Now we are providing buttons to make MacOSX native calls.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a command widget (button)
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the button to have.  In this case the button font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the button and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the button to have.  In this case a message describing the action.
     * Eigth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the box which was created.
     */

    gDialogFromCarbonButton = XtVaCreateManagedWidget("gDialogFromCarbonButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Put up standard dialog from Carbon", NULL);

    /* Now we are providing buttons to make MacOSX native calls.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a command widget (button)
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the button to have.  In this case the button font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the button and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the button to have.  In this case a message describing the action.
     * Eigth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the box which was created.
     */

    gBeepFromCarbonButton = XtVaCreateManagedWidget("gBeepFromCarbonButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Produce a system beep from Carbon", NULL);

    /* Here we are setting the button's behavior that is what routine the button will call.
     * First Argument: The button we are editing the behavior of.  In this case the button we just made.
     * Second Argument: XtNcallback indicating that we want to add this to our default list of button callbacks.
     * Third Argument: Additional data we want to pass the callback.  Here we will provide no extra data
     *    so just pass NULL
     */
    XtAddCallback (gBeepFromCarbonButton, XtNcallback, BeepFromCarbonCallBack, NULL);

    /* Here we are setting the button's behavior that is what routine the button will call.
     * First Argument: The button we are editing the behavior of.  In this case the button we just made.
     * Second Argument: XtNcallback indicating that we want to add this to our default list of button callbacks.
     * Third Argument: Additional data we want to pass the callback.  Here we will provide no extra data
     *    so just pass NULL
     */
    XtAddCallback (gDialogFromCarbonButton, XtNcallback, DialogFromCarbonCallBack, NULL);

    /* Now we are providing buttons to make MacOSX native calls.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a command widget (button)
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the button to have.  In this case the button font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the button and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the button to have.  In this case a message describing the action.
     * Eigth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the box which was created.
     */

    gDialogFromCFButton = XtVaCreateManagedWidget("gDialogFromCFButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Put up standard dialog from Core Foundation", 
                                                        NULL);

    /* Here we are setting the button's behavior that is what routine the button will call.
     * First Argument: The button we are editing the behavior of.  In this case the button we just made.
     * Second Argument: XtNcallback indicating that we want to add this to our default list of button callbacks.
     * Third Argument: Additional data we want to pass the callback.  Here we will provide no extra data
     *    so just pass NULL
     */
    XtAddCallback (gDialogFromCFButton, XtNcallback, DialogFromCFCallBack, NULL);


    //Adding button to show dialog from Cocoa.  Using same method as above.
    gDialogFromCocoaButton = XtVaCreateManagedWidget("gDialogFromCocoaButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Put up standard dialog from Cocoa", 
                                                        NULL);

    //Adding callback to dialog button.  Using same method as above.
    XtAddCallback (gDialogFromCocoaButton, XtNcallback, DialogFromCocoaCallBack, NULL);

    /* Now we are providing buttons to make MacOSX native calls.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a command widget (button)
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the button to have.  In this case the button font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the button and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the button to have.  In this case a message describing the action.
     * Eigth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the box which was created.
     */

    gBeepFromCocoaButton = XtVaCreateManagedWidget("gBeepFromCocoaButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Produce a system beep from Cocoa", NULL);

    /* Here we are setting the button's behavior that is what routine the button will call.
     * First Argument: The button we are editing the behavior of.  In this case the button we just made.
     * Second Argument: XtNcallback indicating that we want to add this to our default list of button callbacks.
     * Third Argument: Additional data we want to pass the callback.  Here we will provide no extra data
     *    so just pass NULL
     */
    XtAddCallback (gBeepFromCocoaButton, XtNcallback, BeepFromCocoaCallBack, NULL);

    /* Now we are providing quit button.
     * First Argument: Name of the widget (must be unique within the program).  
     *    Here we use the same as the variable name
     * Second Argument: The class of the widget.  Here we want a command widget (button)
     * Third Argument: The parent widget.  Here the parent is the box we
     *    created which will hold all elements of this window
     * Forth Argument: XtNfont indicating the font we want to use is described in the next argument
     * Fifth Argument: The font we want the button to have.  In this case the button font we just loaded.
     * Sixth Argument: XtNlabel indicating we want to edit the text of the button and the text is
     *    in the next argument.
     * Seventh Argument: The title we want the button to have.  In this case a message describing the action.
     * Eigth Argument: NULL indicating we are done specifying additional resources for the widget.
     * Return Value: The widget representing the box which was created.
     */

    gQuitButton = XtVaCreateManagedWidget("gQuitButton", commandWidgetClass, 
                                                        gContentWindow, XtNfont, buttonFont, XtNlabel, 
                                                        "Quit Application", 
                                                        NULL);

    /* Here we are setting the button's behavior that is what routine the button will call.
     * First Argument: The button we are editing the behavior of.  In this case the button we just made.
     * Second Argument: XtNcallback indicating that we want to add this to our default list of button callbacks.
     * Third Argument: Additional data we want to pass the callback.  Here we will provide no extra data
     *    so just pass NULL
     */
    XtAddCallback (gQuitButton, XtNcallback, QuitX11Callback, NULL);

    // Display the window. 
    XtRealizeWidget(gX11MainWindow);
    
    return(0); //if got this far were successful
}

void StartX11EventLoop()
{
    // Event loop, waiting for events to happen. 
    XtAppMainLoop (gApplicationContext);
}
