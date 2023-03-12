/*
*	File:		HIATSUIView.c of TypeServicesForUnicode
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

#include "HIATSUIView.h"
#include "MoreATSUnicode.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

#define kXMargin IntToFixed(3)

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus Internal_HIATSUIViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static void Internal_HIATSUIViewFreeData(HIATSUIViewData * myData);
static OSStatus Internal_HIATSUIViewPointToOffset(HIATSUIViewData * myData, HIPoint hiPoint, UniCharArrayOffset * prOffset, ItemCount * lineClicked, Boolean * isLeading);
static OSStatus Internal_HIATSUIViewDetermineSelection(HIATSUIViewData * myData, KindOfClick clickKind, Boolean isLeading, Boolean extend, ItemCount lineClicked, UniCharArrayOffset anchor1, UniCharArrayOffset anchor2, UniCharArrayOffset prOffset, UniCharArrayOffset * newStart, UniCharArrayOffset * newEnd);
static HIPoint Internal_HIATSUIViewSanityCheck(HIATSUIViewData * myData, HIPoint where);
static OSStatus Internal_HIATSUIViewMakeCGImage(HIATSUIViewData * myData, UniCharArrayOffset start, UniCharArrayOffset end, CGImageRef *image, CGContextRef *outContext, HIPoint * offset);
static void Internal_HIATSUIViewFreeFunction(void *clientData, const void *data, size_t size);
static void Internal_HIATSUIViewDragSelection(HIATSUIViewData * myData, UniCharArrayOffset anchor1, UniCharArrayOffset anchor2, HIPoint mouseLoc);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* HICreateATSUIView(boundsRect, outHIATSUIView) 
*
* Purpose:  Create a HIATSUIView with minimum parameter list,
*           the HIATSUIView can be set up later using other APIs
*
* Inputs:   boundsRect          - the HIRect for our view
*           outHIATSUIView      - returning the created HIView
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus HICreateATSUIView(
		const HIRect * boundsRect,					// can be NULL
		HIViewRef * outHIATSUIView)				// cannot be NULL
	{
	OSStatus status;
	HIObjectRef hiObject;
	
	require_action(outHIATSUIView != NULL, exitCreation, status = paramErr);
	*outHIATSUIView = NULL;

	// create the view
	status = HIObjectCreate(GetHIATSUIViewClass(), 0, &hiObject);
	require_noerr(status, exitCreation);
		
	// position the view
	if (boundsRect != NULL)
		{
		status = HIViewSetFrame((HIViewRef)hiObject, boundsRect);
		require_noerr(status, exitCreation);
		}

	// return the view
	*outHIATSUIView = (HIViewRef)hiObject;

exitCreation:
	return status;
	}   // HICreateATSUIView

/*****************************************************
*
* HICreateATSUIViewWithText(boundsRect, theCFString, x, y, numberOfRuns, runLengths, styles, withFrame, wrap, textSize, outHIATSUIView) 
*
* Purpose:  Create a HIATSUIView with full parameter list,
*
* Inputs:   boundsRect          - the HIRect for our view
*           theCFString         - the text
*           x                   - x location, if 0 then a default margin of 3 pixels is used
*           y                   - y location, if 0 then the text will be positionned automatically
*           numberOfRuns        - number of styled runs, if 0 then the following parameters runLengths and styles should both be NULL
*           runLengths          - array containing the length of each run
*           styles              - array containing the style of each run
*           withFrame           - if true then a frawe will be drawn around our bounds
*           wrap                - if true then the text wraps in our bounds, else the width field of the following parameter textSize must be set to non-zero
*           textSize            - if previous parameter is false, then
*                                    on input the width field must be set with the non-zero value of the desired width,
*                                    on output the height field will be set with the value computed by HIATSUIViewBreakLines 
*           outHIATSUIView      - returning the created HIView
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
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
		HIViewRef * outHIATSUIView)				// can be NULL
	{
	OSStatus status = noErr;
	HIObjectRef hiObject = NULL;
	EventRef theInitializeEvent = NULL;

	require_action(wrap || ( textSize->width != 0 ), exitCreation, status = paramErr);

	// create the Initialize event
	status = CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), kEventAttributeUserEvent, &theInitializeEvent);
	require_noerr(status, exitCreation);

	// set all the parameters of the Initialize event
	if (boundsRect != NULL)
		SetEventParameter(theInitializeEvent, kControlCollectionTagBounds, typeHIRect,      sizeof(HIRect),          boundsRect);
	SetEventParameter(	theInitializeEvent, kATSUIViewTextTag,           typeCFStringRef, sizeof(CFStringRef),     &theCFString);
	SetEventParameter(	theInitializeEvent, kATSUIViewXLocTag,           typeUInt32,      sizeof(Fixed),           &x);
	SetEventParameter(	theInitializeEvent, kATSUIViewYLocTag,           typeUInt32,      sizeof(Fixed),           &y);
	SetEventParameter(	theInitializeEvent, kATSUIViewItemCountTag,      typeUInt32,      sizeof(ItemCount),       &numberOfRuns);
	if (runLengths != NULL)
		SetEventParameter(theInitializeEvent, kATSUIViewRunLengthsTag,     typeVoidPtr,     sizeof(UniCharCount *), &runLengths);
	if (styles != NULL)
		SetEventParameter(theInitializeEvent, kATSUIViewStylesTag,         typeVoidPtr,     sizeof(ATSUStyle *),    &styles);
	SetEventParameter(	theInitializeEvent, kATSUIViewFrameTag,          typeBoolean,     sizeof(Boolean),         &withFrame);
	SetEventParameter(	theInitializeEvent, kATSUIViewWrapTag,           typeBoolean,     sizeof(Boolean),         &wrap);
	if (textSize != NULL)
		SetEventParameter(theInitializeEvent, kATSUIViewSizeTag,           typeHISize,      sizeof(HISize),          textSize);

	// create the view with the Initialize event
	status = HIObjectCreate(GetHIATSUIViewClass(), theInitializeEvent, &hiObject);
	require_noerr(status, exitCreation);
	
	// returning the text height
	// HIATSUIViewBreakLines will be called by HIATSUIViewSetText called from the kEventHIObjectInitialize handler
	if (textSize != NULL)
		*textSize = HIATSUIViewGetTextSize((HIViewRef)hiObject);
	
exitCreation:

	// let's cleanup
	if (theInitializeEvent != NULL) ReleaseEvent(theInitializeEvent);
	if (outHIATSUIView != NULL)
		*outHIATSUIView = (HIViewRef)hiObject;

	return status;
	}   // HICreateATSUIViewWithText

/*****************************************************
*
* GetHIATSUIViewClass() 
*
* Purpose:  Registers our custom HIATSUIView view class and installs the appropriate handlers
*
* Inputs:   none
*
* Returns:  our class ID as a CFStringRef
*/
CFStringRef GetHIATSUIViewClass(void)
	{
	// following code is pretty much boiler plate.
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassScrollable, kEventScrollableGetInfo },
				{ kEventClassScrollable, kEventScrollableScrollTo },
				{ kEventClassControl, kEventControlDraw },
				{ kEventClassControl, kEventControlClick },
				{ kEventClassControl, kEventControlHitTest },
				{ kEventClassControl, kEventControlTrack },
				{ kEventClassControl, kEventControlBoundsChanged }
			};
		
		HIObjectRegisterSubclass(kHIATSUIViewClass, kHIViewClassID, 0, Internal_HIATSUIViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		}
	
	return kHIATSUIViewClass;
	}   // GetHIATSUIViewClass

/*****************************************************
*
* HIATSUIViewSetText(theHIATSUIView, theCFString, numberOfRuns, runLengths, styles) 
*
* Purpose:  Sets the text and styles of the ATSUI object
*
* Inputs:   theHIATSUIView      - the view
*           theCFString         - the text
*           numberOfRuns        - number of styled runs, if 0 then the following parameters runLengths and styles should both be NULL
*           runLengths          - array containing the length of each run
*           styles              - array containing the style of each run
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetText(HIViewRef theHIATSUIView, CFStringRef theCFString, ItemCount numberOfRuns, UniCharCount * runLengths, ATSUStyle * styles)
	{
	OSStatus status = noErr;
	
	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	// First let's get rid of any text and styles if already present
	Internal_HIATSUIViewFreeData(myData);

	// getting the characters
	myData->uTextLength = CFStringGetLength(theCFString);
	myData->theUnicodeText = (UniChar *)calloc(myData->uTextLength, sizeof(UniChar));
	require(myData->theUnicodeText != NULL, CantAllocateMemory);
	CFStringGetCharacters(theCFString, CFRangeMake(0, myData->uTextLength), myData->theUnicodeText);

	// If no style information received then we set at least 1 style
	myData->numberOfRuns = (numberOfRuns == 0) ? 1 : numberOfRuns;

	// we allocate the runLengths array and set it to either the received parameter of to end of text if none
	myData->runLengths = (UniCharCount *)calloc(myData->numberOfRuns, sizeof(UniCharCount));
	require(myData->runLengths != NULL, CantAllocateMemory);
	if (runLengths == NULL) myData->runLengths[0] = myData->uTextLength;
	else memcpy(myData->runLengths, runLengths, myData->numberOfRuns * sizeof(UniCharCount));

	// and do the same for the styles using the default style when no parameter received
	myData->styles = (ATSUStyle *)calloc(myData->numberOfRuns, sizeof(ATSUStyle));
	require(myData->styles != NULL, CantAllocateMemory);
	if (styles == NULL)
		{
		ATSUStyle tempS;
		status = ATSUCreateStyle(&tempS);
		require_noerr(status, CantCreateStyle);
		myData->styles[0] = tempS;
		}
	else memcpy(myData->styles, styles, myData->numberOfRuns * sizeof(ATSUStyle));

	// and we create the text layout
	ATSUTextLayout tempTL;
	status = ATSUCreateTextLayoutWithTextPtr(myData->theUnicodeText, 0, myData->uTextLength, myData->uTextLength, myData->numberOfRuns, myData->runLengths, myData->styles, &tempTL);
	require_noerr(status, CantCreateTextLayout);
	myData->textLayout = tempTL;

	// don't justify the last line
	status = atsuSetLayoutOptions(myData->textLayout, kATSLineLastNoJustification);
	require_noerr(status, CantCreateTextLayout);
	
	// let's ATSUI deal with characters not-in-our-specified-font
	status = ATSUSetTransientFontMatching(myData->textLayout, true);
	
	// we changed the text so we need to break it in lines
	HIATSUIViewBreakLines(theHIATSUIView);

CantCreateTextLayout:
CantCreateStyle:
CantAllocateMemory:
CantGetData:
	return status;
	}   // HIATSUIViewSetText

/*****************************************************
*
* HIATSUIViewBreakLines(theHIATSUIView) 
*
* Purpose:  Breaks the text of the ATSUI object in lines according to the desired width
*
* Inputs:   theHIATSUIView      - the view
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus HIATSUIViewBreakLines(HIViewRef theHIATSUIView)
	{
	OSStatus status = noErr;
	
	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	if (myData->uTextLength == 0) return status; // no text to break, we are probably in construction mode
	
	// Let's free any already allocated measurement-related data
	if (myData->endOfLines != NULL) { free(myData->endOfLines); myData->endOfLines = NULL; }
	if (myData->lineHeights != NULL) { free(myData->lineHeights); myData->lineHeights = NULL; }
	if (myData->descents != NULL) { free(myData->descents); myData->descents = NULL; }
	
	// Let's clear any previous soft breaks
	status = ATSUClearSoftLineBreaks(myData->textLayout, kATSUFromTextBeginning, kATSUToTextEnd);
	require_noerr(status, CantClearSoftBreakLines);

	// determining which width we want
	ATSUTextMeasurement lineWidth;
	if (myData->wrap)
		{
		HIRect bounds;
		HIViewGetBounds(theHIATSUIView, &bounds);
		lineWidth = FloatToFixed(bounds.size.width);
		if (myData->xLocation == 0) lineWidth -= 2 * kXMargin;
		else lineWidth -= ( myData->xLocation + kXMargin );
		}
	else lineWidth = FloatToFixed(myData->textSize.width);

	// allocating our arrays
	UniCharArrayOffset offset = 0, endLineOffset;
	ItemCount nbLines = 0;
	ATSUTextMeasurement ascent, descent;
	ItemCount nbAllocated = 200;
	ATSUTextMeasurement * lineHeights = (ATSUTextMeasurement *)calloc(nbAllocated, sizeof(ATSUTextMeasurement));
	require(lineHeights != NULL, CantAllocateMemory);
	ATSUTextMeasurement * descents = (ATSUTextMeasurement *)calloc(nbAllocated, sizeof(ATSUTextMeasurement));
	require(descents != NULL, CantAllocateMemory);
					
	//	line width must be set for flush and justification to work
	status = atsuSetLayoutWidth(myData->textLayout, lineWidth);				
	require_noerr(status, CantSetLineWidth);
	
	// measure and break each line, thus setting a soft line break that we'll grab back later
	while (offset < myData->uTextLength)
		{
		status = ATSUBreakLine(myData->textLayout, offset, lineWidth, true, &endLineOffset);
		if (status == kATSULineBreakInWord) status = noErr;
		require_noerr(status, CantBreakLines);
		
		// it could be that the line width is so narrow that even a single character won't fit.
		// in that case, ATSUBreakLine returns noErr but endLineOffset returns equal to offset.
		// if that's the case, let's break out...
		if (endLineOffset == offset) break;
		
		// let's get ascent & descent to calculate the line height.
		status = ATSUGetUnjustifiedBounds(myData->textLayout, offset, endLineOffset-offset, NULL, NULL, &ascent, &descent);
		require_noerr(status, CantMeasureText);

		// reallocating our arrays if they are too small
		if (nbAllocated == nbLines)
			{
			nbAllocated += 200;
			lineHeights = (ATSUTextMeasurement *)realloc(lineHeights, nbAllocated * sizeof(ATSUTextMeasurement));
			require(lineHeights != NULL, CantAllocateMemory);
			descents = (ATSUTextMeasurement *)realloc(descents, nbAllocated * sizeof(ATSUTextMeasurement));
			require(descents != NULL, CantAllocateMemory);
			}
		lineHeights[nbLines] = ascent + descent;
		if (nbLines > 0) lineHeights[nbLines] += lineHeights[nbLines-1];
		descents[nbLines] = descent;
		
		// Sanity Check, we are operating in the range 0..32767 in Fixed coordinates
		// if we determine that we overflowed, we truncate the text to the last safe offset
		if (lineHeights[nbLines] < 0)
			{
			myData->uTextLength = offset; // we stop immediately
			debug_string("Total text height is bigger than 32767!"); // and alert both the developer and the user
			DialogRef theAlert;
			CreateStandardAlert(kAlertStopAlert, CFSTR("Total text height is bigger than 32767!"), NULL, NULL, &theAlert);
			RunStandardAlert(theAlert, NULL, NULL);
			}
		else
			{
			offset = endLineOffset;
			nbLines++;
			}
		}
	
	// let's grab back all the soft line breaks in the array we'll keep around
	UniCharArrayOffset * endOfLines = (UniCharArrayOffset *)calloc(nbLines, sizeof(UniCharArrayOffset));
	require(endOfLines != NULL, CantAllocateMemory);
	ItemCount softBreakCount;
	status = ATSUGetSoftLineBreaks(myData->textLayout, 0, myData->uTextLength, nbLines, endOfLines, &softBreakCount);

	// the number of softbreaks should always be one less than the number of lines
	// since ATSUBreakLine does not insert a softbreak at the end of the text.
	require( ((status == noErr) && (softBreakCount == nbLines - 1)), CantSoftBreakLines);
	
	// so let's set the last entry of the array
	endOfLines[softBreakCount] = myData->uTextLength;

	myData->endOfLines = endOfLines;
	myData->lineHeights = lineHeights;
	myData->descents = descents;
	myData->numberOfLines = nbLines;

	// for Scrolling
	myData->textSize.width = FixedToFloat(lineWidth);
	myData->textSize.height = FixedToFloat(lineHeights[nbLines-1]);

CantSoftBreakLines:
CantMeasureText:
CantBreakLines:
CantSetLineWidth:
CantAllocateMemory:
CantClearSoftBreakLines:
CantGetData:
	return status;
	}   // HIATSUIViewBreakLines

/*****************************************************
*
* HIATSUIViewGetTextLayout(theHIATSUIView) 
*
* Purpose:  Accessor, returns the text layout of the ATSUI object
*
* Inputs:   theHIATSUIView      - the view
*
* Returns:  the text layout as an ATSUTextLayout or NULL if something went wrong
*/
ATSUTextLayout HIATSUIViewGetTextLayout(HIViewRef theHIATSUIView)
	{
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	return (myData != NULL) ? myData->textLayout : NULL;
	}   // HIATSUIViewGetTextLayout

/*****************************************************
*
* HIATSUIViewGetStyles(theHIATSUIView, numberOfRuns)
*
* Purpose:  Accessor, returns the styles array of the ATSUI object
*
* Inputs:   theHIATSUIView      - the view
*				numberOfRuns        - returns the number of styles in the array or 0 if something went wrong
*
* Returns:  the styles array or NULL if something went wrong
*/
ATSUStyle * HIATSUIViewGetStyles(HIViewRef theHIATSUIView, ItemCount * numberOfRuns)
	{
	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);

	if (myData == NULL)
		{
		numberOfRuns = 0;
		return NULL;
		}

	*numberOfRuns = myData->numberOfRuns;
	return myData->styles;
	}   // HIATSUIViewGetStyles

/*****************************************************
*
* HIATSUIViewSetPosition(theHIATSUIView, x, y)
*
* Purpose:  Sets the x, y position of the text layout in the view local coordinate system
*				Will rebreak the text in lines if needed
*
* Inputs:   theHIATSUIView      - the view
*				x                   - x coordinate in Fixed format
*				y                   - y coordinate in Fixed format
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetPosition(HIViewRef theHIATSUIView, Fixed x, Fixed y)
	{
	OSStatus status = noErr;
	
	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	// we changed the position so we need to break the text in lines again if we wrap
	Boolean needToBreak = (myData->wrap) && (myData->xLocation != x);

	myData->xLocation = x;
	myData->yLocation = y;
	
	if (needToBreak)
		HIATSUIViewBreakLines(theHIATSUIView);

CantGetData:
	return status;
	}   // HIATSUIViewSetPosition

/*****************************************************
*
* HIATSUIViewGetSelection(theHIATSUIView, start, end)
*
* Purpose:  Accessor, returns the range of the current selection
*
* Inputs:   theHIATSUIView      - the view
*				start               - returns the beginning of the selection
*				end                 - returns the end of the selection
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewGetSelection(HIViewRef theHIATSUIView, UniCharArrayOffset * start, UniCharArrayOffset * end)
	{
	OSStatus status = noErr;
	
	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	*start = myData->selectionStart;
	*end = myData->selectionEnd;

CantGetData:
	return status;
	}   // HIATSUIViewGetSelection

/*****************************************************
*
* HIATSUIViewGetTextAndStyle(theHIATSUIView, start, end, oTextString, oStyleBuffer, oStyleBufferSize)
*
* Purpose:  Returns the text and style informations in a format appropriate for later pasteboard handling
*
* Inputs:   theHIATSUIView      - the view
*				start               - the beginning of the selection
*				end                 - the end of the selection
*				oTextString         - returns the text as a CFString
*				oStyleBuffer        - returns the styles as a flattened block of bytes
*				oStyleBufferSize    - returns the size of that block of bytes
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewGetTextAndStyle(HIViewRef theHIATSUIView, UniCharArrayOffset start, UniCharArrayOffset end, CFStringRef * oTextString, void ** oStyleBuffer, ByteCount * oStyleBufferSize)
	{
	OSStatus status = noErr;
	CFStringRef textString = NULL;
	void * styleBuffer = NULL;
	ByteCount styleBufferSize = 0;
	ATSUStyleRunInfo * runInfoArray = NULL;
	ATSUStyle * styleArray = NULL;

	require_action(oTextString != NULL, ExitGetTextAndStyle, status = paramErr);
	require_action(oStyleBuffer != NULL, ExitGetTextAndStyle, status = paramErr);
	require_action(oStyleBufferSize != NULL, ExitGetTextAndStyle, status = paramErr);

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	// getting the text (that's the easy part...!)
	textString = CFStringCreateWithCharacters(NULL, &myData->theUnicodeText[start], end - start);
	require(textString != NULL, CantGetText);
	
	// arrays are allocated at maximum size
	runInfoArray = (ATSUStyleRunInfo * )malloc(myData->numberOfRuns * sizeof(ATSUStyleRunInfo));
	styleArray = (ATSUStyle * )malloc(myData->numberOfRuns * sizeof(ATSUStyle));

	// setting the content of our arrays, read QD/ATSUnicodeFlattening.h for more details
	ItemCount inr, selnr = 0;
	UniCharArrayOffset txtstart = 0, selstart = start;
	for ( inr = 0; inr < myData->numberOfRuns; inr++)
		{
		if ((selstart >= txtstart) && (selstart < txtstart + myData->runLengths[inr]))
			{
			UniCharArrayOffset minLen;
			if (txtstart + myData->runLengths[inr] > end)
				minLen = end;
			else
				minLen = txtstart + myData->runLengths[inr];
			runInfoArray[selnr].runLength = minLen - selstart;
			runInfoArray[selnr].styleObjectIndex = selnr;
			styleArray[selnr] = myData->styles[inr];
			selstart += runInfoArray[selnr].runLength;
			selnr++;
			if (selstart > end) break;
			}
		txtstart += myData->runLengths[inr];
		}

	// calling ATSUFlattenStyleRunsToStream a first time to get the size
	status = ATSUFlattenStyleRunsToStream(	kATSUDataStreamUnicodeStyledText,
														kATSUFlattenOptionNoOptionsMask,
														selnr,
														runInfoArray,
														selnr,
														styleArray,
														0,
														NULL,
														&styleBufferSize);
	require_noerr(status, CantFlatten);
	
	// allocating our style buffer with that size
	styleBuffer = malloc(styleBufferSize);
	require(styleBuffer != NULL, CantFlatten);

	// calling ATSUFlattenStyleRunsToStream a second time to set the content of the style buffer
	status = ATSUFlattenStyleRunsToStream(	kATSUDataStreamUnicodeStyledText,
														kATSUFlattenOptionNoOptionsMask,
														selnr,
														runInfoArray,
														selnr,
														styleArray,
														styleBufferSize,
														styleBuffer,
														NULL);
	require_noerr(status, CantFlatten);

CantFlatten:
CantGetText:
CantGetData:

	if (runInfoArray != NULL) free(runInfoArray);
	if (styleArray != NULL) free(styleArray);
	
	*oTextString = textString;
	*oStyleBuffer = styleBuffer;
	*oStyleBufferSize = styleBufferSize;

ExitGetTextAndStyle:

	return status;
	}   // HIATSUIViewGetTextAndStyle

/*****************************************************
*
* HIATSUIViewPutContentInPasteboard(theHIATSUIView, thePasteboard, start, end)
*
* Purpose:  Puts the content of the selection in the pasteboard which can be either the clipboard or a temporary drag
*
* Inputs:   theHIATSUIView      - the view
*				thePasteboard       - the pasteboard in which we add the data
*				start               - the beginning of the selection
*				end                 - the end of the selection
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewPutContentInPasteboard(HIViewRef theHIATSUIView, PasteboardRef thePasteboard, UniCharArrayOffset start, UniCharArrayOffset end)
	{
	OSStatus status = noErr;
	CFStringRef textString = NULL;
	void * styleBuffer = NULL;
	ByteCount styleBufferSize = 0;
	CFDataRef textData = NULL;
	UniChar * textBuffer = NULL;
	CFDataRef styleData = NULL;
	CFStringRef ustlUTString = NULL;

	// Sanity Check
	if (start == end) goto EndPutContent;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);
	
	// clearing the pasteboard first
	status = PasteboardClear(thePasteboard);
	require_noerr(status, CantClearPasteboard);

	// then synchronizing it (read the documentation for more details)
	PasteboardSyncFlags syncFlags = PasteboardSynchronize(thePasteboard);
	require_action( ! (syncFlags & kPasteboardModified), PasteboardNotSynchedAfterClear, status = badPasteboardSyncErr);
	require_action( syncFlags & kPasteboardClientIsOwner, ClientNotPasteboardOwner, status = notPasteboardOwnerErr);

	// getting our text and style information
	status = HIATSUIViewGetTextAndStyle(theHIATSUIView, start, end, &textString, &styleBuffer, &styleBufferSize);
	require_noerr(status, CantGetTextAndStyle);
	
	// setting our text in a CFData container
	CFIndex len = CFStringGetLength(textString);
	
	textBuffer = (UniChar *)malloc(len * sizeof(UniChar));
	require(textBuffer != NULL, CantCreateTextData);
	
	CFStringGetCharacters(textString, CFRangeMake(0, len), textBuffer);
	
	textData = CFDataCreate(NULL, (UInt8 *)textBuffer, len * sizeof(UniChar));
	require(textData != NULL, CantCreateTextData);
	
	// adding that CFData in the pasteboard using the adequate public UTI
	status = PasteboardPutItemFlavor(thePasteboard, (PasteboardItemID)1, CFSTR("public.utf16-plain-text"), textData, 0);
	require_noerr(status, CantPutItemFlavor);
	
	// setting our styles in a CFData container
	styleData = CFDataCreate(NULL, styleBuffer, styleBufferSize);
	require(styleData != NULL, CantCreateStyleData);
	
	// there is no public UTI for the style information so let's just use the 'ustl' OSType tag
	ustlUTString = UTTypeCreatePreferredIdentifierForTag(kUTTagClassOSType, CFSTR("ustl"), NULL);
  	require(ustlUTString != NULL, CantCreateUTForUSTL);
	
	// adding that CFData in the pasteboard
	status = PasteboardPutItemFlavor(thePasteboard, (PasteboardItemID)2, ustlUTString, styleData, 0);
	require_noerr(status, CantPutItemFlavor);

CantCreateUTForUSTL:
CantCreateStyleData:
CantPutItemFlavor:
CantCreateTextData:
CantGetTextAndStyle:
ClientNotPasteboardOwner:
PasteboardNotSynchedAfterClear:
CantClearPasteboard:
CantGetData:
EndPutContent:

	if (styleData != NULL) CFRelease(styleData);
	if (textData != NULL) CFRelease(textData);
	if (ustlUTString != NULL) CFRelease(ustlUTString);
	if (textString != NULL) CFRelease(textString);
	if (styleBuffer != NULL) free(styleBuffer);
	if (textBuffer != NULL) free(textBuffer);

	return status;
	}   // HIATSUIViewPutContentInPasteboard

/*****************************************************
*
* HIATSUIViewSetSelection(theHIATSUIView, start, end)
*
* Purpose:  Sets the text selection
*
* Inputs:   theHIATSUIView      - the view
*				start               - the beginning of the selection
*				end                 - the end of the selection
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetSelection(HIViewRef theHIATSUIView, UniCharArrayOffset start, UniCharArrayOffset end)
	{
	OSStatus status = noErr;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);
	
	if (start == kATSUFromTextBeginning) start = 0;
	if (end == kATSUToTextEnd) end = myData->uTextLength;

	myData->selectionStart = start;
	myData->selectionEnd = end;

CantGetData:
	return status;
	}   // HIATSUIViewSetSelection

/*****************************************************
*
* HIATSUIViewSetWithFrame(theHIATSUIView, withFrame)
*
* Purpose:  Sets the frame field of the view's private data to let the kEventControlDraw handler know if a frame is desired or not
*
* Inputs:   theHIATSUIView      - the view
*				withFrame           - true if a frame is desired, false if not
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetWithFrame(HIViewRef theHIATSUIView, Boolean withFrame)
	{
	OSStatus status = noErr;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	myData->withFrame = withFrame;

CantGetData:
	return status;
	}   // HIATSUIViewSetWithFrame

/*****************************************************
*
* HIATSUIViewSetWrap(theHIATSUIView, wrap, textSize)
*
* Purpose:  Sets the wrap field of the view's private data to let HIATSUIBreakLines know if we wrap the text or not.
*				If no wrap is desired then the textSize parameter is ignored, else it is kept in the view's private data.
*				Will rebreak the text in lines if needed.
*
* Inputs:   theHIATSUIView      - the view
*           wrap                - if true then the text wraps in our bounds, else the width field of the following parameter textSize must be set to non-zero
*           textSize            - if previous parameter is false, then
*                                    on input the width field must be set with the non-zero value of the desired width,
*                                    on output the height field will be set with the value computed by HIATSUIViewBreakLines 
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetWrap(HIViewRef theHIATSUIView, Boolean wrap, HISize * textSize)
	{
	OSStatus status = noErr;

	require_action(wrap || ( textSize->width != 0 ), ParamErr, status = paramErr);

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	Boolean needToBreak = (myData->wrap != wrap) || ( ( ! wrap ) && ( myData->textSize.width != textSize->width ) );

	myData->wrap = wrap;
	myData->textSize = *textSize;
	
	if (needToBreak)
		HIATSUIViewBreakLines(theHIATSUIView);
	
	*textSize = myData->textSize;

CantGetData:
ParamErr:
	return status;
	}   // HIATSUIViewSetWrap

/*****************************************************
*
* HIATSUIViewGetTextSize(theHIATSUIView)
*
* Purpose:  Accessor, returns the size of the text
*
* Inputs:   theHIATSUIView      - the view
*
* Returns:  the size of the text as a HISize
*/
HISize HIATSUIViewGetTextSize(HIViewRef theHIATSUIView)
	{
	HISize textSize = { 0, 0 };

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	textSize = myData->textSize;

CantGetData:
	return textSize;
	}   // HIATSUIViewGetTextSize

/*****************************************************
*
* HIATSUIViewSetColor(theHIATSUIView, withColor, red, green, blue, alpha)
*
* Purpose:  Sets the color information to let the kEventControlDraw handler know if color is desired or not for the text
*
* Inputs:   theHIATSUIView      - the view
*           withColor           - if true then the following color parameters will be stored in the view's private data and used later
*											 if false, then the color parameters are ignored and the default color (black) will be used for the text
*           red, green,
*				blue, alpha         - color values 
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetColor(HIViewRef theHIATSUIView, Boolean withColor, float red, float green, float blue, float alpha)
	{
	OSStatus status = noErr;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	myData->withColor = withColor;
	
	if (withColor)
		{
		myData->color.red = red;
		myData->color.green = green;
		myData->color.blue = blue;
		myData->color.alpha = alpha;
		}

CantGetData:
	return status;
	}   // HIATSUIViewSetColor

/*****************************************************
*
* HIATSUIViewSetBackColor(theHIATSUIView, withColor, red, green, blue, alpha)
*
* Purpose:  Sets the color information to let the kEventControlDraw handler know if color is desired or not for the background of the text
*
* Inputs:   theHIATSUIView      - the view
*           withColor           - if true then the following color parameters will be stored in the view's private data and used later
*											 if false, then the color parameters are ignored and the default color (clear) will be used for the text
*           red, green,
*				blue, alpha         - color values 
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetBackColor(HIViewRef theHIATSUIView, Boolean withColor, float red, float green, float blue, float alpha)
	{
	OSStatus status = noErr;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	myData->withColor = withColor;

	if (withColor)
		{
		myData->backColor.red = red;
		myData->backColor.green = green;
		myData->backColor.blue = blue;
		myData->backColor.alpha = alpha;
		}

CantGetData:
	return status;
	}   // HIATSUIViewSetBackColor

/*****************************************************
*
* HIATSUIViewSetAlignment(theHIATSUIView, alignment)
*
* Purpose:  Sets the alignment information for the kEventControlDraw handler
*
* Inputs:   theHIATSUIView      - the view
*           alignment           - Fract value (from kATSUStartAlignment (0, aka left in Roman) to kATSUEndAlignment (0x40000000L, aka right in Roman))
*												see QD/ATSUnicodeTypes.h for details
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetAlignment(HIViewRef theHIATSUIView, Fract alignment)
	{
	OSStatus status = noErr;

	// accessing our view's private data
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	status = atsuSetLayoutFlushFactor(myData->textLayout, alignment);

CantGetData:
	return status;
	}   // HIATSUIViewSetAlignment

/*****************************************************
*
* HIATSUIViewSetJustification(theHIATSUIView, justification)
*
* Purpose:  Sets the justification information for the kEventControlDraw handler
*
* Inputs:   theHIATSUIView      - the view
*           justification       - Fract value (from kATSUNoJustification (0) to kATSUFullJustification (0x40000000L))
*												see QD/ATSUnicodeTypes.h for details
*
* Returns:  OSStatus      - error code (0 == no error) 
*/
OSStatus HIATSUIViewSetJustification(HIViewRef theHIATSUIView, Fract justification)
	{
	OSStatus status = noErr;
	HIATSUIViewData* myData = (HIATSUIViewData*)HIObjectDynamicCast((HIObjectRef)theHIATSUIView, kHIATSUIViewClass);
	require(myData != NULL, CantGetData);

	status = atsuSetLayoutJustFactor(myData->textLayout, justification);

CantGetData:
	return status;
	}   // HIATSUIViewSetJustification

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* Internal_HIATSUIViewHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  Event handler that implements our HIATSUIView custom view
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
pascal OSStatus Internal_HIATSUIViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HIATSUIViewData* myData = (HIATSUIViewData*)inUserData;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					// allocate some instance data
					myData = (HIATSUIViewData*) calloc(1, sizeof(HIATSUIViewData));
					
					// get our superclass instance
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					
					// remember our superclass in our instance data
					myData->view = epView;
					// calloc fills our structure with all zeroes which is what we want a default value for all our fields
					// so we don't need to set them to NULL or 0 one by one
					
					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					break;
					}
					
#pragma mark *   kEventHIObjectInitialize
				case kEventHIObjectInitialize:
					{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					status = CallNextEventHandler(inHandlerCallRef, inEvent);
					
					// if that succeeded, do our own initialization
					if (status == noErr)
						{
						myData->color.alpha = 1;

						// retrieving all parameters set either in HICreateATSUIViewWithText or in Interface Builder
						HIRect bounds;
						status = GetEventParameter(inEvent, kControlCollectionTagBounds, typeHIRect, NULL, sizeof(bounds), NULL, &bounds);
						if (status == noErr)
							HIViewSetFrame(myData->view, &bounds);
						
						Fixed value;
						status = GetEventParameter(inEvent, kATSUIViewXLocTag, typeUInt32, NULL, sizeof(value), NULL, &value);
						if (status == noErr)
							myData->xLocation = value;
						
						status = GetEventParameter(inEvent, kATSUIViewYLocTag, typeUInt32, NULL, sizeof(value), NULL, &value);
						if (status == noErr)
							myData->yLocation = value;
						
						Boolean aBool;
						status = GetEventParameter(inEvent, kATSUIViewFrameTag, typeBoolean, NULL, sizeof(aBool), NULL, &aBool);
						if (status == noErr)
							myData->withFrame = aBool;
						else
							myData->withFrame = true;

						status = GetEventParameter(inEvent, kATSUIViewWrapTag, typeBoolean, NULL, sizeof(aBool), NULL, &aBool);
						if (status == noErr)
							myData->wrap = aBool;
						else
							myData->wrap = true;
						
						HISize theSize;
						status = GetEventParameter(inEvent, kATSUIViewSizeTag, typeHISize, NULL, sizeof(theSize), NULL, &theSize);
						if (status == noErr)
							myData->textSize = theSize;
						else
							// if there's no wrapping and a width is not provided then we return a paramErr!
							require_action(myData->wrap, InitializeExit, status = paramErr);
						
						ItemCount numberOfRuns;
						status = GetEventParameter(inEvent, kATSUIViewItemCountTag, typeUInt32, NULL, sizeof(numberOfRuns), NULL, &numberOfRuns);
						if (status != noErr) numberOfRuns = 0;
						
						UniCharCount * runLengths;
						status = GetEventParameter(inEvent, kATSUIViewRunLengthsTag, typeVoidPtr, NULL, sizeof(runLengths), NULL, &runLengths);
						if (status != noErr) runLengths = NULL;
						
						ATSUStyle * styles;
						status = GetEventParameter(inEvent, kATSUIViewStylesTag, typeVoidPtr, NULL, sizeof(styles), NULL, &styles);
						if (status != noErr) styles = NULL;
						
						CFStringRef theText;
						status = GetEventParameter(inEvent, kATSUIViewTextTag, typeCFStringRef, NULL, sizeof(theText), NULL, &theText);
						if (status == noErr)
							status = HIATSUIViewSetText(myData->view, theText, numberOfRuns, runLengths, styles);
						else
							// we didn't get any text to set but everything else is OK so we return noErr
							status = noErr;
						}

InitializeExit:
					break;
					}
					
				case kEventHIObjectDestruct:
					{
					if (myData != NULL)
						{
						Internal_HIATSUIViewFreeData(myData);
						free(myData);
						}
					status = noErr;
					break;
					}
				
				default:
					break;
				}
			break;

		case kEventClassScrollable:
			switch (GetEventKind(inEvent))
				{
#pragma mark *   kEventScrollableGetInfo
				case kEventScrollableGetInfo:
					{
					// we're being asked to return information about the scrolled view that we set as Event Parameters
					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);
					HISize lineSize = { 0, 1 };

					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(myData->textSize), &myData->textSize);
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
					SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(myData->originPoint), &myData->originPoint);
					status = noErr;
					break;
					}

#pragma mark *   kEventScrollableScrollTo
				case kEventScrollableScrollTo:
					{
					// we're being asked to scroll, we just do a sanity check and ask for a redraw if the location is different
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);

					where = Internal_HIATSUIViewSanityCheck(myData, where);

					if ((myData->originPoint.y != where.y) || (myData->originPoint.x != where.x))
						{
						myData->originPoint = where;
					
						HIViewSetNeedsDisplay(myData->view, true);
						}
					status = noErr;
					break;
					}
				
				default:
					break;
				}
			break;

		case kEventClassControl:
			switch (GetEventKind(inEvent))
				{
#pragma mark *   kEventControlDraw
				case kEventControlDraw:
					{
					CGContextRef context;
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, ControlDrawExit);

					HIRect bounds;
					HIViewGetBounds(myData->view, &bounds);

					// dealing with the colors and frame
					if (myData->withColor)
						{
						CGContextSetRGBFillColor(context, myData->backColor.red, myData->backColor.green, myData->backColor.blue, myData->backColor.alpha);
						CGContextFillRect(context, bounds);
						CGContextSetRGBFillColor(context, myData->color.red, myData->color.green, myData->color.blue, myData->color.alpha);
						}

					if (myData->withFrame)
						CGContextStrokeRect(context, bounds);

					// if there's no text to draw, we're done!
					if (myData->uTextLength == 0) goto ControlDrawExit;

					// Adjust the transform so the text doesn't draw upside down:
					// ATSUDrawText, for compatibility and legacy reasons, resets the Text Matrix to identity
					// so any pre-setting with CGContextSetTextMatrix is inoperant.
					// Since the text would be drawn upside down because QuickDraw and CoreGraphics (Quartz) have
					// opposite definitions of the direction of y-positive, we still need to reverse the
					// drawing matrix and since the Text Matrix is closed to us, we have to reverse the CTM
					// (Context Transform Matrix) itself, and translate it so that we don't draw outside the
					// visible bounds.
					CGContextSaveGState(context);
					CGContextScaleCTM(context, 1, -1);
					CGContextTranslateCTM(context, 0, myData->originPoint.y-bounds.size.height);

					// Letting ATSUI know that we draw in a CGContext instead of a QuickDraw port
					ATSUAttributeTag tags[] = { kATSUCGContextTag };
					ByteCount valueSizes[] = { sizeof(CGContextRef) };
					ATSUAttributeValuePtr values[] = { &context };
					status = ATSUSetLayoutControls(myData->textLayout, 1, tags, valueSizes, values);
					require_noerr(status, ControlDrawExit);
					
					// We start drawing at xLocation, yLocation if non-NULL,
					// else we calculate some reasonable margins
					ATSUTextMeasurement x = myData->xLocation, y = myData->yLocation;
					if (y == 0) y = FloatToFixed(bounds.size.height) - myData->lineHeights[0];
					else y = FloatToFixed(bounds.size.height) - y;
					if (x == 0) x = kXMargin;

					// Optimizing the hilighting since we are working in a CGContext
					// Note: the color in the structure ATSUUnhighlightData below is actually ignored
					//       and the actual color used for hiliting is the one chosen by the User
					//       in the System Preferences
					ATSUUnhighlightData unhilightData = {kATSUBackgroundColor, {0.0, 0.0, 0.0, 0.0}};
					status = ATSUSetHighlightingMethod(myData->textLayout, kRedrawHighlighting, &unhilightData);
					require_noerr(status, ControlDrawExit);

					// Drawing all the lines, one by one
					// Since we had to reverse and translate the y axis, we need to back-reverse and translate
					// our coordinates to simulate the QuickDraw coordinates
					ItemCount i;
					UniCharArrayOffset lineStart = 0, lineEnd;
					for (i = 0; i < myData->numberOfLines; i++)
						{
						lineEnd = myData->endOfLines[i];

						// Performance:
						// Let's draw only the lines which are visible within our bounds
						if (	(myData->lineHeights[i] >= FloatToFixed(myData->originPoint.y)) &&
								( (i == 0) || (myData->lineHeights[i-1] <= FloatToFixed(myData->originPoint.y + bounds.size.height)) )	)
							{
							// in a CGContext, we draw the hilighting first so the anti-aliasing works correctly
							// first, let's check if we might have a selection
							if (myData->selectionStart != myData->selectionEnd)
								{
								UniCharArrayOffset selStt = 0, selEnd = 0;
								if ((myData->selectionStart >= lineStart) && (myData->selectionStart <= lineEnd))
									selStt = myData->selectionStart;
								if ((myData->selectionEnd >= lineStart) && (myData->selectionEnd <= lineEnd))
									selEnd = myData->selectionEnd;
								if ((lineStart >= myData->selectionStart) && (lineStart <= myData->selectionEnd))
									selStt = lineStart;
								if ((lineEnd >= myData->selectionStart) && (lineEnd <= myData->selectionEnd))
									selEnd = lineEnd;
								if (selEnd != 0)
									{
									status = ATSUHighlightText(myData->textLayout, x, y+myData->descents[i], selStt, selEnd-selStt);
									require_noerr(status, ControlDrawExit);
									}
								}

							status = ATSUDrawText(myData->textLayout, lineStart, lineEnd-lineStart, x, y+myData->descents[i]);
							require_noerr(status, ControlDrawExit);
							}

						y -= (myData->lineHeights[i+1] - myData->lineHeights[i]);
						lineStart = lineEnd;
						}

					// restoring our context the way we found it when we started
					CGContextRestoreGState(context);

					status = noErr;
ControlDrawExit:
					break;
					}

#pragma mark *   kEventControlHitTest
				case kEventControlHitTest:
					{
					// we need to implement the kEventControlHitTest handler so that our kEventControlTrack handler will be called
					HIPoint mouseLoc;
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(mouseLoc), NULL, &mouseLoc);
					require_noerr(status, ControlHitTestExit);

					HIRect hiBounds;
					status = HIViewGetBounds(myData->view, &hiBounds);
					require_noerr(status, ControlHitTestExit);

					ControlPartCode partCode = ( CGRectContainsPoint(hiBounds, mouseLoc) ) ? kControlLabelPart : kControlNoPart;
					status = SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(partCode), &partCode);
					require_noerr(status, ControlHitTestExit);
  
ControlHitTestExit:
  					break;
					}

#pragma mark *   kEventControlClick
				case kEventControlClick:
					{
					// The kEventControlTrack event does not provide the click count as the kEventControlClick event does
					// Since a kEventControlClick event will be sent every time before a kEventControlTrack event, we just
					// store the current click count in our private data.
					myData->clickCount = 1;
					GetEventParameter(inEvent, kEventParamClickCount, typeUInt32, NULL, sizeof(myData->clickCount), NULL, &myData->clickCount);
					status = eventNotHandledErr;
					break;
					}

#pragma mark *   kEventControlTrack
				case kEventControlTrack:
					{
					// getting the location of the click
					HIPoint mouseLoc;
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(mouseLoc), NULL, &mouseLoc);
					require_noerr(status, ControlTrackExit);
					
					// getting the modifiers of the click
					UInt32 modifiers;
					status = GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
					require_noerr(status, ControlTrackExit);
					Boolean withShift = ((modifiers & shiftKey) == shiftKey);
					
					// determining where in the text we clicked
					UniCharArrayOffset prOffset;
					Boolean isLeading;
					ItemCount lineClicked;
					status = Internal_HIATSUIViewPointToOffset(myData, mouseLoc, &prOffset, &lineClicked, &isLeading);
					require_noerr(status, ControlTrackExit);

					// Let's get the current selection
					UniCharArrayOffset lastSeen = prOffset, anchor1, anchor2, newStart, newEnd;
					HIATSUIViewGetSelection(myData->view, &anchor1, &anchor2);
					
					// Now that we now where we clicked, let's change the selection according to modifiers, click count
					KindOfClick nbClicks = ( withShift ) ? myData->lastKindOfClick : myData->clickCount;

					// do we have the start of a drag?
					if ((nbClicks == 1) && (! withShift) && (prOffset >= anchor1) && (prOffset <= anchor2) && (anchor1 != anchor2))
						{
						// Making sure we are starting a drag operation
						Point globalLoc = { (int)mouseLoc.y, (int)mouseLoc.x };
						LocalToGlobal(&globalLoc);
						if (WaitMouseMoved(globalLoc))
							{
							// we do indeed have the start of a drag operation
							Internal_HIATSUIViewDragSelection(myData, anchor1, anchor2, mouseLoc);
							
							// so we need to break out the kEventControlTrack handler
							break;
							}
						}

					// determining the new selection after the first click depending on the click count and the modifiers
					Internal_HIATSUIViewDetermineSelection(myData, nbClicks, isLeading, withShift, lineClicked, anchor1, anchor2, prOffset, &newStart, &newEnd);
					if ((newStart != anchor1) || (newEnd != anchor2))
						{
						HIATSUIViewSetSelection(myData->view, newStart, newEnd);
						HIViewSetNeedsDisplay(myData->view, true);
						}
					
					// Memorize the current click count for a possible shift-click later
					if (!withShift) myData->lastKindOfClick = myData->clickCount;
					HIATSUIViewGetSelection(myData->view, &anchor1, &anchor2);
					
					KindOfClick currentKindOfClick = (myData->lastKindOfClick > myData->clickCount) ? myData->lastKindOfClick : myData->clickCount;
					
					if (myData->clickCount == kTextClickKind) break; // we already selected everything, no need to drag around...

					// click-and-dragging: using TrackMouseLocation still requires some QuickDraw usage
					Rect qdWindowBounds;
					GetWindowBounds(GetControlOwner(myData->view), kWindowStructureRgn, &qdWindowBounds);
					MouseTrackingResult mouse = kMouseTrackingMouseDown;
					while (mouse != kMouseTrackingMouseUp)
						{
						// a -1 GrafPtr to TrackMouseLocation yields global coordinates
						Point qdPoint;
						TrackMouseLocation((GrafPtr)-1L, &qdPoint, &mouse);						

						// convert to window-relative coordinates
						HIPoint hiPoint;
						hiPoint.x = qdPoint.h - qdWindowBounds.left;
						hiPoint.y = qdPoint.v - qdWindowBounds.top;
						
						// convert to view-relative coordinates
						HIViewConvertPoint(&hiPoint, NULL, myData->view);

						// determining where in the text we are now
						Internal_HIATSUIViewPointToOffset(myData, hiPoint, &prOffset, &lineClicked, &isLeading);
					
						if (prOffset != lastSeen)
							{
							lastSeen = prOffset;

							UniCharArrayOffset curStart, curEnd;
							HIATSUIViewGetSelection(myData->view, &curStart, &curEnd);
							
							// determining the new selection and asking for a refresh, we do not draw in the kEventControlTrack handler!
							Internal_HIATSUIViewDetermineSelection(myData, currentKindOfClick, isLeading, true, lineClicked, anchor1, anchor2, prOffset, &newStart, &newEnd);
							if ((newStart != curStart) || (newEnd != curEnd))
								{
								HIATSUIViewSetSelection(myData->view, newStart, newEnd);
								HIViewSetNeedsDisplay(myData->view, true);
								}
							}
						}
					
ControlTrackExit:
					break;
					}

				case kEventControlBoundsChanged:
					{
					// the bounds of our view changed, let's rebreak the lines if we wrap
					if (myData->wrap)
						HIATSUIViewBreakLines(myData->view);
					break;
					}

				default:
					break;
				}
			break;
			
		default:
			break;
		}
	
	return status;
	}   // Internal_HIATSUIViewHandler

/*****************************************************
*
* Internal_HIATSUIViewSanityCheck(myData, where) 
*
* Purpose:  Makes sure that we always scroll in a position such that we don't display out-of-bounds content
*
* Inputs:   myData              - the view's private data
*				where               - the point where we are being asked to scroll to
*
* Returns:  the point where it is safe to scroll to
*/
static HIPoint Internal_HIATSUIViewSanityCheck(HIATSUIViewData * myData, HIPoint where)
	{
	HIRect bounds;
	HIViewGetBounds(myData->view, &bounds);

	if (where.y + bounds.size.height > myData->textSize.height) 
		where.y = myData->textSize.height - bounds.size.height;
	if (where.y < 0) where.y = 0;

	if (where.x + bounds.size.width > myData->textSize.width) 
		where.x = myData->textSize.width - bounds.size.width;
	if (where.x < 0) where.x = 0;
	
	return where;
	} // Internal_HIATSUIViewSanityCheck

/*****************************************************
*
* Internal_HIATSUIViewPointToOffset(myData, hiPoint, prOffset, lineClicked, isLeading) 
*
* Purpose:  Obtains the text offset for the glyph edge nearest a given point
*
* Inputs:   myData              - the view's private data
*				hiPoint             - the point which text offset we want to know
*				prOffset            - returns the text offset
*				lineClicked         - returns the line number of that offset
*				isLeading           - returns whether the offset produced in prOffset is leading or trailing
*												see the discussion on ATSUPositionToOffset in QD/ATSUnicodeDrawing.h
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus Internal_HIATSUIViewPointToOffset(HIATSUIViewData * myData, HIPoint hiPoint, UniCharArrayOffset * prOffset, ItemCount * lineClicked, Boolean * isLeading)
	{
	OSStatus status = noErr;
	ItemCount i, found = -1;
	ATSUTextMeasurement base = 0, yLoc = myData->yLocation;

	require_action(prOffset != NULL, ParamErr, status = paramErr);
	require_action(lineClicked != NULL, ParamErr, status = paramErr);
	require_action(isLeading != NULL, ParamErr, status = paramErr);

	// determining which line was clicked since
	// we need to determine the position of the click in the coordinates of that line
	if (yLoc == 0) yLoc = FloatToFixed(hiPoint.y + myData->originPoint.y);
	else yLoc = FloatToFixed(hiPoint.y + myData->originPoint.y) - (yLoc - myData->lineHeights[0]);
	for (i = 0; (i < myData->numberOfLines) && (found == -1); i++)
		if ((base <= yLoc) && (yLoc < (base = myData->lineHeights[i])))
			found = i;

	if (found != -1)
		{
		yLoc = yLoc - base + myData->descents[found];
		ATSUTextMeasurement xLoc = myData->xLocation;
		if (xLoc == 0) xLoc = kXMargin;
		xLoc = FloatToFixed(hiPoint.x) - xLoc;

		// calling ATSUPositionToOffset to know the offset of the click in the text buffer
		UniCharArrayOffset secOffset;
		*prOffset = ( found == 0 ) ? 0 : myData->endOfLines[found-1];
		status = ATSUPositionToOffset(myData->textLayout, xLoc, yLoc, prOffset, isLeading, &secOffset);
		require_noerr(status, CantGetOffset);
		}

CantGetOffset:
	
	*lineClicked = found;

ParamErr:

	return status;
	}   // Internal_HIATSUIViewPointToOffset

/*****************************************************
*
* Internal_HIATSUIViewDetermineSelection(myData, clickKind, isLeading, extend, lineClicked, anchor1, anchor2, prOffset, newStart, newEnd) 
*
* Purpose:  Determines the new selection based on the kind of click, the previous selection, the click location, and other factors
*
* Inputs:   myData              - the view's private data
*				clickKind           - the kind of click (simple, double, triple, quadruple)
*				isLeading           - whether the offset given in prOffset is leading or trailing (see Internal_HIATSUIViewPointToOffset)
*				extend              - selection extension. If first click, true is shift key. If click-and-drag, always true.
*				lineClicked         - the line number of prOffset
*				anchor1             - the current start of the selection
*				anchor2             - the current end of the selection
*				prOffset            - the text offset of the current click
*				newStart            - returns the new start of the selection
*				newEnd              - returns the new end of the selection
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus Internal_HIATSUIViewDetermineSelection(HIATSUIViewData * myData, KindOfClick clickKind, Boolean isLeading, Boolean extend, ItemCount lineClicked, UniCharArrayOffset anchor1, UniCharArrayOffset anchor2, UniCharArrayOffset prOffset, UniCharArrayOffset * newStart, UniCharArrayOffset * newEnd)
	{
	OSStatus status = noErr;
	UniCharArrayOffset tmp1, tmp2;
	switch(clickKind)
		{
		case kCharacterClickKind:
			// simple click, we just have to check for selection extension
			if (extend)
				{
				if (prOffset < anchor1) { *newStart = prOffset; *newEnd = anchor2; }
				else if (prOffset > anchor2) { *newStart = anchor1; *newEnd = prOffset; }
				else if ( (prOffset - anchor1) < (anchor2 - prOffset) ) { *newStart = prOffset; *newEnd = anchor2; }
				else { *newStart = anchor1; *newEnd = prOffset; }
				}
			else
				{
				*newStart = *newEnd = prOffset;
				}
			break;
		case kWordClickKind:
			// let's get the end and the start of the word around the click position
			// the input position needs to be adjusted so that we don't grab the next word by mistake
			// note that leading *must* be true if thePos is the beginning of the text and false if it's the end
			status = ATSUPreviousCursorPosition(myData->textLayout, ( isLeading ) ? prOffset + 1 : prOffset, kATSUByWord, &tmp1);
			require_noerr(status, SelectionExit);
			status = ATSUNextCursorPosition(myData->textLayout, tmp1, kATSUByWord, &tmp2);
			require_noerr(status, SelectionExit);
			if (extend)
				{
				*newStart = ( tmp1 < anchor1 ) ? tmp1 : anchor1;
				*newEnd = ( tmp2 > anchor2 ) ? tmp2 : anchor2;
				}
			else
				{
				*newStart = tmp1;
				*newEnd = tmp2;
				}
			break;
		case kLineClickKind:
			// let's get the end and the start of the line around the click position
			tmp1 = ( lineClicked == 0 ) ? 0 : myData->endOfLines[lineClicked-1];
			tmp2 = myData->endOfLines[lineClicked];
			if (extend)
				{
				*newStart = ( tmp1 < anchor1 ) ? tmp1 : anchor1;
				*newEnd = ( tmp2 > anchor2 ) ? tmp2 : anchor2;
				}
			else
				{
				*newStart = tmp1;
				*newEnd = tmp2;
				}
			break;
		case kTextClickKind:
			// quadruple-click, we select the whole text unless we get a subsequent shift-click
			// which then probably means that the user wants to reduce the selection
			if (extend)
				{
				if (prOffset < anchor1) { *newStart = prOffset; *newEnd = anchor2; }
				else if (prOffset > anchor2) { *newStart = anchor1; *newEnd = prOffset; }
				else if ( (prOffset - anchor1) < (anchor2 - prOffset) ) { *newStart = prOffset; *newEnd = anchor2; }
				else { *newStart = anchor1; *newEnd = prOffset; }
				}
			else
				{
				// let's select the whole text
				*newStart = 0;
				*newEnd = myData->uTextLength;
				}
			break;
		}

SelectionExit:

	return status;
	}   // Internal_HIATSUIViewDetermineSelection

/*****************************************************
*
* Internal_HIATSUIViewFreeData(myData) 
*
* Purpose:  Frees all internal buffers and arrays of our view's private data.
*				All fields are reset to NULL in case this function is called twice.
*
* Inputs:   myData              - the view's private data
*
* Returns:  none 
*/
static void Internal_HIATSUIViewFreeData(HIATSUIViewData * myData)
	{
	if (myData->theUnicodeText != NULL) { free(myData->theUnicodeText); myData->theUnicodeText = NULL; }
	if (myData->endOfLines != NULL) { free(myData->endOfLines); myData->endOfLines = NULL; }
	if (myData->lineHeights != NULL) { free(myData->lineHeights);myData->lineHeights = NULL; }
	if (myData->descents != NULL) { free(myData->descents);myData->descents = NULL; }
	if (myData->runLengths != NULL) { free(myData->runLengths);myData->runLengths = NULL; }
	if (myData->styles != NULL)
		{
		// we need to free each ATSUStyle individually in addition to freeing the array of ATSUStyle
		// since 1 ATSUStyle might be shared by more than 1 run, we need to be careful
		// to not free the same ATSUStyle twice or more.
		ItemCount i;
		for ( i= 0; i < myData->numberOfRuns; i++)
			if (myData->styles[i] != NULL)
				{
				ItemCount j;
				for ( j = i+1; j < myData->numberOfRuns; j++)
					if (myData->styles[i] == myData->styles[j])
						myData->styles[j] = NULL;	// so that we don't try to dispose it again
				ATSUDisposeStyle(myData->styles[i]);
				}
		free(myData->styles);
		myData->styles = NULL;
		}
	if (myData->textLayout != NULL) { ATSUDisposeTextLayout(myData->textLayout); myData->textLayout = NULL; }
	}   // Internal_HIATSUIViewFreeData

/*****************************************************
*
* Internal_HIATSUIViewFreeFunction(clientData, data, size)
*
* Purpose:  Callback function that releases any private data or resources associated with the data provider (CGDataProviderReleaseInfoCallback).
*
* Inputs:   clientData          - pointer to data of any type, or NULL
*				data                - pointer to the array of data that the provider contains
*				size                - value that specifies the number of bytes that the data provider contains
*
* Returns:  none 
*/
static void Internal_HIATSUIViewFreeFunction(void *clientData, const void *data, size_t size)
	{
	free((void *)data);
	}   // Internal_HIATSUIViewFreeFunction

/*****************************************************
*
* Internal_HIATSUIViewMakeCGImage(myData, start, end, image, outContext, offset)
*
* Purpose:  Creates a CGImage that will be used when dragging text
*
* Inputs:   myData              - the view's private data
*				start               - the beginning of the selection
*				end                 - the end of the selection
*				image               - returns the CGImage
*				outContext          - returns the CGContext used by the CGImage
*				offset              - returns the point offset from the mouse to the upper left of the image (see SetDragImageWithCGImage)
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static OSStatus Internal_HIATSUIViewMakeCGImage(HIATSUIViewData * myData, UniCharArrayOffset start, UniCharArrayOffset end, CGImageRef *image, CGContextRef *outContext, HIPoint * offset)
	{
	*image = NULL;
	*outContext = NULL;
	OSStatus status = noErr;
	void * data = NULL;
	CGContextRef context = NULL;
	CGDataProviderRef cgData = NULL;
	CGImageRef cgImage = NULL;

	require_action(image != NULL, ParamErr, status = paramErr);
	require_action(outContext != NULL, ParamErr, status = paramErr);
	require_action(offset != NULL, ParamErr, status = paramErr);
	
	// determining the first visible character
	ItemCount i = 0;
	while (FloatToFixed(myData->originPoint.y) > myData->lineHeights[i]) i++;
	UniCharArrayOffset first = (i == 0) ? 0 : myData->endOfLines[i-1];
	if (start < first) start = first;
	
	// determining the top of the visible selection
	i = 0;
	while (start > myData->endOfLines[i]) i++;
	ATSUTextMeasurement top = (i == 0) ? 0 : myData->lineHeights[i-1];
	ItemCount topLine = i;
	top -= FloatToFixed(myData->originPoint.y);

	// determining the last visible character
	HIRect bounds;
	HIViewGetBounds(myData->view, &bounds);
	i = 0;
	while ( (FloatToFixed(myData->originPoint.y + bounds.size.height) > myData->lineHeights[i]) && (i < myData->numberOfLines) ) i++;
	if (i >= myData->numberOfLines) i = myData->numberOfLines - 1;
	UniCharArrayOffset last = myData->endOfLines[i];
	if (end > last) end = last;

	// determining the bottom of the visible selection
	i = 0;
	while (end >= myData->endOfLines[i]) i++;
	if (i >= myData->numberOfLines) i = myData->numberOfLines - 1;
	ItemCount bottomLine = i;
	ATSUTextMeasurement bottom = myData->lineHeights[i];
	bottom -= FloatToFixed(myData->originPoint.y);

	// determining the graphic positions (x coordinate) of the beginning and end of the selection
	SInt32 width, height = (bottom - top) >> 16;
	ATSUCaret mCaret, sCaret;
	Boolean isSplit;
	status = ATSUOffsetToPosition(myData->textLayout, start, true, &mCaret, &sCaret, &isSplit);
	Fixed startX = mCaret.fX;
	status = ATSUOffsetToPosition(myData->textLayout, end, true, &mCaret, &sCaret, &isSplit);
	Fixed endX = mCaret.fX;

	// determining the width of the dragging area
	if (topLine == bottomLine)
		{
		width = (endX - startX) >> 16;
		if (width < 0) // Right-to-Left direction
			{
			width = -width;
			endX = startX;
			startX = mCaret.fX;
			}
		offset->x = offset->x - (myData->xLocation >> 16) - (startX >> 16);
		}
	else width = (int)(bounds.size.width);

	// computing the point offset from the mouse to the upper left of the image
	offset->x = -offset->x;
	if (myData->yLocation == 0)
		offset->y = offset->y  - (top >> 16);
	else
		offset->y = offset->y - (myData->yLocation >> 16) - (top >> 16) + (myData->lineHeights[0] >> 16);
	offset->y = -offset->y;

	// create a color space
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	require(colorSpace != NULL, CantCreateColorSpace);
	
	// allocating the memory for the actual pixels
	data = malloc(width * height * 4);
	require(data != NULL, CantAllocateData);
	
	// create a bitmap context
	context = CGBitmapContextCreate(data, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedFirst);
	*outContext = context;
	require(context != NULL, CantCreateContext);

	// create a Data provider using the bitmap context's bitmap data for the drag image
	cgData = CGDataProviderCreateWithData(NULL, data, width * height * 4, Internal_HIATSUIViewFreeFunction);
	require(cgData != NULL, CantCreateDataProvider);
	
	// Create an Image using the Data Provider from the bitmap context.
	cgImage = CGImageCreate(width, height, 8, 32, width * 4, colorSpace, kCGImageAlphaFirst, cgData, NULL, false, kCGRenderingIntentDefault);
	*image = cgImage;
	CGColorSpaceRelease(colorSpace); colorSpace = NULL;
	CGDataProviderRelease(cgData); cgData = NULL;
	require(cgImage != NULL, CantCreateImage);

	// saving the context
	CGContextSaveGState(context);

	// translating the context to the selection area
	if (topLine != bottomLine)
		CGContextTranslateCTM(context, 0, -(bounds.size.height - (bottom >> 16)) + myData->originPoint.y);
	else
		CGContextTranslateCTM(context, -(startX >> 16), -(bounds.size.height - (bottom >> 16)) + myData->originPoint.y);

	// Letting ATSUI know that we draw in a CGContext instead of a QuickDraw port
	ATSUAttributeTag tags[] = { kATSUCGContextTag };
	ByteCount valueSizes[] = { sizeof(CGContextRef) };
	ATSUAttributeValuePtr values[] = { &context };
	status = ATSUSetLayoutControls(myData->textLayout, 1, tags, valueSizes, values);
	require_noerr(status, CantDraw);
	
	// We start drawing at xLocation, yLocation if non-NULL,
	// else we calculate some reasonable margins
	ATSUTextMeasurement x = myData->xLocation, y = myData->yLocation;
	if (y == 0) y = FloatToFixed(bounds.size.height) - myData->lineHeights[0];
	else y = FloatToFixed(bounds.size.height) - y;
	if (x == 0) x = kXMargin;

	// Drawing all the lines, one by one
	// Since we had to reverse and translate the y axis, we need to back-reverse and translate
	// our coordinates to simulate the QuickDraw coordinates
	UniCharArrayOffset lineStart = 0, lineEnd;
	for (i = 0; i < myData->numberOfLines; i++)
		{
		lineEnd = myData->endOfLines[i];

		// we do not want to draw any character outside of the selection
		if ((lineStart <= start) && (start <= lineEnd)) lineStart = start;
		if ((lineStart <= end) && (end <= lineEnd)) lineEnd = end;

		// Performance:
		// Let's draw only the lines which are visible within our bounds
		if (	(myData->lineHeights[i] >= FloatToFixed(myData->originPoint.y)) &&
				( (i == 0) || (myData->lineHeights[i-1] <= FloatToFixed(myData->originPoint.y + bounds.size.height)) )	)
			{
			status = ATSUDrawText(myData->textLayout, lineStart, lineEnd-lineStart, x, y+myData->descents[i]);
			require_noerr(status, CantDraw);
			}

		y -= (myData->lineHeights[i+1] - myData->lineHeights[i]);
		lineStart = lineEnd;
		}

CantDraw:

	// restoring our context the way we found it when we started
	CGContextRestoreGState(context);
	
	// if we reach here, we keep all our allocations and return 
	return status;

CantCreateImage:
CantCreateDataProvider:
CantCreateContext:
CantAllocateData:
CantCreateColorSpace:

	// if we reach here, something went wrong and we need to deallocate all temps
	if (cgImage != NULL) CGImageRelease(cgImage);
	if (cgData != NULL) CGDataProviderRelease(cgData);
	if (context != NULL) CGContextRelease(context);
	if (data != NULL) free(data);
	if (colorSpace != NULL) CGColorSpaceRelease(colorSpace);

ParamErr:

	return status;
	}   // Internal_HIATSUIViewMakeCGImage

/*****************************************************
*
* Internal_HIATSUIViewDragSelection(myData, start, end, mouseLoc)
*
* Purpose:  Creates a CGImage that will be used when dragging text
*
* Inputs:   myData              - the view's private data
*				start               - the beginning of the selection
*				end                 - the end of the selection
*				mouseLoc            - the coordinate of the click
*
* Returns:  none 
*/
static void Internal_HIATSUIViewDragSelection(HIATSUIViewData * myData, UniCharArrayOffset start, UniCharArrayOffset end, HIPoint mouseLoc)
	{
	OSStatus status = noErr;
	PasteboardRef pasteboard = NULL;
	DragRef drag = NULL;
	RgnHandle theRegion = NULL;
	CGImageRef cgImage = NULL;
	CGContextRef context = NULL;
	
	// creating a temporary pasteboard reference
	status = PasteboardCreate(kPasteboardUniqueName, &pasteboard); 
	require_noerr(status, CantCreatePasteboardForDrag);

	// putting the current selection in that pasteboard
	status = HIATSUIViewPutContentInPasteboard(myData->view, pasteboard, start, end);
	require_noerr(status, CantAddDataToTheDragPasteboard);

	// creating a drag reference with this pasteboard
	status = NewDragWithPasteboard(pasteboard, &drag); 
	require_noerr(status, CantCreateDrag);

	// the Drag Manager is still Quickdraw-based
	EventRecord convertedEvent;
	convertedEvent.what = mouseDown; 
	GetGlobalMouse(&(convertedEvent.where));
	convertedEvent.modifiers = 0;

	// we need a region even if empty
	theRegion = NewRgn(); 
	require(theRegion != NULL, CantCreateDrag);
	SetEmptyRgn(theRegion);

	// more interestingly, let's have a translucent drag with the selected text as an image
	HIPoint offset = mouseLoc;
	status = Internal_HIATSUIViewMakeCGImage(myData, start, end, &cgImage, &context, &offset);
	require_noerr(status, CantCreateDrag);

	status = SetDragImageWithCGImage(drag, cgImage, &offset, kDragRegionAndImage);
	require_noerr(status, CantCreateDrag);

	// start the drag
	status = TrackDrag(drag, &convertedEvent, theRegion);
	if (status != userCanceledErr)
		require_noerr(status, CantTrackDrag);

CantTrackDrag:
CantCreateDrag:
CantAddDataToTheDragPasteboard:
CantCreatePasteboardForDrag:

	// let's cleanup
	if (theRegion != NULL) DisposeRgn(theRegion);
	if (cgImage != NULL) CGImageRelease(cgImage);
	if (context != NULL) CGContextRelease(context);
	if (drag != NULL) DisposeDrag(drag);
	if (pasteboard != NULL) CFRelease(pasteboard);
	
	return;
	}