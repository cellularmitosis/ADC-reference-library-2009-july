File: ReadMe.txt
 
 Abstract: ReadMe file for the iChatStatusFromApplication sample.
 
 Version: 1.1
 
 Disclaimer: IMPORTANT:	This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.	If you do not agree with these terms, please do not use,
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
 Software without specific prior written permission from Apple.	 Except
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
 
 Copyright (C) 2008 Apple Inc. All Rights Reserved.

"iChatStatusFromApplication" demonstrates how to use the Scripting Bridge framework to communicate with iChat. It uses the Accessibility API to listen for frontmost application change events, then uses the scripting bridge to set the user's status message and icon to represent what application they are currently using. The user's status message is set to "Using Safari" if Safari is the frontmost application, for example.  It also presents the user with a "Join Chat" button that, when clicked, uses the scripting bridge to ask iChat to join a specific AIM chat room ("ichatstatus").  You could use this or similar techniques to, for example, allow users of an application to join a discussion or support AIM chat room for that app.

"iChatStatusFromApplication" requires that "Enable access for assistive devices" be enabled in your Universal Access preferences in order to listen for active-application-changed events. It is worth noting that turning on "Enable access for assistive devices" is not a requirement for using the Scripting Bridge framework with iChat.


Building the Sample
1. Create a new project.
"iChatStatusFromApplication" is a Cocoa application. Open Xcode, choose File > New Project and select Cocoa Application under application in the New Project Assistant window. Click Next and save the project as iChatStatusFromApplication.

2. Add the Scripting Bridge framework to your project.
Open the Frameworks and Libraries group in the Groups & Files list of the project window and then right-click or control-click on the Linked Frameworks subgroup. Choose Add > Existing Frameworks from the pop-up menu and navigate to the /System/Library/Frameworks/ folder to select and add the ScriptingBridge.framework to your project.

3. Add the minimum system version Info.plist key
This project uses the Scripting Bridge framework that is only available in Mac OS X 10.5 and later. Set the LSMinimumSystemVersion to 10.5 so users can receive notices informing them about the minimum system requirement when they attempt to run the application on a system earlier than Mac OS X 10.5. 
Double-click the Info.plist file in the resource area of the project and copy and paste the following lines into the file. 
	<key>LSMinimumSystemVersion</key>
	<string>10.5</string>

4. Set the Target SDK to Mac OS X 10.5
Double-click the iChatStatusFromApplication project icon in the Groups & Files list. When the project info window appears, go to the General tab and set the "Cross-Develop Using Target SDK" to Mac OS X 10.5. 


5. Create iChat header file
Open the Terminal application and run the following command:
		sdef /Applications/iChat.app | sdp -fh --basename iChat

You should see an iChat.h file in your working directory. Drag and drop iChat.h into the Other Sources group in your Xcode project window. Check "Copy items into destination group's folder (if needed)" in the ensuing dialog. 

6. Create the ISFAController’s class files
Select File > New File and click Objective-C class under Cocoa in the New File Assistant window. Name the file ISFAController and check the "Also create..." checkbox if unchecked. 

7. Add the iChat header and define a scripting bridge object in the ISFAController.h file. See the ISFAController.h  file in this sample for its implementation.

8. Add Scripting Bridge to the code 
the "joinChatRoom" method in the ISFAController.m file describes how to use the Scripting Bridge to communicate with iChat.

9. Add code to watch for application switch events 
The "registerForAppSwitchNotificationFor" method in the ISFAController.m file describes how to watch for an application switch events. It uses an accessibility observer to pay attention to an application switch events.

10. Add code to update the current status and icon of users
The "applicationSwitched" method in the ISFAController.m file refreshes the current status and icon of users in iChat and "iChatStatus" window.

11. Build and run the sample
Click the Build and Go toolbar item in the project window to compile and run the sample. 


Using the Sample 
Turn on the "Enable access for assistive devices" (if disabled) in your Universal Access preferences.
Open this sample in Xcode 3.0 and Mac OS X 10.5 and later. Click the Build and Go toolbar item in the project window to compile and run the application. 

Switch to another application to see your status message and icon changed to your frontmost application's name and icon. 
Click the "Join Chat Room" button to join the "ichatstatus" AIM chat room in iChat. iChat will join the chat room in the same way as if you had selected to do so manually within iChat, so all normal actions will then work - for example, you can show/hide the participants drawer using the View < Show/Hide Chat Participants menu item, click the "+" button in the drawer to invite and add a user to the chat room. 



Changes from Previous Versions
Updated sample to work with released Leopard instead of WWDC 2007 Leopard Beta.
Check if the "Enable access for assistive devices" feature is turned on and warn users if it is not.



Feedback and Bug Reports
Please send all feedback about this sample by using the Feedback form on the bottom of the sample's webpage.
Please submit any bug reports about this sample to the Bug Reporting <http://developer.apple.com/bugreporter> page.