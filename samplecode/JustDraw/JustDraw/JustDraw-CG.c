/*
    File:	JustDraw-CG.c
    
    Version:	1.0

    Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
		("Apple") in consideration of your agreement to the following terms, and your
		use, installation, modification or redistribution of this Apple software
		constitutes acceptance of these terms.  If you do not agree with these terms,
		please do not use, install, modify or redistribute this Apple software.

		In consideration of your agreement to abide by the following terms, and subject
		to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
		copyrights in this original Apple software (the "Apple Software"), to use,
		reproduce, modify and redistribute the Apple Software, with or without
		modifications, in source and/or binary forms; provided that if you redistribute
		the Apple Software in its entirety and without modifications, you must retain
		this notice and the following text and disclaimers in all such redistributions of
		the Apple Software.  Neither the name, trademarks, service marks or logos of
		Apple Computer, Inc. may be used to endorse or promote products derived from the
		Apple Software without specific prior written permission from Apple.  Except as
		expressly stated in this notice, no other rights or licenses, express or implied,
		are granted by Apple herein, including but not limited to any patent rights that
		may be infringed by your derivative works or by other works in which the Apple
		Software may be incorporated.

		The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
		WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
		WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
		PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
		COMBINATION WITH YOUR PRODUCTS.

		IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
		CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
		GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
		ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
		OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
		(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
		ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright ¬© 2005 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>


enum {
    eSelectCGText	= 0,
    eSelectHIThemeText	= 1,
    eSelectATSUIText	= 2
};

Boolean	    gUseQD		    = false;
Boolean	    gUseHIViewCoordinates   = true;
int	    gTextDrawSelect	    = eSelectCGText;

//----------------------------------------------------------------------------------------------
static void DrawWithATSUI(CGContextRef ctx, float posX, float posY, CFStringRef str, Str255 fontName, float fontSize)
{
    size_t textLength = CFStringGetLength(str);
    UniChar* theUnicodeText = (UniChar*)calloc(textLength, sizeof(UniChar));
    CFStringGetCharacters(str, CFRangeMake(0, textLength), theUnicodeText);

    ATSUFontID atsuFontID;
    ATSUFindFontFromName(&fontName[1], fontName[0], kFontFamilyName, kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode, &atsuFontID);

    Fixed atsuSize = FloatToFixed(fontSize);
    
    ATSUAttributeTag	    theTags  [2] = { kATSUFontTag, kATSUSizeTag };
    ByteCount		    theSizes [2] = { sizeof(ATSUFontID), sizeof(Fixed) };
    ATSUAttributeValuePtr   theValues[2];
    theValues[0] = &atsuFontID;
    theValues[1] = &atsuSize;

    ATSUStyle theStyle;
    ATSUCreateStyle(&theStyle);
    ATSUSetAttributes( theStyle, 2, theTags, theSizes, theValues );

    ATSUTextLayout theLayout;
    ATSUCreateTextLayoutWithTextPtr(theUnicodeText, 0, textLength, textLength, 1, &textLength, &theStyle, &theLayout);

    // Ask ATSUI to use our ctx
    theTags  [0] = kATSUCGContextTag;
    theSizes [0] = sizeof(CGContextRef);
    theValues[0] = &ctx;
    ATSUSetLayoutControls(theLayout, 1, theTags, theSizes, theValues);

    ATSUDrawText(theLayout, kATSUFromTextBeginning, kATSUToTextEnd, FloatToFixed(posX), FloatToFixed(posY) );
    
    free(theUnicodeText);
    ATSUDisposeStyle(theStyle);
    ATSUDisposeTextLayout(theLayout);
}

//----------------------------------------------------------------------------------------------
static void DrawWithHIThemeText(CGContextRef ctx, float posX, float posY, CFStringRef str, short qdFontID, float fontSize)
{
    ThemeDrawState  state = kThemeStateActive;
    ThemeFontID	    themeFontID = kThemeCurrentPortFont;    // the only way to stay in control of fontID and fontSize - discouraged!
							    // should use e.g. kThemeApplicationFont
    HIThemeTextInfo textInfo =  {   0,		// version
				    state, 
				    themeFontID, 
				    kHIThemeTextHorizontalFlushLeft, 
				    kHIThemeTextVerticalFlushTop, 
				    kHIThemeTextBoxOptionNone,
				    kHIThemeTextTruncationNone,
				    0,
				    0 
				};
    HIThemeOrientation  orientation = (gUseHIViewCoordinates ? kHIThemeOrientationNormal : kHIThemeOrientationInverted);
    Point hv;
    SInt16 baseLine;
    
    TextFont(kFontIDGeneva);	// bad QD - no cookie!
    TextSize(fontSize);		// (ditto)
    GetThemeTextDimensions(str, themeFontID, state, false, &hv, &baseLine);
    HIRect bounds;
    if (gUseHIViewCoordinates)
	bounds = CGRectMake(posX, posY - hv.v - baseLine, hv.h, hv.v);
    else    // it really doesn't make much sense to use HIThemeDrawTextBox without using HIViewCoordinates ...
	bounds = CGRectMake(posX, posY + baseLine, hv.h, hv.v);
    HIThemeDrawTextBox(str,  &bounds, &textInfo, ctx, orientation);
}

//----------------------------------------------------------------------------------------------
static void JustDrawSomething(CGContextRef ctx, HIRect viewBounds)
{
    Str255 s = "\pHello World!";
    Str255 fontName = "\pGeneva";
    
    if (gUseQD)
    {
	// HIViews have set up the port for us, already - just draw!
	MoveTo(0, 0);
	LineTo(100, 100);
	TextFont(kFontIDGeneva);
	TextSize(18);
	DrawString(s);
    }
    else
    {
	if (gUseHIViewCoordinates)
	{
	    // We can keep the hard-coded QD coordinates
	    CGContextMoveToPoint(ctx, 0, 0);
	    CGContextAddLineToPoint(ctx, 100, 100);
	}
	else // our coordinates need to be recalculated for the default Quartz coordinates
	{
	    CGContextMoveToPoint(ctx, 0, viewBounds.size.height);
	    CGContextAddLineToPoint(ctx, 100, viewBounds.size.height - 100);
	}
	
	CGContextStrokePath(ctx);   // This draws the line
	
	// And how about replacing the DrawString()?
	// First, convert to a CFString, once and for all
	CFStringRef str = CFStringCreateWithPascalString(NULL, s, kCFStringEncodingMacRoman);
	switch (gTextDrawSelect)
	{
	    case eSelectCGText:
		fontName[fontName[0] + 1] = 0;	// make (char*)&fontName[1] a C-string
		CGContextSelectFont(ctx, (char*)&fontName[1], 18, kCGEncodingMacRoman);
 		if (gUseHIViewCoordinates)
		{
		    // Compensate in the text matrix for the inverted y-axis
		    CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));
		    CGContextShowTextAtPoint(ctx, 100, 100, (char*)&s[1], s[0]);
		    // Set it back - the text matrix is NOT part of the GState,
		    // and we might get confused if we left the text mirroring around.
		    CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);
		}
		else  // the text is drawn correctly, but the coordinates need to be recalculated
		{
		    CGContextShowTextAtPoint(ctx, 100, viewBounds.size.height - 100, (char*)&s[1], s[0]);
		}
	    break;
	    
	    case eSelectHIThemeText:
 		if (gUseHIViewCoordinates)
		    DrawWithHIThemeText(ctx, 100, 100, str, kFontIDGeneva, 18);
		else  // it really doesn't make much sense to use HIThemeDrawTextBox without using HIViewCoordinates ...
		    DrawWithHIThemeText(ctx, 100, viewBounds.size.height - 100, str, kFontIDGeneva, 18);
	    break;
	    
	    case eSelectATSUIText:
		if (gUseHIViewCoordinates)  // ATSUI cannot deal with the flipped HIView coordinate system;
		{			    // it always sets the text matrix to identity - we need to work around with the CTM 
					    // and consequently use recalculated coordinates, no matter what
		    CGContextSaveGState(ctx);
		    CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, viewBounds.size.height));
		    DrawWithATSUI(ctx, 100, viewBounds.size.height - 100, str, fontName, 18);
		    CGContextRestoreGState(ctx);    // this resets the CTM to what it was
		}
		else
		{
		    DrawWithATSUI(ctx, 100, viewBounds.size.height - 100, str, fontName, 18);
		}
	    break;
	}
	CFRelease(str);
    }
}

//----------------------------------------------------------------------------------------------
// We want to show the current global settings for the drawing in the window title.
// Unfortunately, it appears that calling SetWindowTitle from within a Draw handler doesn't
// work reliably (rdar://4123251). We are working around the problem by calling it
// from a one-shot EventLoopTimer, which only fires after all composited drawing is finished.
static void ShowGlobalsInWindowTitle(WindowRef w)
{
    Str255 qdString = "\pQD - DrawString";
    Str255 cgString = "\pCG - ";
    Str255 cgText = "\pCGContextShowText";
    Str255 hiText = "\pHIThemeDrawText";
    Str255 atsuiT = "\pATSUIDrawText";
    Str255 hiviewC = "\p - HIViewCoords";
    Str255 title;
    char* p;
    size_t n = 0;
    
    if (gUseQD)
	memcpy(title, qdString, qdString[0] + 1);
    else
    {
    	memcpy(title, cgString, cgString[0] + 1);
	switch (gTextDrawSelect)
	{
	    case eSelectCGText:		p = (char*)&cgText[1]; n = cgText[0];   break;
	    case eSelectHIThemeText:    p = (char*)&hiText[1]; n = hiText[0];   break;
	    case eSelectATSUIText:	p = (char*)&atsuiT[1]; n = atsuiT[0];   break;
	}
	memcpy(&title[title[0] + 1], p, n);
	title[0] += n;
	if (gUseHIViewCoordinates)
	{
	    memcpy(&title[title[0] + 1], &hiviewC[1], hiviewC[0]);
	    title[0] += hiviewC[0];
	}
    }
    CFStringRef str = CFStringCreateWithPascalString(NULL, title, kCFStringEncodingMacRoman);    
    SetWindowTitleWithCFString(w, str);
    CFRelease(str);
}

//----------------------------------------------------------------------------------------------
static void OneShotTimer(EventLoopTimerRef inTimer, void* inRefCon)
{
    ShowGlobalsInWindowTitle((WindowRef)inRefCon);
    RemoveEventLoopTimer(inTimer);
}

//----------------------------------------------------------------------------------------------
OSStatus DrawHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    CGContextRef ctx;
    GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx );

    CallNextEventHandler(inHandlerCallRef, inEvent);    // this erases the view for us
    
    HIRect	viewBounds;
    HIViewRef   theView = (HIViewRef)inUserData; 
    HIViewGetBounds(theView, &viewBounds);

    if (gUseHIViewCoordinates == false)	    // i.e. we want to have the default Quartz coordinate system
    {
	CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, viewBounds.size.height));
	// Now the origina is at the bottom-left of the view, and the y axis goes upwards.
    }
    
    JustDrawSomething(ctx, viewBounds);
    
    // See comment to ShowGlobalsInWindowTitle, above
    InstallEventLoopTimer(GetCurrentEventLoop(), 0, 0, OneShotTimer, GetControlOwner(theView), NULL);
    
    return noErr;
}

//----------------------------------------------------------------------------------------------
OSStatus KeyHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
    UInt8 ch;
    OSStatus err = GetEventParameter( inEvent, 
					kEventParamKeyMacCharCodes, 
					typeChar,
					NULL,
					sizeof( char ), 
					NULL, 
					&ch );
    if (err == noErr)
    {
	if (ch == '0')
	    gTextDrawSelect = eSelectCGText;
	else if (ch == '1')
	    gTextDrawSelect = eSelectHIThemeText;
	else if (ch == '2')
	    gTextDrawSelect = eSelectATSUIText;
	else if (ch == 'h')
	    gUseHIViewCoordinates = !gUseHIViewCoordinates;
	else
	    // any other key just toggles between QD and CG
	    gUseQD = !gUseQD;
	
	WindowRef w = (WindowRef)inUserData;
	HIViewRef contentView;
	HIViewFindByID( HIViewGetRoot(w), kHIViewWindowContentID, &contentView );
	HIViewSetNeedsDisplay(contentView, true);
    }
    return err;
}
//----------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		window;
    
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    ///////////////////////////////////////////////////////////////////
    // Install DrawHandler on contentView
    EventTypeSpec   drawEvent = {kEventClassControl, kEventControlDraw};
    EventTypeSpec   keyEvent = {kEventClassKeyboard, kEventRawKeyDown};
    HIViewRef	    contentView;
    HIViewFindByID( HIViewGetRoot(window), kHIViewWindowContentID, &contentView );
    err = InstallEventHandler(HIViewGetEventTarget(contentView), DrawHandler, 1, &drawEvent, contentView, NULL);
    require_noerr( err, CantInstallHandler );
    // Also install KeyHandler on the window so we can control the drawing
    err = InstallEventHandler(GetWindowEventTarget(window), KeyHandler, 1, &keyEvent, window, NULL);
    require_noerr( err, CantInstallHandler );
    ///////////////////////////////////////////////////////////////////

    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

CantInstallHandler:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

