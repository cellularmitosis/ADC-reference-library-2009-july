/*
*	File:		HIATSUIView.h of TypeServicesForUnicode
* 
*	Contains:	A ready-to-use HIATSUIView which is a custom HIView
*
*					The HIATSUIView handles:
*						- getting and setting the text content
*						- getting and setting the styles
*						- getting and setting the selection
*						- various features like colors, wrapping, framing, justification, and transparency
*						- copying and dragging
*						- scrolling and auto-scrolling
*						- handling of	double-clicks, triple-clicks, quadruple=clicks,
*											click-and-drag, shift-clicking
*	
*	Version:	1.0
* 
*	Created:	11/5/04
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef UInt32 KindOfClick;
enum
	{
	kCharacterClickKind = 1,
	kWordClickKind = 2,
	kLineClickKind = 3,
	kTextClickKind = 4
	};

// Initialization Event Parameter Tags
enum
	{
	kATSUIViewTextTag       = 'Text', // CFStringRef
	kATSUIViewXLocTag       = 'xLoc', // Fixed formatted as UInt32 (Interface Builder doesn't know Fixed)
	kATSUIViewYLocTag       = 'yLoc', // Fixed formatted as UInt32 (Interface Builder doesn't know Fixed)
	kATSUIViewItemCountTag  = 'iCnt', // UInt32
	kATSUIViewRunLengthsTag = 'rLng', // UniCharCount *
	kATSUIViewStylesTag     = 'Styl', // ATSUStyle *
	kATSUIViewFrameTag      = 'Frme', // Boolean
	kATSUIViewWrapTag       = 'Wrap', // Boolean
	kATSUIViewSizeTag       = 'Size', // HISize
	};

#define kHIATSUIViewClass	CFSTR("com.apple.sample.dts.HIATSUIView")

typedef struct
	{
	HIViewRef view;							// our view

													// options:
	Boolean withFrame;						// if true draw a simple line frame, if false do nothing
	Boolean wrap;								// if true text will wrap and total height will be recalculated in
													// the field textSize below for potential scrolling
													// if false, the provided desired width and height will be stored in
													// the field textSize
	Boolean withColor;						// if true then use the color fields below
	ATSURGBAlphaColor color;				// color of text
	ATSURGBAlphaColor backColor;			// color of background

													// selection handling:
	UInt32 clickCount;						// the current click count
	KindOfClick lastKindOfClick;			// simple, by word, by line, whole text

													// Scrolling related fields:
	HIPoint originPoint;						// our moving origin
	HISize textSize;							// our virtual size

													// ATSUI related fields:
	UniCharArrayPtr theUnicodeText;		// the Text in Unicode
	UniCharCount uTextLength;				// its length
	ATSUTextMeasurement xLocation;		// where it starts drawing at x
	ATSUTextMeasurement yLocation;		//                        and y
	UniCharArrayOffset selectionStart;	// start of the selection
	UniCharArrayOffset selectionEnd;		// end of the selection (equal to selectionStart if no selection)
	ItemCount numberOfLines;				// number of lines to draw
	UniCharArrayOffset * endOfLines;		// positions of the end of line breaks
	ATSUTextMeasurement * lineHeights;	// lines heights
	ATSUTextMeasurement * descents;		// lines descents
	ItemCount numberOfRuns;					// number of style runs
	UniCharCount * runLengths;				// array of the lengths of each style run
	ATSUStyle * styles;						// array of styles
	ATSUTextLayout textLayout;				// the resulting text layout
	}
HIATSUIViewData;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

//****************************************************
#pragma mark -
#pragma mark * exported function prototypes *

// Registration and Creation
CFStringRef GetHIATSUIViewClass(void);
OSStatus HICreateATSUIView(
		const HIRect * boundsRect,					// can be NULL
		HIViewRef * outHIATSUIView);				// cannot be NULL
OSStatus HICreateATSUIViewWithText(
		const HIRect * boundsRect,					// can be NULL
		CFStringRef theCFString,
		Fixed x, Fixed y,								// can be 0
		ItemCount numberOfRuns,						// can be 0
		UniCharCount * runLengths,					// can be NULL
		ATSUStyle * styles,							// can be NULL
		Boolean withFrame,
		Boolean wrap,
		HISize * textSize,							// can be NULL
		HIViewRef * outHIATSUIView);				// can be NULL

// Setters and Getters
OSStatus HIATSUIViewSetText(HIViewRef theHIATSUIView, CFStringRef theCFString, ItemCount numberOfRuns, UniCharCount * runLengths, ATSUStyle * styles);
OSStatus HIATSUIViewSetPosition(HIViewRef theHIATSUIView, Fixed x, Fixed y);
OSStatus HIATSUIViewSetWithFrame(HIViewRef theHIATSUIView, Boolean withFrame);
OSStatus HIATSUIViewSetWrap(HIViewRef theHIATSUIView, Boolean wrap, HISize * textSize);
OSStatus HIATSUIViewSetColor(HIViewRef theHIATSUIView, Boolean withColor, float red, float green, float blue, float alpha);
OSStatus HIATSUIViewSetBackColor(HIViewRef theHIATSUIView, Boolean withColor, float red, float green, float blue, float alpha);

OSStatus HIATSUIViewBreakLines(HIViewRef theHIATSUIView);
OSStatus HIATSUIViewSetSelection(HIViewRef theHIATSUIView, UniCharArrayOffset start, UniCharArrayOffset end);
OSStatus HIATSUIViewGetSelection(HIViewRef theHIATSUIView, UniCharArrayOffset * start, UniCharArrayOffset * end);
OSStatus HIATSUIViewGetTextAndStyle(HIViewRef theHIATSUIView, UniCharArrayOffset start, UniCharArrayOffset end, CFStringRef * oTextString, void ** oStyleBuffer, ByteCount * oStyleBufferSize);
OSStatus HIATSUIViewPutContentInPasteboard(HIViewRef theHIATSUIView, PasteboardRef thePasteboard, UniCharArrayOffset start, UniCharArrayOffset end);

ATSUTextLayout HIATSUIViewGetTextLayout(HIViewRef theHIATSUIView);
ATSUStyle * HIATSUIViewGetStyles(HIViewRef theHIATSUIView, ItemCount * numberOfRuns);
HISize HIATSUIViewGetTextSize(HIViewRef theHIATSUIView);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

#ifdef __cplusplus
}
#endif
