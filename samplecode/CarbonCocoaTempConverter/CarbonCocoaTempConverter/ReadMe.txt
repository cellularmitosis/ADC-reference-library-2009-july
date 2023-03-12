 File: ReadMe.txt
 
 Abstract: ReadMe file for the CarbonCocoaTempConverter sample.
 
 Version: 1.1
 
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


"CarbonCocoaTempConverter" demonstrates how to integrate Carbon and Cocoa user interfaces in the same Cocoa application. Both user interfaces allow users to convert Fahrenheit temperatures to Celsius. This sample implements routines that listens for and handle events in the Carbon window. 


Listening for Events in the Carbon Window
This sample uses the Carbon InstallWindowEventHandler macro to listen for events in the Carbon window. This macro calls the "CarbonWindowCommandHandler"  routine whenever it detects command events in the Carbon window. These events are generated whenever users click in the window. See below for an example:

InstallWindowEventHandler(window,NewEventHandlerUPP(CarbonWindowCommandHandler),1, &commSpec, (void *) window, NULL);


Handling Events in the Carbon Window
This sample defines a command ID kConvertCommand, which is associated with the "Convert" button in the Carbon window. 
The "CarbonWindowCommandHandler" routine is called as soon as InstallWindowEventHandler receives a command event. This routine checks the value of the received command event and calls "ConvertCommandHandler" if the command ID of the received event is equal to kConvertCommand. 


Converting Fahrenheit Temperatures to Celsius
This sample uses the "fahrenheitToCelsius method" to convert Fahrenheit temperatures to Celsius when users click the "Convert" button in the Cocoa window.
"CarbonCocoaTempConverter" uses the "ConvertCommandHandler" routine to perform the above operation when users click the "Convert" button in the Carbon window.


Using the Sample 
Open this sample in Mac OS X 10.4 and later using Xcode.  
Click the Build and Go toolbar item in the project window to compile and run this application.
Enter a number in the fahrenheit textfield in either Carbon or Cocoa window when they appear. Click the "Convert" button to get a Celsius value. 


Changes from Previous Versions
Updated the ReadMe file.
Removed the precompiledheader.h file.
Cleaned up code and added comments.
Added the init and dealloc methods to the MyController.m.
Renamed the convertTemp IBAction method to fahrenheitToCelsius. 
Upgraded sample so it can run on both PowerPC and Intel-based Macintoshes.


Further Reading
Introduction to Carbon-Cocoa Integration Guide 
<http://developer.apple.com/documentation/Cocoa/Conceptual/CarbonCocoaDoc/CarbonCocoaDoc.html>

Creating and Registering an Event Handler 
<http://developer.apple.com/documentation/Carbon/Conceptual/Carbon_Event_Manager/Tasks/chapter_3_section_4.html>

Command Events
<http://developer.apple.com/documentation/Carbon/Conceptual/Carbon_Event_Manager/Tasks/chapter_3_section_8.html>


Feedback and Bug Reports
Please send all feedback about this sample by using the Feedback form on the bottom of the sample's webpage. Please submit any bug reports about this sample to the Bug Reporting <http://developer.apple.com/bugreporter> page.