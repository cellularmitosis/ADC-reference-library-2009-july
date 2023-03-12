/*
 palmJava.java
 ScripBuildPhases sample code

 Description: 
 
 This sample shows how to augment Project Builder build phases with scripts of your own, using build settings variables that will be expanded at compiletime, to do extra work and invoke other tools for processing.  In this particular sample, a java applet is compiled by Project Builder, and then a script in a shell script build phase does some post-processing to generate Palm OS executables that will run under the Superwaba handheld Java VM (obtainable from <http://www.superwaba.org> - download the SDK package). 
 
 This file provides the java source code that will have further post-processing
 performed on it after normal java compilation in Project Builder via the postprocessing.sh script
 and a shell script build phase (see the Build Phases section of the Targets pane). Any extra
 compilation or work you want done on files can be done in a shell script build phase, and such
 build phases don't have to go at the end of the build process like the one does here.  This gives
 you the developer a tremendous amount of power and flexibility! This particular java
 source code file is meant to run from the open source superwaba PalmOS/handheld device Java VM - see <http://www.superwaba.org> for download info (get the complete SDK, not just the VM).  The shell script expects that the superwaba folder/executables/classes will be placed at the root level ("/") of your boot volume, but you could edit the script to look for superwaba somewhere else.

 Author: MCF

 Copyright (c) 2002, Apple Computer, Inc., all rights reserved.
 */
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


// make sure that the right classes are available
import waba.fx.*;
import waba.sys.*;
import waba.util.*;
import waba.ui.*;

// extend the MainWindow class to add our custom content
public class palmJava extends MainWindow
{
    MenuBar myMenuBar;
    Button testButton;
    Label myLabel;

    // in SuperWaba, menus are arrays of Strings, and when processing events, each menu item 
    // automatically has its ID number be arrayIndex+100*zeroBasedMenuNumber. So for menu1,
    // "Bar" which is the second item of the second
    // menu would be 1+100*1=101.  The zeroth element of each menu is the menu's title.
    String menu0[] =
    {
        "Main menu",
        "About palmJava",
        "Preferences",
        "-",
        "Quit",
    };
    String menu1[] =
    {
        "Some menu",
        "Foo",
        "Bar",
    };
    

    // initialize the main window with a title, etc. and turn on double buffering for smoother
    // graphics
    public palmJava()
    {
        super("palmJava",TAB_ONLY_BORDER);
        setDoubleBuffer(true);
    }

    public void onStart()
    {
        // At launch, we want to create a new menubar.  So we do so here, using the string arrays
        // we created above as the menus.
        setMenuBar(myMenuBar = new MenuBar(new String[][]{menu0,menu1}));

        // Create and add a new button
        testButton = new Button("This is a button");
        testButton.setRect(5, 30, 150, 25);
        add(testButton);

        // Add the current date to the window
        Time t = new Time();
        myLabel = new Label("Today is "+t.month+"/"+t.day+"/"+t.year);
        myLabel.setRect(5,70,150,25);
        add(myLabel);
    }

    // Called by the system to pass events to the application.
    public void onEvent(Event event)
    {
        if (event.type == ControlEvent.WINDOW_CLOSED)
        {
            if (event.target == myMenuBar)
            {
                int item = myMenuBar.getSelectedMenuItem();
                // Change the text of our label in the window based on whatever
                // menu item was selected.
                if (item<100)
                    myLabel.setText("menu item: "+menu0[item]+" which is "+item);
                else
                    myLabel.setText("menu item: "+menu1[item-100]+" which is "+item);
            }
        }
    }

}