File: ReadMe.txt

Abstract: readme file for the ScriptingBridgeiCal sample.

Version: 1.0

(c) Copyright 2007 Apple, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.




Introduction

This file details the steps involved in putting together a project that uses Scripting Bridge to send Apple events to the iCal application for creating
calendars and adding or updating events.  



1. Start with a new Objective-C application.

This targets the iCal application and was built using the 'Cocoa Application' Xcode template.  The techniques we are demonstrating in this sample can be used in any Cocoa Application. 




2. Add a Scripting Bridge build rule to the project.

The first thing to do is to set up Xcode to automatically generate the Scripting Bridge source for the application you would like to target.  The following steps describe how you can do that:

(a) Turn down the "Targets" tab in the "Groups & Files" list on the left hand side and select the main target.  In this sample,  select the "ScriptingBridgeiCal" target.  

(b) With the "ScriptingBridgeiCal" target selected, open the information panel (by clicking on the Info icon, or by control- or right-clicking on the target name and selecting "Get Info" from the pop-up menu).

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

NOTE: this rule uses the sdef and sdp command line tools.  To learn more about these tools, use the following commands in the terminal window:
   man sdp
   man sdef




3. Select a target application.

To do this, drag and drop the iCal application into the project files group inside of the "Groups & Files" list in the project window.   

You can drop the application among the source files you are using for your application.  Because of the build rule we added in step 2, Xcode will treat the iCal application as if it were one of the source files during the build.  

You should uncheck the 'Copy items into destination group's folder (if needed)' option so the application is not copied into your project directory.  In this sample we have selected 'Absolute Path' as the reference type so we can easily move the project around from machine to machine without invalidating the reference (so long as the iCal application is present in the Applications folder, the Xcode will be able to find it).




4. Add the target application to the Compile Sources.

After you have added the target application to your project, you must also add it to the main target's compile sources.  You can do that by adding the application to the 'Compile Sources' build phase under the main target.  In the "Groups & Files" list, open the "Targets" and the "ScriptingBridgeiCal" target  and the Compile Sources build phase items by clicking on the arrows to the left of them.  Then, drag the iCal.app from the project files group into the Compile Sources.




5. Add the Scripting Bridge framework to your project.

Turn down the "Frameworks" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu.  Then, add the "ScriptingBridge.framework" (/System/Library/Frameworks/ScriptingBridge.framework) to the project.  




6. Add the minimum system version Info.plist key.

Since the ScriptingBridge.framework is necessary for this application to run and that framework is not present on previous system versions, we should add the following key/value pair to the Info.plist file for the application.  If someone tries to run this application on a system earlier than Mac OS X 10.5, then they will receive a notice from launch services letting them know that the application is meant to be run on a later version of Mac OS X.

	<key>LSMinimumSystemVersion</key>
	<string>10.5</string>

You can edit the Info.plist file by either clicking on its icon in the resources section, or by clicking on the ScriptingBridgeiCal target, clicking on the Info icon, selecting the Properties tab, and then clicking on "Open Info.plist as File".




7. Make sure the Target SDK is set to Mac OS X 10.5

You won't have to change this setting for this project, but if you are adding Scripting Bridge to another project that you started an earlier version of the Mac OS than Mac OS X, then you will need to update the Target SDK for the project.  In the Groups & Files view, select the ScriptingBridgeiCal project icon at the top of the list and then open the information window.  Under the General tab, change the Cross-Develop Using Target SDK: setting to Mac OS X 10.5.




8. Build your project.

If you have followed the steps above, Xcode will generate the Scripting Bridge source for your project.  They will be put inside of your build folder in a place where the linker and compiler can find them.  

The build rule that we installed will create a .h file with the same name as the application.  For example, if you added iCal to our project, then the build rule will create iCal.h.  The files will be created inside of the build directory in the DerivedSources directory where the compiler can find them.

For the Debug build, the iCal.h file will be located in this sub folder of the build directory:
/build/ScriptingBridgeiCal.build/Debug/ScriptingBridgeiCal.build/DerivedSources/iCal.h

For the Release build, the iCal.h file will be located in this sub folder of the build directory:
/build/ScriptingBridgeiCal.build/Release/ScriptingBridgeiCal.build/DerivedSources/iCal.h

A convenient way for you to open and inspect these files is to use the 'Open Quickly' command in the file menu.  For most purposes, the .h file will contain most of the interesting information so to view that file you open the 'iCal.h' file.

NOTE:  Building the application will launch the iCal application in order to obtain its scripting information.  




9. Add in the iCal Scripting Bridge header.

In the file Controller.m, add '#import "iCal.h"' near the top of the file.  This will include all of the Scripting Bridge definitions for iCal.

	#import "Controller.h"
	#import "iCal.h"

In your own application, of course, you would import the iCal.h file in the file where you intend to call it from.  In this sample, we are using the Scripting Bridge interface inside of three methods in our Controller class so that is why we are importing it into Controller.m.




10.  Add in the Scripting Bridge code.

In this sample All of the Scripting Bridge functionality happens in the addUpdateEvent message handler inside of the Controller.m file.  In the following steps, we'll add sources using Scripting Bridge to that message handler. 




11. Add Declarations.

At the beginning of the message handler we'll add some variable declarations and we'll instantiate an instance of our iCalApplication Scripting Bridge class.  Since we would like to see the changes being made in the iCal window, we'll call [iCal activate] to bring the application to the front.

Here is the code that performs these operations.  It is from the -addUpdateEvent: method in the Controller class.  Have indicated that method with the comment '/* code from Step 11: */' in that method:

		/* reference to our iCal application object */
	iCalApplication *iCal;
	
		/* get the scripting bridge object for the target application. */
    iCal = [SBApplication applicationWithBundleIdentifier:@"com.apple.iCal"];
	
		/* bring the iCal application into view */
    [iCal activate];
	
	
This is equivalent to the AppleScript:

	tell application "iCal"
		activate
	end tell


	


12. Add calendar creation/selection.

Next we're going to discuss the code used to retrieve a reference to the calendar with a name matching the name typed into the window.  If one doesn't exist, we'll create a new calendar with that name.  Interesting items to note in this section are:

(a) We're using the -filteredArrayUsingPredicate: method together with an NSPredicate object to find the calendar with the required name. 

(b) If no calendar is found,  we create a new calendar.  You'll notice that the names of the properties in the dictionary are the same names a user would type in AppleScript when they are creating a new object.   

(c) In the else part, the new object isn't actually created and added inside of the iCal application until the call to addObject: is made.  


Here is the code that performs these operations.  It is from the -addUpdateEvent: method in the Controller class.  Have indicated that method with the comment '/* code from Step 12: */' in that method:

	
		/* reference to our calendar object */
	iCalCalendar *theCalendar;

		/* get the name of the calendar from the window. */
	NSString *calendarName = [calendar stringValue];
	
		/* get the calendar with the specified name using the
		-filteredArrayUsingPredicate: method.  */
	NSArray *matchingCalendars =
		[[iCal calendars] filteredArrayUsingPredicate:
			[NSPredicate predicateWithFormat:@"name == %@", calendarName]];
	if ( [matchingCalendars count] > 0 ) {
		theCalendar = (iCalCalendar *) [matchingCalendars objectAtIndex:0];
	}
	
		/* If no such calendar exists, then create a new one with that name. */
		NSDictionary *props =
			[NSDictionary dictionaryWithObjectsAndKeys:
				calendarName, @"name",
				nil];
		
			/* allocate and initialize the new calendar */
		theCalendar = [[iCalCalendar alloc] initWithProperties: props];
		
			/* ...and add it to the list of calendars in the iCal application. */
		[[iCal calendars] addObject: theCalendar]; 
	}


Barring the UI handling code, our sample is now equivalent in functionality to the AppleScript:

	tell application "iCal"
	
			(* parts added in step 11 *)
		activate
		
			(* parts added in step 12 *)
		set calendarName to "My Calendar"
		if (count of (every calendar whose name is calendarName)) > 0 then
			set theCalendar to item 1 of (every calendar whose name is calendarName)
		else
			set theCalendar to make new calendar with properties {name:calendarName}
		end if
		
	end tell

Note: You can build and run the sample after adding this code.  It should launch iCal application, bring it into the foreground, and create a new calendar using the calendar name displayed in the ScriptingBridgeiCal window.




13.  Event creation/selection.

In this final step, we're going to look inside of the calendar object we retrieved in step 12 for an event who's summary matches the name typed in the user interface.  In iCal the event summary is the name that shows up in the calendar window, and events do not have names.  Because we're using the event summary value to identify our events, this gives us an opportunity to show how to use some other features of scripting bridge.  Interesting points about this section are:

(a) We are using an NSEnumerator to traverse the list of events inside of the calendar.  As NSEnumerator is a standard Cocoa class, this is a good example of how you can use standard Cocoa classes together with Scripting Bridge classes.

(b) In the else part, we create a new event.  You'll notice that the names of the properties in the dictionary are the kvc names for the properties as they
appear in the iCal.h header file.   

	
Here is the code that performs these operations.  It is from the -addUpdateEvent: method in the Controller class.  Have indicated that method with the comment '/* code from Step 13: */' in that method:

		/* get the event with the specified name from the calendar.  If
		no such event exists, then create a new one with that name.  */
		
		/* reference to our event object */
	iCalEvent *theEvent;

		/* get the name of the event from the window. */
	NSString *eventName = [event stringValue];
	
		/* calculate start and end times for a one hour event
		starting at the time displayed in the window. */
	NSDate* startDate = [time dateValue];
	
		/* set the end date to the start time plus one hour (3600 seconds). */
	NSDate* endDate = [[NSDate alloc] initWithTimeInterval:3600 sinceDate:startDate];
	
	
		/* the event summary contains the name displayed in the iCal calendar windows,
		so we can't use [[[theCalendar events] objectWithName:eventName] exists]
		to find out if the event exists. 
		
		So, instead of that we're going to use filteredArrayUsingPredicate to retrieve
		an array of all of the events with a matching summary. */
		
	NSArray *matchingEvents =
		[[theCalendar events] filteredArrayUsingPredicate:
			[NSPredicate predicateWithFormat:@"summary == %@", eventName]];
		
		/* if we found at least one matching event, then we'll update the
		times for the first event with a matching summary */
	if ( [matchingEvents count] >= 1 ) {
	
			/* get the first matching event out of the response */
		theEvent = (iCalEvent *) [matchingEvents objectAtIndex:0];

			/* update the dates for the event */
		[theEvent setStartDate:startDate];
		[theEvent setEndDate:endDate];
		
	} else {
			/* otherwise, create a new event */
			
			/* set up the event properties.  Note, we're using kvc names from
			the iCal.h file to name the properties encoded in the NSDictonary. */
		NSDictionary *props = 
			[NSDictionary dictionaryWithObjectsAndKeys:
				eventName, @"summary",
				startDate, @"startDate",
				endDate, @"endDate",
				nil];

			/* create the new event */
		theEvent = [[iCalEvent alloc] initWithProperties: props];
		
			/* add it to the list of events for this calendar. */
		[[theCalendar events] addObject: theEvent];
		
	}
		

Barring the UI handling code, our sample is now equivalent in functionality to the AppleScript:

	tell application "iCal"
	
			(* parts added in step 11 *)
		activate
		
			(* parts added in step 12 *)
		set calendarName to "K"
		if (count of (every calendar whose name is calendarName)) >= 1 then
			set theCalendar to item 1 of (every calendar whose name is calendarName)
		else
			set theCalendar to make new calendar with properties {name:calendarName}
		end if
		
			(* parts added in step 13 *)
		set eventName to "J"
		set startDate to (current date)
		set endDate to startDate + 3600
		if (count of (every event of theCalendar whose summary is eventName)) > 0 then
			set theEvent to item 1 of (every event of theCalendar whose summary is eventName)
			set start date of theEvent to startDate
			set end date of theEvent to endDate
		else
			set theEvent to make new event at end of events of theCalendar with properties {summary:eventName, start date:startDate, end date:endDate}
		end if
		
	end tell




12. Build and run the sample.

It should launch iCal application, bring it into the foreground,  create a new calendar using the calendar name displayed in the ScriptingBridgeiCal window, create a new event in that calendar, and set the start time and end time for that event.  Of course, if the calendar or event with the correct name already exists, then the app will just update those.




13. Where to next?

Documentation for Scripting Bridge can be found in the Scripting Bridge Release Note.  To find the Scripting Bridge Release Note, select 'Documentation' from the Help Menu in Xcode, select "Release Notes" from the "Jump To:" menu in the top right of the document window, click on the "View the complete Release Notes List." link near the top left of the window and scroll down to the Scripting Bridge Release Note.

There are man pages available for the Scripting Bridge command line tools.  To access those pages, enter the following commands into the terminal window:

   man sdp
   man sdef

There are two other Scripting Bridge samples available including SBSetFinderComment and ScriptingBridgeFinder showing how to use Scripting Bridge together with the Finder.






