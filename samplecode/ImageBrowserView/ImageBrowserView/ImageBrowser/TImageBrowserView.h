/*
    File:		TImageBrowserView.h
    
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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/


#include <Carbon/Carbon.h>
#include "TView.h"

class TImageBrowserView : TView
{
	public:
		
		// Registers the custom TImageBrowserView class
		static OSStatus			RegisterClass( void );
		
		// Creates a TImageBrowserView instance
		static OSStatus			Create(
									WindowRef			inWindow,		/* can be NULL */
									const HIRect*		inBounds,
									CFArrayRef			inImageURLs,	/* can be NULL */
									HIViewRef*			outView );
		
		// Adds images to the view
		static OSStatus			AddImages(
									HIViewRef			inView,
									CFArrayRef			inImageURLs );
	
	private:
		
		// Class constructor
		static OSStatus			Construct(
									HIObjectRef			inObjectRef,
									TObject**			outObject );
		
		// Instance methods
		TImageBrowserView(			HIViewRef			inView );
		
		~TImageBrowserView();
		
		virtual ControlKind		GetKind();
		
		virtual OSStatus		Initialize(
									TCarbonEvent&		inEvent );
		
		virtual OSStatus		Encode(
									HIArchiveRef		inEncoder );

		
		virtual void			Draw(
									RgnHandle			inLimitRgn,
									CGContextRef		inContext );
									
		virtual ControlPartCode	HitTest(
									const HIPoint&		inWhere );
									
		virtual OSStatus		GetRegion(
									ControlPartCode		inPart,
									RgnHandle			outRgn );
		
		virtual OSStatus		Track(
									TCarbonEvent&		inEvent,
									ControlPartCode*	outPartHit );
		
		virtual OSStatus		ControlHit(
									ControlPartCode		inPart,
									UInt32				inModifiers );
		
		virtual OSStatus		SetFocusPart(
									ControlPartCode		inDesiredFocus,
									Boolean				inFocusEverything,
									ControlPartCode*	outActualFocus );
		
		virtual OSStatus		GetFocusPart(
									HIViewPartCode*		outCurrentFocusPart );
		
		virtual void			ActiveStateChanged();
		
		virtual void			HiliteChanged(
									ControlPartCode		inOriginalPart,
									ControlPartCode		inCurrentPart );
		
		virtual bool			DragEnter(
									DragRef				inDrag );
		
		virtual bool			DragLeave(
									DragRef				inDrag );
		
		virtual OSStatus		DragReceive(
									DragRef				inDrag );
		
		virtual OSStatus		TrackingAreaEvent(
									bool				inIsEnterEvent );
		
		virtual OSStatus		TextInput(
									TCarbonEvent&		inEvent );
		
		virtual OSStatus		CopyAccessibleChildAtPoint(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									const HIPoint&		inWhere,
									CFTypeRef*			outChild );
		
		virtual OSStatus		CopyAccessibleFocusedChild(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFTypeRef*			outChild );
		
		virtual OSStatus		GetAccessibleAttributeNames(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFMutableArrayRef	outNames );
		
		virtual OSStatus		GetAccessibleNamedAttribute(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFStringRef			inAttribute );
		
		virtual OSStatus		GetAccessibleActionNames(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFMutableArrayRef	outNames );
		
		virtual OSStatus		PerformAccessibleNamedAction(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFStringRef			inName );
		
		virtual OSStatus		CopyAccessibleNamedActionDescription(
									EventHandlerCallRef	inCallRef,
									EventRef			inEvent,
									AXUIElementRef		inElement,
									UInt64				inIdentifier,
									CFStringRef			inName,
									CFStringRef*		outDescription );
		
		virtual void			OwningWindowChanged(
									WindowRef			oldWindow,
									WindowRef			newWindow );
		
		virtual OSStatus		HandleEvent(
									EventHandlerCallRef	inCallRef,
									TCarbonEvent&		inEvent );
		
		// Helper routines
		HIRect					GetPartFrame(
									HIViewPartCode		inPart );
		
		bool					IsPartAvailable(
									HIViewPartCode		inPart );
		
		CFURLRef				CreateFileURLFromPasteboard(
									PasteboardRef		inPasteboard,
									CFIndex				inIndex );
		
		void					InvalidateImageCache( void );
		
		void					PreviousImage( void );
		
		void					NextImage( void );
		
		void					AddImage(
									CFURLRef			inFileURL );
		
		void					DeleteCurrentImage( void );
		
		void					ConvertImageToBitmapImage(
									CGImageRef*			ioImage );
		
		HIPoint*				ConvertGlobalToLocalPoint(
									const HIPoint&		inPoint,
									HIPoint*			outPoint );
		
		HIPoint*				ConvertLocalToGlobalPoint(
									const HIPoint*		inPoint,
									HIPoint*			outPoint );
		
		HIViewPartCode			GetViewPart(
									const HIPoint&		inPoint );
		
		CFMutableArrayRef		fImageURLs;
		CFIndex					fImageIndex;
		bool					fTrackingDrag;
		bool					fMouseInView;
		HIViewTrackingAreaRef	fTrackingArea;
		HIViewPartCode			fCurrentFocusPart;
		CGImageRef				fImageCache;
		CGImageRef				fImageCacheDisabled;
		CFIndex					fImageCacheIndex;
};

