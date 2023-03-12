File: ReadMe.txt

Abstract: readme file for the SBSetFinderComment sample.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2007 Apple Inc. All Rights Reserved.




Introduction

This file details the steps involved in putting together a project that uses Scripting Bridge to send Apple events to the Finder application for the purposes of setting and getting comments assigned to files and folders.  



1. Start with a new Cocoa application.

In this sample we're going to target the Finder application so we'll call our sample 'SBSetFinderComment'.  The techniques we are demonstrating in this sample can be used in any Cocoa Application.  We used a simple one window Cocoa application for this sample to keep it simple.  




2. Add a Scripting Bridge build rule to the project.

The first thing to do is to set up Xcode to automatically generate the Scripting Bridge source for the application you would like to target.  The following steps describe how you can do that:

(a) Turn down the "Targets" tab in the "Groups & Files" list on the left hand side and select the main target.  In this sample,  select the "SBSetFinderComment" target.  

(b) With the "SBSetFinderComment" target selected, open the information panel (by clicking on the Info icon, or by control- or right-clicking on the target name and selecting "Get Info" from the pop-up menu).

(c) In the info window, click on the Rules tab and then click on the + button at the bottom to add a new rule.

(d) Set up the new rule as follows:

	Process 'Source files with names matching:'   *.app
   
	using: 'Custom Script'
   
	set the script field to:
   
	sdef "$INPUT_FILE_PATH" | sdp -fh -o "$DERIVED_FILES_DIR" --basename "$INPUT_FILE_BASE" --bundleid `defaults read "$INPUT_FILE_PATH/Contents/Info" CFBundleIdentifier`

	click on the "+" icon below the 'with output files:' field, and then set the field to contain:
	
	$(DERIVED_FILES_DIR)/$(INPUT_FILE_BASE).h

	NOTE: if you're typing this rule in by hand, note that it should all be one one line, and it must be typed exactly as shown above.  If you have difficulty entering the above command, then copy and paste the command from the readme into the rule.

(e) All done.  You can close the info window.  Xcode is now set up to automatically generate Scripting Bridge source for any applications you add to your project.

NOTE: this rule uses the sdef and sdp command line tools.  To learn more about these tools, use the following commands in the Terminal window:
   man sdp
   man sdef




3. Select a target application.

To do this, drag and drop the application you would like to target into the project files group inside of the "Groups & Files" list in the project window.  

You can drop the application among the source files you are using for your application.  Because of the build rule we added in step 2, Xcode will treat the Finder application as if it were one of the source files during the build.  

You should uncheck the 'Copy items into destination group's folder (if needed)' option so the application is not copied into your project directory.  In this sample we have selected 'Absolute Path' as the reference type so we can easily move the project around from machine to machine without invalidating the reference (so long as the Finder application is present in the System/Library/CoreServices folder,  Xcode will be able to find it).

In this case, we are adding the Finder to our project.  The Finder application is located in the /System/Library/CoreServices folder.




4. Add the target application to the Compile Sources.

After you have added the target application to your project, you must also add it to the main target's Compile Sources.  You can do that by adding the application to the 'Compile Sources' build phase under the main target.  In the "Groups & Files" list, open the "Targets" and the "SBSetFinderComment" target  and the Compile Sources build phase items by clicking on the arrows to the left of them.  Then, drag the Finder.app from the project files group into the Compile Sources.




5. Add the Scripting Bridge framework to your project.

Turn down the "Frameworks" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu.  Then, add the "ScriptingBridge.framework" (/System/Library/Frameworks/ScriptingBridge.framework) to the project.  




6. Add a minimum system version Info.plist key.

Since the ScriptingBridge.framework is necessary for this application to run and that framework is not present on previous system versions, you should add the following key/value pair to the Info.plist file for the application.  If someone tries to run this application on a system earlier than Mac OS X 10.5, then they will receive a notice from launch services letting them know that the application is meant to be run on a later version of Mac OS X.

	<key>LSMinimumSystemVersion</key>
	<string>10.5</string>

You can edit the Info.plist file by either clicking on its icon in the resources section, or by clicking on the SBSetFinderComment target, clicking on the Info icon, selecting the Properties tab, and then clicking on "Open Info.plist as File".




7. Make sure the Target SDK is set to Mac OS X 10.5

You won't have to change this setting for this project, but if you are adding Scripting Bridge to another project that you started an earlier version of the Mac OS than Mac OS X 10.5, then you will need to update the Target SDK for the project.  In the Groups & Files view, select the SBSetFinderComment project icon at the top of the list and then open the information window.  Under the General tab, change the Cross-Develop Using Target SDK: setting to Mac OS X 10.5.




8. Build your project.

If you have followed the steps above, Xcode will generate the Scripting Bridge source for your project.  They will be put inside of your build folder in a place where the linker and compiler can find them.  

The build rule that we installed will create a .h and a .m file with the same name as the application.  For example, if you added Finder to our project, then the build rule will create Finder.h.  The files will be created inside of the build directory in the DerivedSources directory where the compiler can find them.

For the Debug build, the Finder.h file will be located in this sub folder of the build directory:
/build/SBSetFinderComment.build/Debug/SBSetFinderComment.build/DerivedSources/Finder.h

For the Release build, the Finder.h file will be located in this sub folder of the build directory:
/build/SBSetFinderComment.build/Release/SBSetFinderComment.build/DerivedSources/Finder.h

A convenient way for you to open and inspect these files is to use the 'Open Quickly' command in the file menu.  For most purposes, the .h file will contain most of the interesting information so to view that file you open the 'Finder.h' file.  

In some cases, depending on what frameworks are in your project, the 'Open Quickly' command may open the system's 'Finder.h' file that includes constants and definitions used by the file system and the Finder application.  If that happens for you, then you will need to navigate into the build folder to find the correct header file.




9. Add in the Finder's Scripting Bridge header.

In the file Controller.m, add '#import "Finder.h"' near the top of the file below '#import "Controller.h"'.  This will include all of the Scripting Bridge definitions for the Finder.

When you're done, the imports section should look like this:

	#import "Controller.h"
	#import "Finder.h"

In your own application, of course, you would import the Finder.h file in the file where you intend to call it from.  In this sample, we are using the Scripting Bridge interface inside of three methods in our Controller class so that is why we are importing it into Controller.m.




10. Roadmap - check out what the SBSetFinderComment sample is doing.

So, we've set up our sample and installed the basic parts.  Let's take a moment to look at the bigger picture so we can see where we're going and how Scripting Bridge fits into this sample.

In this sample we have have provided three methods that use Scripting Bridge to perform some commonly implemented tasks: (a) retrieving a file's Finder comment, (b) changing a file's Finder comment, and (c) revealing an item in the Finder.

We'll discuss each of these items in the sections below.




11. Retrieving a file's Finder Comment (aka, Spotlight Comment).

The method -finderCommentForFileURL: in the Controller class uses Scripting Bridge to retrieve the Finder Comment for an item referred to by the file url provided as a parameter.  As you can see, it's a very simple method:


- (NSString*) finderCommentForFileURL:(NSURL*) theFileURL {
    NSString* result;

    @try {
            /* retrieve the Finder application Scripting Bridge object. */
        FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
        
            /* retrieve a reference to our finder item asking for it by location */
        FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];
        
            /* set the result.  */
        result = theItem.comment;
    }
    @catch(NSException *e) {
        result = nil;
    }
    
        /* return the comment (or nil on error). */
    return result;
}

Interesting items to note here are:

(a) we use Objective-C exceptions for error handling.  If you need specialized error handling, the SBApplication class allows you to set a delegate object that implements a -eventDidFail:withError: delegate method.  The default simply @throws an error and we interpret all errors as failures in this method.

(b) we use the +applicationWithBundleIdentifier: SBApplication class method to create our Application object.  This will dynamically load the application object based on the dictionary inside of the application itself rather than building the application class from statically compiled information.

(c) the 'comment' property of the FinderItem is implemented as an Objective-C 2.0 property.  Most of the work involved with retrieving the comment including the Apple events required to retrieve the information happens inside of the property getter defined for this property.






12.  Changing a file's Finder Comment (aka, Spotlight Comment).

The method -changeFinderComment:forFileURL: in the Controller class uses Scripting Bridge to set the Finder Comment for an item referred to by the file url provided as a parameter.  As you can see, it is also a very simple method:


- (BOOL) changeFinderComment:(NSString*) comment forFileURL:(NSURL*) theFileURL {
    BOOL result;

    @try {
            /* retrieve the Finder application Scripting Bridge object. */
        FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
                
            /* retrieve a reference to our finder item asking for it by location */
        FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];

            /* attempt to set the comment for the Finder item.  */
        theItem.comment = comment;
        
            /* successful result */
        result = YES;
    }
    @catch(NSException *e) {
        result = NO;
    }

        /* return YES on success */
    return result;
}


Interesting items to note here are:

(a) Again, we use Objective-C exceptions for error handling.  If you need specialized error handling, the SBApplication class allows you to set a delegate object that implements a -eventDidFail:withError: delegate method.  The default simply @throws an error and we interpret all errors as failures in this method.

(b) we use the +applicationWithBundleIdentifier: SBApplication class method to create our Application object.  This will dynamically load the application object based on the dictionary inside of the application itself rather than building the application class from statically compiled information.

(c) the 'comment' property of the FinderItem is implemented as an Objective-C 2.0 property.  Most of the work involved with changing the comment including the Apple events required to set the comment to a new value inside of the property setter defined for this property.









13.  Revealing an item in the Finder

The method -finderRevealFileURL: in the Controller class uses Scripting Bridge to reveal the referenced item in one of the Finder's windows.  As you can see, it is also a very simple method:


- (BOOL) finderRevealFileURL:(NSURL*) theFileURL {
    BOOL result;

    @try {
            /* retrieve the Finder application Scripting Bridge object. */
        FinderApplication* finder = [SBApplication applicationWithBundleIdentifier:@"com.apple.finder"];
        
            /* retrieve a reference to our finder item asking for it by location */
        FinderItem * theItem = [[finder items] objectAtLocation: theFileURL];
        
            /* display the item  */
        [theItem reveal];
        
            /* activate the Finder application  */
        [finder activate];

            /* successful result  */
        result = YES;
    }
    @catch(NSException *e) {
        result = NO;
    }
    
        /* return YES on success */
    return result;
}



Interesting items to note here are:

(a) Again, we use Objective-C exceptions for error handling.  If you need specialized error handling, the SBApplication class allows you to set a delegate object that implements a -eventDidFail:withError: delegate method.  The default simply @throws an error and we interpret all errors as failures in this method.

(b) we use the +applicationWithBundleIdentifier: SBApplication class method to create our Application object.  This will dynamically load the application object based on the dictionary inside of the application itself rather than building the application class from statically compiled information.

(c) We call the 'reveal' method defined on the FinderItem class to ask the Finder to reveal the item in one of its windows.  This method call is where most of the work involved in displaying the item occurs.

(d) To switch in the Finder so it is the frontmost application, we call the FinderApplication's activate method.  You'll notice that this method is also defined on the SBApplication class, so you can call it on any application to bring it into the foreground.  Calling the activate method is equivalent to the following AppleScript:

tell application "Finder"
    activate
end tell






14. Other details.

Much of the rest of the application code is there for housekeeping and managing the GUI.  Interesting parts worth mentioning are:

(a) We limit the size of Finder Comments to 750 Unicode characters in length.  Radar rdar://problem/4857955 states that Finder comments are limited to 750 Unicode characters.  This is the current recommendation at the time of this writing.

(b) we implement a -applicationDidBecomeActive: NSApplication delegate method to receive notifications whenever our application is switched into the forground.  We take this opportunity to refresh the comment field in case the comment was changed while our process was in the background.





15. Where to next?

Documentation for Scripting Bridge can be found in the Scripting Bridge Release Note.  To find the Scripting Bridge Release Note, select 'Documentation' from the Help Menu in Xcode, select "Release Notes" from the "Jump To:" menu in the top right of the document window, click on the "View the complete Release Notes List." link near the top left of the window and scroll down to the Scripting Bridge Release Note.

There are man pages available for the Scripting Bridge command line tools.  To access those pages, enter the following commands into the Terminal window:

   man sdp
   man sdef

There are two other Scripting Bridge samples available including ScriptingBridgeiCal and ScriptingBridgeFinder showing how to use Scripting Bridge together with the Apps named in their titles.



