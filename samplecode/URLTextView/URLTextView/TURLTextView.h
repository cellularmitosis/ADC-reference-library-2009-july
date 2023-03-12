/*
    File:		TURLTextView.h
    
    Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#ifndef TURLTextView_H_
#define TURLTextView_H_

#include <Carbon/Carbon.h>

#include "TView.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------------------------------*/
//	Constants
/*------------------------------------------------------------------------------------------------------*/

/*!
	@enum
	@abstract		Get/SetControlData tags supported by the URLTextView control
	
	@constant		kControlURLTag
						Used to get or set the URL associated with the control. Data should be a CFURLRef.
						
	@constant		kControlURLTextTag
						Used to get or set the text associated with the control. Data should be a CFStringRef.
*/
enum
{
	kControlURLTag		= 'URL ',
	kControlURLTextTag	= 'cfst'
};

/*------------------------------------------------------------------------------------------------------*/
//	APIs
/*------------------------------------------------------------------------------------------------------*/

/*!
	@function		HIURLTextViewCreate
	@abstract		Creates a new URLTextView object.
	
	@discussion		A URLTextView displays a static text item with an associated URL using a browser-style
					appearance; the text is drawn in blue, and underlined. When clicked, the view tracks the
					mouse until the mouse is released, and the view uses LaunchServices to open the URL
					associated with the view.
					
					The view has two primary attributes: its URL and its text. At least one of these must
					be specified for the view to draw any content. If only the URL is specified, then the
					view draws the URL's display name, if available, or the URL itself (minus scheme) as
					its content. The view client may also specify the view's text separately, which is drawn
					instead of the URL; for example, the client might specify "http://www.mycompany.com" for
					the URL, and "MyCompany home page" for the text.
	
	@param			inBounds
						Bounds for the control. These are in coordinates relative to the view in which this
						view will be embedded.
						
	@param			inURL
						The control's URL. May be NULL.
						
	@param			inText
						The control's text. May be NULL. If no text is specified, the URL's text is used instead.
						
	@param			outURLTextView
						On exit, contains the new control.
*/
extern OSStatus
HIURLTextViewCreate( const HIRect* inBounds, CFURLRef inURL, CFStringRef inText, HIViewRef* outURLTextView );

/*!
	@function		HIURLTextViewSetURL
	@abstract		Sets the URL for a URLTextView.
	
	@param			inURLTextView
						The view.
						
	@param			inURL
						The new URL. May be NULL.
*/
extern OSStatus
HIURLTextViewSetURL( HIViewRef inURLTextView, CFURLRef inURL );

/*!
	@function		HIURLTextViewCopyURL
	@abstract		Retrieves the URL for a URLTextView.
	
	@param			inURLTextView
						The view.
						
	@param			outURL
						On exit, contains the view's current URL, or NULL if the view has no URL.
*/
extern OSStatus
HIURLTextViewCopyURL( HIViewRef inURLTextview, CFURLRef* outURL );

/*!
	@function		HIURLTextViewSetText
	@abstract		Sets the text for a URLTextView.
	
	@param			inURLTextView
						The view.
						
	@param			inText
						The new text. May be NULL.
*/
extern OSStatus
HIURLTextViewSetText( HIViewRef inURLTextView, CFStringRef inText );

/*!
	@function		HIURLTextViewCopyText
	@abstract		Retrieves the text for a URLTextView.
	
	@param			inURLTextView
						The view.
						
	@param			outText
						On exit, contains the view's current text, or NULL if the view has no text.
*/
extern OSStatus
HIURLTextViewCopyText( HIViewRef inURLTextView, CFStringRef* outText );

#ifdef __cplusplus

/*------------------------------------------------------------------------------------------------------*/
//	Classes
/*------------------------------------------------------------------------------------------------------*/

class TURLTextView
	: public TView
{
public:
	static OSStatus			Create(
								HIViewRef*					outControl,
								const HIRect*				inBounds = NULL,
								CFURLRef					inURL = NULL,
								CFStringRef					inText = NULL );

	// Accessors

protected:
	// Contstructor/Destructor
							TURLTextView(
								HIViewRef					inControl );
	virtual					~TURLTextView();

	static EventRef			CreateInitializationEvent(
								CFURLRef					inURL,
								CFStringRef					inText );
	
	virtual ControlKind		GetKind();

	virtual OSStatus		SetFocusPart(
								ControlPartCode				inDesiredFocus,
								RgnHandle					inInvalidRgn,
								Boolean						inFocusEverything,
								ControlPartCode*			inActualFocus );
	
	virtual void			Draw(
								RgnHandle					inLimitRgn,
								CGContextRef				inContext );
	virtual OSStatus		ControlHit(
								ControlPartCode				inPart,
								UInt32						inKeyModifiers );
	virtual ControlPartCode	HitTest(
								const HIPoint&				inWhere );
	virtual OSStatus		Initialize(
								TCarbonEvent&				inEvent );
	virtual OSStatus		TextInput(
								TCarbonEvent&				inEvent );
	virtual OSStatus		GetRegion(
								ControlPartCode				inPart,
								RgnHandle					outRgn );
	virtual OSStatus		GetOptimalSize(
								HISize*				outSize,
								float*				outBaseLine );
	virtual OSStatus		GetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								Size*				outSize,
								void*				inPtr );
	virtual OSStatus		SetData(
								OSType				inTag,
								ControlPartCode		inPart,
								Size				inSize,
								const void*			inPtr );

private:
	static void 			RegisterClass();
	static OSStatus			Construct(
								ControlRef					inControl,
								TView**						outView );
								
	CFStringRef				CopyText();
	HIRect					TextBounds(
								SInt16*						outBaseline );
	
	CFURLRef				fURL;
	CFStringRef				fText;
};

#endif	// __cplusplus

#ifdef __cplusplus
}
#endif

#endif // TURLTextView_H_