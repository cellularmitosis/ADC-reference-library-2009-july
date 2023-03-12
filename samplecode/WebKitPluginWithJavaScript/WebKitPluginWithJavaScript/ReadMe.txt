File: ReadMe.txt

Abstract: readme file for the PluginWithJavaScript sample.

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


This file details the steps involved in making this simple WebKit plug-in.  If you're just getting started with a webkit plug-in sample, these steps should be sufficient to get your project on the road.  



1. Create a new project.
Create a new project in Xcode using the "Cocoa Bundle" template.




2. Add the WebKit framework to your project.
Turn down the "Frameworks and Libraries" group in the Groups & Files area, and then control-click (or right click) on the "Linked Frameworks" sub-group.  Select "Add > Existing Frameworks..." from the pop-up menu and add the "WebKit.framework" (/System/Library/Frameworks/WebKit.framework) to the project.  

NOTE: Be sure to add "#import <WebKit/WebKit.h>" to the beginning of any files where you use the webkit apis.




3. Add your plug-in's class files to the project.
Add your plug-in class files to the project.  control-click (or right click) on the classes group in the Groups & Files area and select "Add > New File..." from the pop-up menu.  Select the 'Objective-C class template and name the file "PluginWithJavaScript" (or whatever you would like to call your principle class).




4. Identify your bundle as a WebKit plug-in.
To do that, change the CFBundlePackageType entry in your Info.plist file to the value 'WBPL': 

	<key>CFBundlePackageType</key>
	<string>WBPL</string> 




5. Identify your plug-in's class for the bundle.
Identify the principal class used to define the functionality of your plug-in by setting the NSPrincipalClass entry in your Info.plist to the name of the class where you have implemented your plugInViewWithArguments: WebPlugInViewFactory factory method.  In this sample, that will be our "PluginWithJavaScript" class.

	<key>NSPrincipalClass</key>
	<string>PluginWithJavaScript</string>




6. Provide descriptive information about your plug-in.
Add a name and description for your plug-in by providing values for the WebPluginName and WebPluginDescription keys in your Info.plist file.  These names are used for displaying information about your plug-in to users.  In this sample we have added the following entries:

	<key>WebPluginName</key>
	<string>Simple WebKit Plug-In</string>
	<key>WebPluginDescription</key>
	<string>A simple WebKit plug-in example.</string>




7. identify the mime types your plug-in is capable of displaying:
You can provide any number of mime types or file name extensions.  In this sample, we only provide one.


	<key>WebPluginMIMETypes</key>
	<dict>
		<key>application/x-javascriptplugin</key>
		<dict>
			<key>WebPluginExtensions</key>
			<array>
				<string>swkp</string>
			</array>
			<key>WebPluginTypeDescription</key>
			<string>Simple WebKit Plug-In Data</string>
		</dict>
	</dict>




8. Create the plug-in class.
Make PluginWithJavaScript a subclass of NSView and then add the allocation and factory methods to the new PluginWithJavaScript class.  Here is the adjusted interface declaration for the PluginWithJavaScript class:

	@interface PluginWithJavaScript : SpeedometerView <WebPlugInViewFactory> {
		NSDictionary *pluginArguments;
		NSString *title;
		NSNumber *counter;
	}
	
		/* WebPlugInViewFactory message handler for vending plug-in instances */
	+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments;
	
		/* storage management message handlers */
	-(id) initWithArguments:(NSDictionary *)arguments;
	- (void) dealloc;
	
		/* accessors for pluginArguments */
	- (NSDictionary *)pluginArguments;
	- (void)setPluginArguments:(NSDictionary *)value;
	
	- (NSString *)title;
	- (void)setTitle:(NSString *)value;
	
	- (NSNumber *)counter;
	- (void)setCounter:(NSNumber *)value;
	
	
	@end
	
Items to note are:
- PluginWithJavaScript is a subclass of NSView.  You can make your special plug-in class a subclass of any SubClass of NSView.  
- In the declaration of WebPlugInViewFactory, we have explicitly called out that PluginWithSimpleGUI conforms to the WebPlugInViewFactory protocol.  This will give us an extra level of compiler error checking to make sure we have implemented the requirements of that protocol correctly. 
- The method -plugInViewWithArguments: is a class method required by the WebPlugInViewFactory formal protocol for all WebKit plug-ins.
- We have added an instance variable 'pluginArguments' where we can save away a copy of the arguments we receive when our -plugInViewWithArguments: method is called.




9. Add code for calling JavaScript from the plug-in.

In this sample we install a key-value observer on three of the properties maintained by our superclass, the SpeedometerView.  What this means is that whenever one of the setter methods for one of those properties is called, our observer method will be called to let us know that the value is changing and what the new value will be.   We use this opportunity to send notifications about the changing values to the JavaScript in the html page containing the WebKit plug-in.  

	
In our -initWithArguments: method, we set ourself as the key value observer for the keys @"speed", @"curvature", @"ticks", and KVC accessors are defined for those keys on our SpeedometerView superclass.  Inside of the observer, we intercept calls to the accessors and call through to the JavaScript in the containing page to provide notifications about the changing values.
	
The names of the JavaScript functions called is based on the name of the key.  For the keys @"speed", @"curvature", and @"ticks" we call JavaScript methods named @"speedValueChangedTo", @"curvatureValueChangedTo", and @"ticksValueChangedTo".

The file test.html includes definitions for these methods so you can try them out directly.  The most visible one is the speed value.  It will change as you drag the speedometer pointer around.  





10. Add code for calling our plug-in from JavaScript in the containing page.
Essentially, there are three parts involved in allowing JavaScripts in the containing page call through to your plug-in.  These are discussed in the following three sections.



(a) Identify the object that contains the methods and properties (instance variables) being made available to JavaScript.
In this sample, we use the plug-in object itself, but you can use another defined in a separate file if that would be a more convenient way to factor your code.  To identify the object implementing the following method in our plug-in's class:


- (id)objectForWebScript {
    return self;
}

This identifies the plug-in object as the object implementing all of the methods and properties that will be shared with JavaScript in the containing page.



(b) Identify the methods that will be shared with JavaScript.  
We do this by providing a method that participates in a filtering mechanism where WebKit calls our +isSelectorExcludedFromWebScript: class method with the selectors it considers eligible (methods implemented in our plug-in subclass) for calling from JavaScript and our method should return a YES/NO result indicating our preference.  In this sample we have implemented some methods for getting and setting the speed, curvature, ant ticks values by simply calling through to the accessors in the superclass.

The comments above the definition of +webScriptNameForSelector: in the WebKit header file WebScriptObject.h describe the default name translations that occur when the selectors for Objective-C method names are converted into method names that can be used from JavaScript.  But, if you would like to provide your own special name conversions you can do so by implementing +webScriptNameForSelector: class method.  In this sample, we implement this method to provide translations for method names such as 'setCurrentCurvature:' to 'SetCurvature', and so forth.

Once a method has been made available to JavaScript, the JavaScripts on the containing page will be able to call it as if it were a JavaScript method defined on the plug-in object in the page.  For example, if our plug-in object were stored in the variable jsPlugin, we could call the SetCurvature method as follows:

   jsPlugin.SetCurvature(22);



(c) Identify the instance variables that will be shared with JavaScript.
Here, again, we notify WebKit about what instance variables should be shared with JavaScript by providing methods that participate in a filtering mechanism.  WebKit will call our +isKeyExcludedFromWebScript: for every instance variable it considers eligible for sharing with JavaScript (provided in our plug-in subclass), and our class method should return a YES/NO result indicating our preference.

As with method names, you can provide your own name translation for property values by implementing a +webScriptNameForKey: method.  In this sample, we have implemtned one name translation so that the 'counter' instance variable is made available in JavaScript using the name 'total'

Once a property has been made available to JavaScript, the JavaScripts on the containing page will be able to access the property as if it were a JavaScript property defined on the plug-in object in the page.  For example, if our plug-in object were stored in the variable jsPlugin, we could access the the total property as follows:

   jsPlugin.total = 22;

NOTE:  only instance variables defined in your subclass can be made available in this way.  Instance variabled defined in superclasses are not eligible for sharing with JavaScript in this way.





11. Follow the steps in Technical Q&A QA1500, "Debugging a WebKit Plug-in in Xcode" (http://developer.apple.com/qa/qa2006/qa1500.html) to set up your plug-in project for debugging with Xcode.





12. Build and run the sample.
If you have followed the steps in QA1500, then when you click 'Build and Run' in Xcode, a new copy of Safari will be launched with the plug-in loaded.  A test heml page for the plug-in named test.html has been included in the project directory.  You can try out the plug-in by dragging and dropping test.html into window displayed by the copy of Safari that was launched as a result of the 'Build and Run' command.





Additional Information


Resources

This sample didn't call for it, but if for some reason your custom plug-in would like to obtain a reference to its bundle so it can access, say, the Resources folder inside of its bundle, then you could use the following code to do that:

		/* get a reference to your bundle */
	NSBundle *plugInBundle = [NSBundle bundleForClass:[self class]];



Install Locations

Internet Plug-ins can be installed in the following locations:

To make the plug-in available to the current user only, install it here:

	~/Library/Internet Plug-Ins/

To make the plug-in available to all users, install it here:

	/Library/Internet Plug-Ins/











