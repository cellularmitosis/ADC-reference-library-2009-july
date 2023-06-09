/*

File: Extras-2.c

Abstract:  Extra code & information regaring the 2nd portion of the sample,
           but not included in the main project.

� Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

*/



// ---------------------------------------------------------------------------
// To create an HIScrollView programmatically, use the following call:

HIViewRef scrollView, textView;
OptionBits options;
OSStatus status;

//...

// Create the ScrollView
options = kHIScrollViewOptionsVertScroll;
status = HIScrollViewCreate(options, &scrollView);



// ---------------------------------------------------------------------------
// To embed an HITextView in another view (such as the ScrollView), use the
// following code:

HIViewRef scrollView, textView;
OSStatus status;

//...

// and embed the TextView in it
status = HIViewAddSubview(scrollView, textView);



// ---------------------------------------------------------------------------
// To make any HIView auto-size along with its parent view, use the following
// code.
//
// Note that in our example, the ScrollView's layout is bound to the root view
// of the window, and the HITextView (a subview of the ScrollView) has its
// layout bound to it for resize.

HIViewRef aView;
HILayoutInfo layoutInfo;
HIRect parentFrame;

//...

// Bind the view to its parent on all sides (affects resize)
layoutInfo.binding.left.kind = kHILayoutBindLeft;
layoutInfo.binding.left.toView = NULL;
layoutInfo.binding.left.offset = 0.0;
layoutInfo.binding.top.kind = kHILayoutBindTop;
layoutInfo.binding.top.toView = NULL;
layoutInfo.binding.top.offset = 0.0;
layoutInfo.binding.right.kind = kHILayoutBindRight;
layoutInfo.binding.right.toView = NULL;
layoutInfo.binding.right.offset = 0.0;
layoutInfo.binding.bottom.kind = kHILayoutBindBottom;
layoutInfo.binding.bottom.toView = NULL;
layoutInfo.binding.bottom.offset = 0.0;
status = HIViewSetLayoutInfo(aView, &layoutInfo);

// Set the initial size to be the same as the parent
status = HIViewGetFrame(HIViewGetSuperview(aView), &parentFrame);
status = HIViewSetFrame(aView, &parentFrame);
