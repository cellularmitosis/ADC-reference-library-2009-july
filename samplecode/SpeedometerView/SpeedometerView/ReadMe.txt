File: ReadMe.txt

Abstract: readme file for the SpeedometerView sample.

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





This sample illustrates how to make a custom NSView that does some drawing and responds to mouse clicks.  The SpeedometerView class is a subclass of NSView and it is defined in the files SpeedometerView.h and SpeedomenterView.m.  The files SpeedyCategories.h and SpeedyCategories.m define some helper categories that are used by the view.  



To use this custom NSView in a project of your own, follow these steps:


1. Add the files defining the custom view to your project in Xcode.  In this case, we add the files SpeedometerView.h, SpeedomenterView.m, SpeedyCategories.h, SpeedyCategories.m to our project.


2. Open the nib file in interface builder where you would like to use the custom view.


3. Next, register the custom view with Interface Builder.  To do that, drag and drop the .h file containing the custom view into the nib file's window in Interface builder.  In this case, we drag the file SpeedometerView.h in Xcode into the MainMenu.nib file's window in Interface Builder.


4. Drag a CustomView from the Cocoa-Containers palette into a window.  


5. Select the CustomView in the window and open the inspector to display the Custom Class panel.


6. In the Custom Class panel in the inspector, select the name of your custom NSView class.  In this case, we have selected 'SpeedometerView' as our custom class.


These same steps can be used to add your own custom NSViews into your application's windows.


This sample uses bindings to manage most of its user interface.  As such, there is no code in the project for maintaining these items.  Here is a summary of the bindings that are in place:


1. The ticks slider's value has a binding to the custom view's ticks value.  The slider's attributes are set to continually update as the value is changed.  

2. The curve slider is bound the custom view's curvature value.  The slider's attributes are set to continually update as the value is changed.

3. The speed slider is bound to the custom view's speed value.  The slider's attributes are set to continually update as the value is changed. 

4. The little progress indicator is bound to the draggingIndicator value on the custom view.  The little indicator shows up while the speedometer pointer is being dragged.  


