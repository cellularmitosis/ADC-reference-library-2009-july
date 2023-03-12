File: ReadMe.txt

Abstract: readme file for the SBSendEmail sample.

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

Copyright (C) 2008 Apple Inc. All Rights Reserved.




Introduction

This file details the steps involved in putting together a project that uses Scripting Bridge to send Apple events to the Mail application for the purposes of automatically sending emails.  The following details the steps involved in creating this sample.



1. Start with a new Cocoa application.

In this sample we're going to use the Mail application to send email so we'll call our sample 'SBSendEmail'.  The techniques we are demonstrating in this sample can be used in any Cocoa Application.  We used a simple one window Cocoa application for this sample to keep it simple.   

This project comes with the Interface Builder .nib file and the Controller object to support that already set up.  It is assumed that the reader is familiar with those parts of Cocoa programming so the mechanics of putting those parts together are not discussed in this readme.




2. Add a Scripting Bridge build rule to the project.

The first thing to do is to set up Xcode to automatically generate the Scripting Bridge source for the application you would like to target.  The following steps describe how you can do that:

(a) Turn down the "Targets" tab in the "Groups & Files" list on the left hand side and select the main target.  In this sample,  select the "SBSendEmail" target.  

(b) With the "SBSendEmail" target selected, open the information panel (by clicking on the Info icon, or by control- or right-clicking on the target name and selecting "Get Info" from the pop-up menu).

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

You can drop the application among the source files you are using for your application.  Because of the build rule we added in step 2, Xcode will treat the Mail application as if it were one of the source files during the build.  

You should uncheck the 'Copy items into destination group's folder (if needed)' option so the application is not copied into your project directory.  In this sample we have selected 'Absolute Path' as the reference type so we can easily move the project around from machine to machine without invalidating the reference (so long as the Mail application is present in the System/Library/CoreServices folder,  Xcode will be able to find it).

In this case, we are adding the Mail to our project.  The Mail application is located in the /System/Library/CoreServices folder.




4. Add the target application to the Compile Sources.

After you have added the target application to your project, you must also add it to the main target's Compile Sources.  You can do that by adding the application to the 'Compile Sources' build phase under the main target.  In the "Groups & Files" list, open the "Targets" and the "SBSendEmail" target  and the Compile Sources build phase items by clicking on the arrows to the left of them.  Then, drag the Mail.app from the project files group into the Compile Sources.




5. Add the Scripting Bridge framework to your project.

Turn down the "Frameworks" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu.  Then, add the "ScriptingBridge.framework" (/System/Library/Frameworks/ScriptingBridge.framework) to the project.  




6. Add a minimum system version Info.plist key.

Since the ScriptingBridge.framework is necessary for this application to run and that framework is not present on previous system versions, you should add the following key/value pair to the Info.plist file for the application.  If someone tries to run this application on a system earlier than Mac OS X 10.5, then they will receive a notice from launch services letting them know that the application is meant to be run on a later version of Mac OS X.

	<key>LSMinimumSystemVersion</key>
	<string>10.5</string>

You can edit the Info.plist file by either clicking on its icon in the resources section, or by clicking on the SBSendEmail target, clicking on the Info icon, selecting the Properties tab, and then clicking on "Open Info.plist as File".




7. Make sure the Target SDK is set to Mac OS X 10.5

You won't have to change this setting for this project, but if you are adding Scripting Bridge to another project that you started an earlier version of the Mac OS than Mac OS X 10.5, then you will need to update the Target SDK for the project.  In the Groups & Files view, select the SBSendEmail project icon at the top of the list and then open the information window.  Under the General tab, change the Cross-Develop Using Target SDK: setting to Mac OS X 10.5.




8. Build your project.

If you have followed the steps above, Xcode will generate the Scripting Bridge source for your project.  They will be put inside of your build folder in a place where the linker and compiler can find them.  

The build rule that we installed will create a .h file with the same name as the application.  For example, if you added Mail to our project, then the build rule will create Mail.h.  The files will be created inside of the build directory in the DerivedSources directory where the compiler can find them.

For the Debug build, the Mail.h file will be located in this sub folder of the build directory:
/build/SBSendEmail.build/Debug/SBSendEmail.build/DerivedSources/Mail.h

For the Release build, the Mail.h file will be located in this sub folder of the build directory:
/build/SBSendEmail.build/Release/SBSendEmail.build/DerivedSources/Mail.h

A convenient way for you to open and inspect these files is to use the 'Open Quickly' command in the file menu.  For most purposes, the .h file will contain most of the interesting information so to view that file you open the 'Mail.h' file.  

In some cases, depending on what frameworks are in your project, the 'Open Quickly' command may open the system's 'Mail.h' file that includes constants and definitions used by the file system and the Mail application.  If that happens for you, then you will need to navigate into the build folder to find the correct header file.




9. Add in the Mail's Scripting Bridge header.

In the file Controller.m, we have added the import statement '#import "Mail.h"' near the top of the file below '#import "Controller.h"'.  This will include all of the Scripting Bridge definitions for the Mail.

Here is the imports section in our Controller.m file:

	#import "Controller.h"
	#import "Mail.h"

In your own application, of course, you would import the Mail.h file in the file where you intend to call it from.  In this sample, we are using the Scripting Bridge interface inside of three methods in our Controller class so that is why we are importing it into Controller.m.





10. The email sending code

So far, the preceeding steps have described how to set up a project to use Scripting Bridge to target the Mail application.  This sample illustrates how to send an email and the code to do that is relatively simple.  In this section, we'll talk about the code in the -sendEmailMessage: method of the Controller class.

Here is the method itself:


- (IBAction)sendEmailMessage:(id)sender {

        /* create a Scripting Bridge object for talking to the Mail application */
    MailApplication *mail = [SBApplication
            applicationWithBundleIdentifier:@"com.apple.Mail"];
    
        /* create a new outgoing message object */
    MailOutgoingMessage *emailMessage =
            [[[mail classForScriptingClass:@"outgoing message"] alloc]
        initWithProperties:
            [NSDictionary dictionaryWithObjectsAndKeys:
                [self.subjectField stringValue], @"subject",
                [[self.messageContent textStorage] string], @"content",
                nil]];
                
        /* add the object to the mail app  */
    [[mail outgoingMessages] addObject: emailMessage];
    
        /* set the sender, show the message */
    emailMessage.sender = [self.fromField stringValue];
    emailMessage.visible = YES;
                
        /* create a new recipient and add it to the recipients list */
    MailToRecipient *theRecipient =
            [[[mail classForScriptingClass:@"to recipient"] alloc]
        initWithProperties:
            [NSDictionary dictionaryWithObjectsAndKeys:
                [self.toField stringValue], @"address",
                nil]];
    [emailMessage.toRecipients addObject: theRecipient];
    
        /* add an attachment, if one was specified */
    NSString *attachmentFilePath = [self.fileAttachmentField stringValue];
    if ( [attachmentFilePath length] > 0 ) {
    
            /* create an attachment object */
        MailAttachment *theAttachment = [[[mail
            classForScriptingClass:@"attachment"] alloc]
                initWithProperties:
                    [NSDictionary dictionaryWithObjectsAndKeys:
                        attachmentFilePath, @"fileName",
                        nil]];
                        
            /* add it to the list of attachments */
        [[emailMessage.content attachments] addObject: theAttachment];
    }
        /* send the message */
    [emailMessage send];
}


Interesting points to note:

a. We add the newly created outgoing method to the Mail application's outgoingMessages just after creating it.  This forces Scripting Bridge to send apple events over to the Mail application asking it to create an actual scripting object for the Scripting Bridge proxy object.  After that, we can set properties on the scripting object through our proxy object without any trouble. 

b. When creating new objects the property name in the dictionary providing the properties for the new object is specified using the Objective-C key name.  For example, when creating a new attachment, we used the Cocoa key "fileName" for the AppleScript "file name" property.

c. When creating new objects, we do not specify the Cocoa key value for the class name.  For example, when creating an outgoing email message, we used the AppleScript name "outgoing message" for the class.


d. Inside of the -chooseFileAttachment: method, the NSOpenPanel is set to allow directory selections (via the call to -setCanChooseDirectories:).  When you attach a directory to an outgoing mail message in the Mail application, Mail takes care of bundling up the contents of the directory as a single .zip file attachment that is sent with your message.

e. In this application we only provide facilities for adding a single attachment to an outgoing email message.  This is on purpose to keep the user interface simple.  However, it is not too difficult to revise the -sendEmailMessage: method to add more than one attachment.

First, suppose that the UI was revised to provide an NSArray of path names referring to attachment files.  For illustration, we'll use this NSArray definition:

    /* files to be sent as attachments */
NSArray *attachmentPaths =
    [NSArray arrayWithObjects:
        @"/tmp/one.gif",
        @"/tmp/two.jpg",
        @"/tmp/three.pdf",
        nil];

Then, given that we have the NSArray attachmentPaths, the following code could be used to add them as attachments to the email message.

    /* iterate through the list of files */
for ( NSString* nthPath in attachmentPaths ) {

        /* create an attachment object */
    MailAttachment *theAttachment = [[[mail
        classForScriptingClass:@"attachment"] alloc]
            initWithProperties:
                [NSDictionary dictionaryWithObjectsAndKeys:
                    nthPath, @"fileName",
                    nil]];
                    
        /* add it to the list of attachments */
    [[emailMessage.content attachments] addObject: theAttachment];
}




11. Where to next?

Documentation for Scripting Bridge can be found in the Scripting Bridge Release Note at this address:

    http://developer.apple.com/releasenotes/ScriptingAutomation/RN-ScriptingBridge/index.html

There are man pages available for the Scripting Bridge command line tools.  To access those pages, enter the following commands into the Terminal window:

   man sdp
   man sdef

There are some other Scripting Bridge samples available including ScriptingBridgeiCal and ScriptingBridgeFinder showing how to use Scripting Bridge together with the Apps named in their titles.



