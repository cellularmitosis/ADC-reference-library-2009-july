File: ReadMe.txt

Abstract: readme file for the SimpleWebKitPlugin sample.

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


This file details the steps involved in making this simple WebKit plug-in that uses a GUI loaded from a .nib file created in Interface Builder.  If you're just getting started with a WebKit plug-in sample and you'd like to make one with an GUI loaded from a .nib file, these steps should be sufficient to get your project on the road.  



1. Create a new project.
Create a new project in Xcode using the "Cocoa Bundle" template.




2. Add the WebKit framework to your project.
Turn down the "Frameworks and Libraries" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu and add the "WebKit.framework" (/System/Library/Frameworks/WebKit.framework) to the project.  

NOTE: Be sure to add "#import <WebKit/WebKit.h>" to the beginning of any files where you use the WebKit apis.




3. Add your plug-in's class files to the project.
Add your plug-in class files to the project.  control-click (or right click) on the classes group in the Groups & Files area and select "Add > New File..." from the pop-up menu.  Select the 'Objective-C class template and name the file "PluginWithSimpleGUI" (or whatever you would like to call your principle class).




4. Identify your bundle as a WebKit plug-in.
To do that, change the CFBundlePackageType entry in your Info.plist file to the value 'WBPL': 

	<key>CFBundlePackageType</key>
	<string>WBPL</string> 




5. Identify your plug-in's class for the bundle.
Identify the principal class used to define the functionality of your plug-in by setting the NSPrincipalClass entry in your Info.plist to the name of the class where you have implemented your plugInViewWithArguments: WebPlugInViewFactory factory method.  In this sample, that will be our "PluginWithSimpleGUI" class.

	<key>NSPrincipalClass</key>
	<string>PluginWithSimpleGUI</string>




6. Provide descriptive information about your plug-in.
Add a name and description for your plug-in by providing values for the WebPluginName and WebPluginDescription keys in your Info.plist file.  These names are used for displaying information about your plug-in to users.  In this sample we have added the following entries:

	<key>WebPluginName</key>
	<string>Simple GUI plug-in</string>
	<key>WebPluginDescription</key>
	<string>This is a sample WebKit plug-in that shows how to use a GUI designed in Interface Builder.</string>




7. Identify the mime types your plug-in is capable of displaying.
You can provide any number of mime types or file name extensions.  In this sample, we only provide one.

	<key>WebPluginMIMETypes</key>
	<dict>
		<key>application/x-simpleguiplugin</key>
		<dict>
			<key>WebPluginExtensions</key>
			<array>
				<string>sgui</string>
			</array>
			<key>WebPluginTypeDescription</key>
			<string>Simple GUI Plug-in Data</string>
		</dict>
	</dict>




8. Create the plug-in class.
Make PluginWithSimpleGUI a subclass of NSView and then add the allocation and factory methods to the new SimpleWebKitPlugin class.  Here is the adjusted interface declaration for the SimpleWebKitPlugin class:

		/* our main plug-in class definition. */
	@interface PluginWithSimpleGUI : NSView <WebPlugInViewFactory> {
		NSDictionary *pluginArguments;
	}
	
		/* WebPlugInViewFactory message handler for vending plug-in instances */
	+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments;
	
		/* storage management message handlers */
	-(id) initWithArguments:(NSDictionary *)arguments;
	- (void) dealloc;
	
	@end
	

Items to note are:
- PluginWithSimpleGUI is a subclass of NSView.  You can make your special plug-in class a subclass of any SubClass of NSView.  
- In the declaration of WebPlugInViewFactory, we have explicitly called out that PluginWithSimpleGUI conforms to the WebPlugInViewFactory protocol.  This will give us an extra level of compiler error checking to make sure we have implemented the requirements of that protocol correctly. 
- The method -plugInViewWithArguments: is a class method required by the WebPlugInViewFactory formal protocol for all WebKit plug-ins.
- We have added an instance variable 'pluginArguments' where we can save away a copy of the arguments we receive when our -plugInViewWithArguments: method is called.





9. Add the .nib file to your project

(a) In interface builder, select the File > New... menu command and choose the empty Cocoa nib template from the window that comes up.  This will create a new, empty, and untitled nib file for you.

(b) Select File > Save and save the nib file in the English.lproj folder inside of your plug-in's Xcode project directory.  In this sample we have named the .nib file 'PluginView.nib', but you can choose any name you prefer.

(c) After you confirm the save command and if you have the plug-in project open in Xcode, Interface Builder should ask you if you would like to add the nib file to your plug-in project.  If it does that, say yes.  Otherwise, if it does not ask you, then you will have to drag your .nib file's icon from the Finder into the Groups & Files list in your plug-in's Xcode project window.





10. Register your custom plug-in class with Interface Builder.
In this sample, we have implemented our plug-in as a subclass of NSView named PluginWithSimpleGUI.  We will want to make reference to this class when setting up your .nib file, so we need to register this class with Interface Builder.  To do that, drag the file PluginWithSimpleGUI.h from the Groups & Files view in Xcode into the .nib window in Interface Builder.

NOTE:  You can repeat this operation after making any changes to the PluginWithSimpleGUI.h file that you would like to propogate over to Interface Builder.  This is especially useful after adding new IBOutlets instance variables or IBAction methods.





11. Set the custom class of the File's owner in the .nib file to your plug-in's class. 
When we load the .nib file, we will specify our plug-in instance as the .nib file's owner.  Because of that, we'll set the file's owner to the the same class as our plug-in.  By doing so, all of the outlets we set up in our plug-in's class will be initialized for us when we load the .nib file.

(a) In the .nib file window, click on the instances tab and select the File's Owner icon. 

(b) Open the inspector window and select the Custom Class pane.

(c) Scroll down and select the name of your plug-in's class (in this sample, PluginWithSimpleGUI).





12.  In Interface Builder, add a custom view to the .nib file.
You can do this by dragging the Custom View from the Cocoa Containers palette into the .nib file window.  Once you have added your custom view to the project, open it and add in all of the user interface elements you would like to display in your plug-in.  In this sample, we have added a two text fields and a button organized inside of a box and split view.

For our purposes, all we will need to keep track of is the view itself and the two text fields.  And, we will want to set up the button so we will receive a notification when it has been pressed.  We'll set up these items in the next steps.




13. Add some IBOutlet and IBAction items to your plug-in class.
In this sample, we have added the following instance variables to our plug-in class that will be used as outlets by Interface Builder:

    IBOutlet NSView* viewPanel; 
    IBOutlet NSTextView *scriptText;
    IBOutlet NSTextView *scriptResult;

and we have added the following action that we will hook up to the run button:

	-(IBAction) runScript:(id)sender;

After adding your actions and outlets, save your file and re-fresh the information in Interface Builder by dragging and dropping the plug-in's header file (in this sample, PluginWithSimpleGUI.h) from the Groups & Files view in Xcode into the .nib window in Interface Builder (see step 10 above).




14. Connect up the outlets and actions in Interface Builder.
Just as in a regular application, we you can connect up the outlets defined on our custom plug-in class by dragging from the File's Owner icon into.  Here is how we set up the outlets we added in the previous step.  These references are already set up in this sample, but information about how they have been set up is provided below.

(a) IBOutlet NSView* viewPanel; 
This is a reference to our custom view.  To set up this outlet, hold down the control key and click on the File's Owner icon in the .nib file window.  Then, while continuing to hold down the mouse button and the control key, drag the mouse pointer over to the custom view's icon inside of the .nib file window.  Release the mouse when it is over the custom view's icon.  The inspector window should come up asking which outlet you would like to connect the view to.  Select the viewPanel outlet.

(b) IBOutlet NSTextView *scriptText;
This is a reference to the field where script text is entered.  Before trying to set up this outlet, your custom view should be open and all of the items in it should be visible.  Hold down the control key and click on the File's Owner icon in the .nib file window.  Then, while continuing to hold down the mouse button and the control key, drag the mouse pointer over to the top text field in the custom view.  Release the mouse when it is over the upper text field.  The inspector window should come up asking which outlet you would like to connect the view to.  Select the scriptText outlet.

(c) IBOutlet NSTextView *scriptResult;
This is a reference to the field where script text is entered.  Before trying to set up this outlet, your custom view should be open and all of the items in it should be visible.  Hold down the control key and click on the File's Owner icon in the .nib file window.  Then, while continuing to hold down the mouse button and the control key, drag the mouse pointer over to the bottom text field in the custom view.  Release the mouse when it is over the lower text field.  The inspector window should come up asking which outlet you would like to connect the view to.  Select the scriptResult outlet.


(d) -(IBAction) runScript:(id)sender;
in this step we're setting up the action called when the Run JavaScript button is clicked.  Before trying to set up this outlet, your custom view should be open and all of the items in it should be visible.   Hold down the control key and click on the Run JavaScript button in the custom view.  Then, while continuing to hold down the mouse button and the control key, drag the mouse pointer over File's Owner icon in the .nib file window.  Release the mouse when it is over the File's Owner icon.  The inspector window should come up asking which action you would like to connect the view to.  Select the runScript: action.

(e) Save changes to your .nib file and return to Xcode.
 




15. Add code to load the .nib file.
Load the nib file inside of your plug-in's WebPlugIn informal protocol method webPlugInInitialize.   In this sample, the following call is used to load our .nib file.  Since we have specified 'self' as the owner, all of the outlets and actions defined in the .nib file for the file's owner will be connected up while the .nib file is loaded.

- (void)webPlugInInitialize {
	BOOL result;
	result = [NSBundle loadNibNamed:@"PluginView" owner:self];
}




16. Add code to embed the the view from the .nib into the plug-in's NSView.
Once the nib file is loaded, perform some final steps for setting up the view.  The awakeFromNib method we have defined on our plug-in class will be called after the .nib file has been loaded and it is ready for use.  Once that is finished, we perform the final calls to set up the plug-ins frame and to add the custom view we created in Interface Builder as a subview of our plug-in.  Here is the awakeFromNib method from this sample:

- (void)awakeFromNib {
	NSLog(@"%@ received %@", self, NSStringFromSelector(_cmd));

	if( viewPanel != nil ) {
		[self setFrame:[viewPanel frame]];
		[self addSubview:viewPanel];
	}
}




19. Set up your project for debugging your plug-in.
The enclosed project has already been set up for debugging, but if you would like to set up your own project for debugging then follow the steps in Technical Q&A QA1500, "Debugging a WebKit Plug-in in Xcode" (http://developer.apple.com/qa/qa2006/qa1500.html) to set up your plug-in project for debugging with Xcode.  



20. Build and run the sample.
If you have followed the steps in QA1500, then when you click 'Build and Run' in Xcode, a new copy of Safari will be launched with the plug-in loaded.  A test heml page for the plug-in named test.html has been included in the project directory.  You can try out the plug-in by dragging and dropping test.html into window displayed by the copy of Safari that was launched as a result of the 'Build and Run' command.



Additional Information


Resources

This sample didn't call for it, but if for some reason your custom plug-in would like to obtain a reference to its bundle so it can access, say, the Resources folder inside of its bundle, then you could use the following code to do that:

		/* get a reference to your bundle */
	NSBundle *plugInBundle = [NSBundle bundleForClass:[self class]];




Size Constraints

The directions didn't talk about how we handled the size settings for the items in the .nib file, but we have taken special care to set those up so no matter what rectangle is specified in the embed tag in the web page, the contents of the view will fit into that rectangle properly.

The key here is to make sure to turn on the internal springs for the top level custom view added to the .nib file.  With that done, the view and its contents will be re-scaled properly when we embed it in our plug-in's frame frame inside of our -awakeFromNib method.

To get a clearer picture of how to set up the size constraints, spend some time browsing the size constraints for the custom NSView inside of Interface Builder.




Install Locations

Internet Plug-ins can be installed in the following locations:

To make the plug-in available to the current user only, install it here:

	~/Library/Internet Plug-Ins/

To make the plug-in available to all users, install it here:

	/Library/Internet Plug-Ins/









