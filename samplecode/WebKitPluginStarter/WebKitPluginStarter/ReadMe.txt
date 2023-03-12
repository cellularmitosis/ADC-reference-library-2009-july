File: ReadMe.txt

Abstract: readme file for the WebKitPluginStarter sample.

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


This file details the steps involved in making this simple WebKit plug-in.  If you're just getting started with a webkit plugin sample, these steps should be sufficient to get your project on the road.  



1. Create a new project.
Create a new project in Xcode using the "Cocoa Bundle" template.




2. Add the WebKit framework to your project.
Turn down the "Frameworks and Libraries" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu and add the "WebKit.framework" (/System/Library/Frameworks/WebKit.framework) to the project.  

NOTE: Be sure to add "#import <WebKit/WebKit.h>" to the beginning of any files where you use the WebKit apis.




3. Add your plug-in's class files to the project.
Add your plug-in class files to the project.  control-click (or right click) on the classes group in the Groups & Files area and select "Add > New File..." from the pop-up menu.  Select the 'Objective-C class template and name the file "PluginStarter" (or whatever you would like to call your principle class).




4. Identify your bundle as a WebKit plug-in.
To do that, change the CFBundlePackageType entry in your Info.plist file to the value 'WBPL': 

	<key>CFBundlePackageType</key>
	<string>WBPL</string> 




5. Identify your plug-in's class for the bundle.
Identify the principal class used to define the functionality of your plug-in by setting the NSPrincipalClass entry in your Info.plist to the name of the class where you have implemented your plugInViewWithArguments: WebPlugInViewFactory factory method.  In this sample, that will be our "PluginStarter" class.

	<key>NSPrincipalClass</key>
	<string>PluginStarter</string>




6. Provide descriptive information about your plug-in.
Add a name and description for your plug-in by providing values for the WebPluginName and WebPluginDescription keys in your Info.plist file.  These names are used for displaying information about your plug-in to users.  In this sample we have added the following entries:

	<key>WebPluginName</key>
	<string>Simple Web Kit Plug-In</string>
	<key>WebPluginDescription</key>
	<string>A simple web kit plug-in example.</string>




7. Identify the mime types your plug-in is capable of displaying.
You can provide any number of mime types or file name extensions.  In this sample, we only provide one.


	<key>WebPluginMIMETypes</key>
	<dict>
		<key>application/x-simplewebkitplugin</key>
		<dict>
			<key>WebPluginExtensions</key>
			<array>
				<string>swkp</string>
			</array>
			<key>WebPluginTypeDescription</key>
			<string>Simple Web Kit Plug-In Data</string>
		</dict>
	</dict>




8. Create the plug-in class.
Your plugin class should be a subclass of NSView and it should conform to the WebPlugInViewFactory protocol.  In this sample, we have used the custom NSView class called SpeedometerView from the SpeedometerView sample as our parent class, and we have declared the PluginStarter class as conformant to the WebPlugInViewFactory protocol.  Here is the declaration:

	@interface PluginStarter : SpeedometerView <WebPlugInViewFactory> {
		NSDictionary *pluginArguments;
	}
	
	+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments;
	
	-(id) initWithArguments:(NSDictionary *)arguments;
	- (void) dealloc;
	
	@end
	
Items to note are:

- SimpleWebKitPlugin must be a descendant of NSView.  You can make your special plugin class a subclass of any SubClass of NSView.  Here, we have used a custom NSView from another sample as our parent class.

- SpeedometerView is a subclass of NSView.  It was taken directly from the SpeedometerView view sample without modification to show that you can make any custom NSView into a Cocoa WebKit plug-in using the steps outlined in this sample.

- plugInViewWithArguments: is a class method required by the WebPlugInViewFactory formal protocol for all webkit plugins.  We explicitly declare the PluginStarter conformant to the WebPlugInViewFactory protocol so we have an extra level of compiler checking to make sure our declarations are in line with the requirements of that protocol.

- we have added an instance variable 'pluginArguments' where we can save away a copy of the arguments we receive when our plugInViewWithArguments: method is called.





9. Follow the steps in Technical Q&A QA1500, "Debugging a Web Kit Plug-in in Xcode" (http://developer.apple.com/qa/qa2006/qa1500.html) to set up your plugin project for debugging with Xcode.




10. build and run the sample.  It will launch a new copy of Safari with the plug-in loaded.  To test the sample, drag and drop the file test.html into the Safari window.  The plug-in will be displayed in the web page.




Additional Information

Internet Plug-ins can be installed in the following locations:

To make the plug-in available to the current user only, install it here:

	~/Library/Internet Plug-Ins/

To make the plug-in available to all users, install it here:

	/Library/Internet Plug-Ins/




Where to next?

This sample shows how to create a minimal WebKit plug-in that does some custom drawing.  Other samples that build on top of the techniques illustrated in this sample include the following samples:

(a) WebKitPluginWithJavaScript - shows how to call JavaScripts in the containing page and how to call your plug-in from JavaScripts in the containing page.

(b) WebKitPluginWithSimpleGUI - shows how your plug-in can display a GUI loaded from a .nib file created in Interface Builder.




