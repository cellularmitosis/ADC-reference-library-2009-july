/*
	File:		LabelItemView.cp

    Version:	Mac OS X

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

	Copyright © 2003 Apple Computer, Inc., All Rights Reserved
*/

#include "LabelItemView.h"
#include "TView.h"


//======================================================================================
//	• Constants
//======================================================================================

#define kTViewHILabelViewClassID	CFSTR( "com.apple.sample.HILabelView" )		// HILabelView class ID

#define kRolloverTimerDelay 0.1		// delay before the rollover timer fires

enum
{
	kLabelNameHeight	= 15,	// height allowed for the label name
	kLabelPartWidth		= 17,	// width of the entire label part
	kLabelPartHeight	= 16,	// height of the entire label part
	kFirstLabelSpace	= 5		// extra space between the "none" part and the first label part
};

// menu item property constants
enum
{
	kMenuPropertyLabelViewCreator	= 'Labl',		// used for all LabelView properties
	kMenuPropertyLabelViewTag		= 'LbVw'		// an HILabelView*
};

//======================================================================================
//	• Types
//======================================================================================

class HILabelView : public TView
{
	public:
		static OSStatus			Create( MenuRef inMenu, MenuItemIndex inMenuItem, HIViewRef inMenuContentView, HIViewRef* outView );
		
	protected:
								HILabelView( HIViewRef inControl );
								~HILabelView();
		
		ControlKind				GetKind();
		
		OSStatus				Initialize( TCarbonEvent& inEvent );
		
		OSStatus				ControlHit( HIViewPartCode inPart, UInt32 inModifiers );
		
		void					Draw( RgnHandle inLimitRgn, CGContextRef inContext );
		
		OSStatus				GetRegion( ControlPartCode inPart, RgnHandle outRgn );
		HIViewPartCode			HitTest( const HIPoint& inWhere );
		OSStatus				Track( TCarbonEvent& inEvent, ControlPartCode* outPartHit );
		
		OSStatus				GetSizeConstraints( HISize* outMin, HISize* outMax );
		OSStatus				GetOptimalSizeSelf( HISize* outSize, float* outBaseLine );
		
		void					VisibilityChanged();
		
	private:
		enum { kNumParts = 8 };		// seven label buttons, plus the "none" button
		
		static const EventTypeSpec	kControlEvents[];
		static const EventTypeSpec	kWindowEvents[];
		static const EventTypeSpec	kMenuEvents[];
		static const EventTypeSpec	kMenuContentEvents[];
		static CGImageRef			sImages[ kNumParts ];
		static CGImageRef			sSelectedBackgroundImage;
		static CGImageRef			sPressedBackgroundImage;
		static CFStringRef			sTitleString;
		static CFStringRef			sLabelStrings[ 7 ];
									
		MenuRef					fMenu;					// menu that owns this label item view
		MenuItemIndex			fMenuItem;				// menu item that is being used to draw this label item view
		HIViewRef				fMenuContentView;		// menu content view in which we are embedded
		EventHandlerRef			fMenuHandler;			// event handler installed on the menu
		EventHandlerRef			fMenuContentHandler;	// event handler installed on the menu content view
		EventHandlerRef			fWindowHandler;			// event handler installed on our owning window
		EventLoopTimerRef		fRolloverTimer;			// event loop timer used to avoid flicker
		HIViewPartCode			fRollover;				// the item part that is currently showing rollover effects
		HIViewPartCode			fTextRollover;			// during SimulateHit, the item part that shows rollover for its text
		HIViewPartCode			fPartRollover;			// during SimulateHit, the item part that shows rollover for its part
		HIViewPartCode			fNewRollover;			// the item part that the mouse has just moved over
		OptionBits				fSelected;				// which labels are selected
		int						fLeftEdge;				// x offset from left side of view to left edge of text and label parts
		float					fPartYOffset;			// y offset from top of label view to top of label parts
		
		static OSStatus			Construct( HIObjectRef inObjectRef, TObject** outObject );
		static pascal OSStatus	EventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
		static pascal void		RolloverChangedTimer( EventLoopTimerRef inTimer, void* inRefcon );
		static CFStringRef		GetTitleString();
		static CFStringRef		GetLabelString( int inPart );
		static OSStatus			LoadPartImages();
		static int				LabelIndexFromPart( int inPart );
		static CGImageRef		GetPartImage( int inPart );
		static CGImageRef		GetSelectedBackgroundImage();
		static CGImageRef		GetPressedBackgroundImage();
		static CGImageRef		GetBackgroundImage( CFStringRef inName, CGImageRef* ioImage );

		MenuItemIndex			FindItem();
		HILabelView*			GetMenuItemViewProperty( MenuItemIndex inItem );
		void					RedrawWithDelay( int inDelayTicks );
		void					DrawCenteredImage( CGContextRef inContext, CGPoint inCenter, CGImageRef inImage );
		void					GetTitleBounds( HIRect* outBounds );
		void					GetLabelBounds( HIRect* outBounds );
		void					GetPartBounds( int inPart, HIRect* outBounds );
		int						GetLeftEdge();
		
		void					ClearRollover()
								{
									fRollover = fTextRollover = fPartRollover = fNewRollover = 0;
								}
};

static void		_HIViewConvertFromGlobalPoint( HIPoint* ioPoint, HIViewRef inDestView );

//======================================================================================
//	• Globals
//======================================================================================

const EventTypeSpec		HILabelView::kControlEvents[] =
						{
							{ kEventClassControl, kEventControlVisibilityChanged }
						};
const EventTypeSpec		HILabelView::kWindowEvents[] =
						{
							{ kEventClassMouse, kEventMouseMoved },
							{ kEventClassMouse, kEventMouseDragged }
						};
const EventTypeSpec		HILabelView::kMenuEvents[] =
						{
							{ kEventClassMenu, kEventMenuMeasureItemWidth },
							{ kEventClassMenu, kEventMenuMeasureItemHeight },
							{ kEventClassMenu, kEventMenuDrawItem },
							{ kEventClassMenu, kEventMenuCalculateSize },
							{ kEventClassCommand, kEventCommandProcess },
							
							/*
								These are currently required to support rollover effects in contextual menus.
								For pull-down menus, we get these events by registering on the menu window;
								however, the WWDC build of HIToolbox sends these events to the clicked window
								rather than to the menu window for contextual menus, so we need to register
								for them on the menu as well. This problem is fixed in HIToolbox for the post-
								WWDC builds, and once those builds are available, these handlers can be removed.
							*/
							{ kEventClassMouse, kEventMouseMoved },
							{ kEventClassMouse, kEventMouseDragged }
						};
const EventTypeSpec		HILabelView::kMenuContentEvents[] =
						{
							{ kEventClassControl, kEventControlSimulateHit }
						};
CGImageRef				HILabelView::sImages[];
CGImageRef				HILabelView::sSelectedBackgroundImage;
CGImageRef				HILabelView::sPressedBackgroundImage;
CFStringRef				HILabelView::sTitleString;
CFStringRef				HILabelView::sLabelStrings[];


//======================================================================================
//	• Functions and methods
//======================================================================================

//——————————————————————————————————————————————————————————————————————————————————————
//	• HILabelViewCreate
//	Creates a new instance of the label item view.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelViewCreate( MenuRef inMenu, MenuItemIndex inMenuItem, HIViewRef inMenuContentView, HIViewRef* outView )
{
	return HILabelView::Create( inMenu, inMenuItem, inMenuContentView, outView );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• HILabelView constructor
//——————————————————————————————————————————————————————————————————————————————————————
HILabelView::HILabelView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fMenu( NULL ),
		fMenuItem( 0 ),
		fMenuContentView( NULL ),
		fMenuHandler( NULL ),
		fMenuContentHandler( NULL ),
		fWindowHandler( NULL ),
		fRolloverTimer( NULL ),
		fRollover( 0 ),
		fTextRollover( 0 ),
		fPartRollover( 0 ),
		fNewRollover( 0 ),
		fSelected( 0 ),
		fLeftEdge( 0 ),
		fPartYOffset( 0 )
{
	// add autoinvalidation
	ChangeAutoInvalidateFlags(	kAutoInvalidateOnHilite   |
								kAutoInvalidateOnActivate |
								kAutoInvalidateOnEnable,
								0 );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• HILabelView destructor
//——————————————————————————————————————————————————————————————————————————————————————
//
HILabelView::~HILabelView()
{
	if ( fMenuHandler != NULL )
		RemoveEventHandler( fMenuHandler );
	if ( fMenuContentHandler != NULL )
		RemoveEventHandler( fMenuContentHandler );
	if ( fWindowHandler != NULL )
		RemoveEventHandler( fWindowHandler );
	if ( fRolloverTimer != NULL )
		RemoveEventLoopTimer( fRolloverTimer );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• Create
//	Public class method for creating a HILabelView.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::Create(
	MenuRef			inMenu,
	MenuItemIndex	inMenuItem,
	HIViewRef		inMenuContentView,
	HIViewRef*		outView )
{
	OSStatus	err = noErr;
	EventRef	event = CreateInitializationEvent(); // create initialization event
	
	require_action( event != NULL, CantCreateEvent, err = eventInternalErr );
	
	// add extra parameters
	if ( inMenu != NULL )
		SetEventParameter( event, kEventParamMenuRef, typeMenuRef, sizeof( inMenu ), &inMenu );
	if ( inMenuItem != 0 )
		SetEventParameter( event, kEventParamMenuItemIndex, typeMenuItemIndex, sizeof( inMenuItem ), &inMenuItem );
	if ( inMenuContentView != 0 )
		SetEventParameter( event, kEventParamControlRef, typeControlRef, sizeof( inMenuContentView ), &inMenuContentView );
	
	// register the subclass
	static bool sRegistered = false;
	if( !sRegistered )
	{
		RegisterSubclass( kTViewHILabelViewClassID, Construct );
		sRegistered = true;
	}
	
	// instantiate the object
	err = HIObjectCreate( kTViewHILabelViewClassID, event, (HIObjectRef*) outView );
	
	ReleaseEvent( event );

CantCreateEvent:
	
	return err;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetKind
//	Returns this view's ControlKind.
//——————————————————————————————————————————————————————————————————————————————————————
//
ControlKind
HILabelView::GetKind()
{
	static const ControlKind kHILabelViewKind = { 'LblI', 'LblI' };
	return kHILabelViewKind;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• Construct
//	Allocates a new HILabelView object.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::Construct(
	HIObjectRef			inObjectRef,
	TObject**			outObject )
{
	*outObject = new HILabelView( (HIViewRef) inObjectRef );
	return noErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• Initialize
//	Initializes a new HILabelView object.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus		err;
	MenuItemIndex	item;
	
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitBaseClass );
	
	inEvent.GetParameter( kEventParamMenuRef, &fMenu );
	item = inEvent.GetParameter<MenuItemIndex>( kEventParamMenuItemIndex, typeMenuItemIndex );
	
	// install a handler on ourselves for a few events that are not currently wrapped by TView
	InstallControlEventHandler( GetViewRef(), EventHandler, GetEventTypeCount( kControlEvents ),
								kControlEvents, this, NULL );
	
	if ( fMenu != NULL )
	{
		//
		// We can't allow this menu to be cached, because the appearance of the label menu item
		// changes dynamically. If the menu image is cached, then the menu will always initially
		// display the label item as it was before the user started interacting with it (selecting/
		// unselecting label colors), and any changes to the label item won't be initially visible.
		//
		ChangeMenuAttributes( fMenu, kMenuAttrDoNotCacheImage, 0 );
		
		//
		// Insert a new item that will have the specified item index. Use CustomDraw
		// for this item so we can override MeasureItemWidth/Height to give it the
		// right size for our view.
		//
		InsertMenuItemTextWithCFString( fMenu, NULL, item - 1, kMenuItemAttrCustomDraw, 0 );
		
		//
		// Set a menu property on the item that we've inserted so that we can identify this item
		// later, even if other items are inserted or removed before it (which will change its index).
		//
		HILabelView* view = this;
		SetMenuItemProperty( fMenu, item, kMenuPropertyLabelViewCreator, kMenuPropertyLabelViewTag, sizeof( this ), &view );
		
		//
		// Install our handlers to implement custom measurement for this item.
		// The view itself will provide the custom drawing.
		//
		InstallMenuEventHandler( fMenu, EventHandler, GetEventTypeCount( kMenuEvents ), kMenuEvents,
								 this, &fMenuHandler );
								 
		//
		// If we are given a menu content view, attach ourselves to it.
		//
		inEvent.GetParameter( kEventParamControlRef, &fMenuContentView );
		if ( fMenuContentView != NULL )
		{
			HIViewAddSubview( fMenuContentView, GetViewRef() );
			InstallEventHandler( GetControlEventTarget( fMenuContentView ), EventHandler,
								 GetEventTypeCount( kMenuContentEvents ), kMenuContentEvents,
								 this, &fMenuContentHandler );
		}
	}
	
CantInitBaseClass:

	return err;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• ControlHit
//	Handles clicks on the parts of a label item.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::ControlHit( HIViewPartCode inPart, UInt32 inModifiers )
{
	UInt32 oldSelected;
	
	if ( inPart == 1 )
		fSelected = 0;
	else
		fSelected ^= 1 << ( inPart - 1 );
		
	if ( oldSelected != fSelected )
		HIViewSetNeedsDisplay( GetViewRef(), true );
		
	return noErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• Draw
//	Draws the label item.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
	
	HIRect bounds;
	GetTitleBounds( &bounds );
	
	HIThemeTextInfo textInfo = { kHIThemeTextInfoVersionZero, kThemeStateActive, kThemeMenuItemFont,
								 kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop, 0,
								 kHIThemeTextTruncationNone, 1, 0 };
	HIThemeDrawTextBox( GetTitleString(), &bounds, &textInfo, inContext, kHIThemeOrientationNormal );
	
	HIViewPartCode rollPart = fTextRollover;
	if ( rollPart == 0 )
		rollPart = fRollover;
	
	if ( rollPart > 1 )
	{
		CFStringRef str = GetLabelString( rollPart );
		if ( str != NULL )
		{
			GetLabelBounds( &bounds );
			
			textInfo.fontID = kThemeSmallEmphasizedSystemFont;
			textInfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
			textInfo.truncationPosition = kHIThemeTextTruncationEnd;
			
			// draw this text in gray
			CGContextSetRGBFillColor( inContext, 0.5, 0.5, 0.5, 1.0 );
			
			HIThemeDrawTextBox( str, &bounds, &textInfo, inContext, kHIThemeOrientationNormal );
		}
	}
	
	rollPart = fPartRollover;
	if ( rollPart == 0 /* && GetCurrentEventButtonState() != 0 */ )
		rollPart = fRollover;
	
	for ( int i = 1; i <= kNumParts; i++ )
	{
		Boolean rollover = i == rollPart;
		Boolean pressed = i == GetControlHilite( GetViewRef() );
		Boolean selected = ( fSelected & ( 1 << ( i - 1 ) ) ) != 0;
		
		GetPartBounds( i, &bounds );
		
		CGPoint center = { bounds.origin.x + ( bounds.size.width / 2 ),
							bounds.origin.y + ( bounds.size.height / 2 ) };

		if ( rollover )
			// ideally we'd have a custom image here for the rollover effect, rather than re-using the pressed image
			DrawCenteredImage( inContext, center, GetPressedBackgroundImage() );
		else
		if ( pressed )
			DrawCenteredImage( inContext, center, GetPressedBackgroundImage() );
		else if ( selected )
			DrawCenteredImage( inContext, center, GetSelectedBackgroundImage() );
		
		DrawCenteredImage( inContext, center, GetPartImage( i ) );
	}
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetTitleString
//	Returns the string that is drawn above the label buttons ("Color Label:").
//——————————————————————————————————————————————————————————————————————————————————————
CFStringRef
HILabelView::GetTitleString()
{
	if ( sTitleString == NULL )
		sTitleString = CFBundleCopyLocalizedString( CFBundleGetMainBundle(), CFSTR("HILabelTitle"), NULL, NULL );
	return sTitleString;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetLabelString
//	Returns a string containing the name of a label part;
//——————————————————————————————————————————————————————————————————————————————————————
CFStringRef
HILabelView::GetLabelString( int inPart )
{
	check( inPart >= 2 );
	check( inPart <= 8 );
	
	// convert from 1-based label part index to zero-based label array index
	int index = inPart - 2;
	
	// ••• we need to invalidate this cache when the label changes
	if ( sLabelStrings[ index ] == NULL )
	{
		RGBColor rgb;
		Str255 st;
		GetLabel( LabelIndexFromPart( inPart ), &rgb, st );
		
		CFStringRef labelStr = CFStringCreateWithPascalString( NULL, st, GetApplicationTextEncoding() );
		if ( labelStr != NULL )
		{
			CFStringRef formatStr = CFBundleCopyLocalizedString( CFBundleGetMainBundle(),
																 CFSTR("HILabelFormat"),
																 NULL, NULL );
			if ( formatStr != NULL )
			{
				sLabelStrings[ index ] = CFStringCreateWithFormat( NULL, NULL, formatStr, labelStr );
				CFRelease( formatStr );
			}
			
			CFRelease( labelStr );
		}
	}
	
	return sLabelStrings[ index ];
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• DrawCenteredImage
//	Draws a CGImageRef centered around a point.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::DrawCenteredImage( CGContextRef inContext, CGPoint inCenter, CGImageRef inImage )
{
	float imageWidth = CGImageGetWidth( inImage );
	float imageHeight = CGImageGetHeight( inImage );
	
	HIRect bounds;
	bounds.size.width = imageWidth;
	bounds.size.height = imageHeight;
	bounds.origin.x = inCenter.x - ( imageWidth / 2 );
	bounds.origin.y = inCenter.y - ( imageHeight / 2 );
	
	HIViewDrawCGImage( inContext, &bounds, inImage );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetRegion
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::GetRegion( ControlPartCode inPart, RgnHandle outRgn )
{
	//
	// We provide the clickable meta-region so that if the user clicks in portions of the view
	// that are not trackable, the click can pass through the control and initiate async dragging.
	//
	if ( inPart == kControlClickableMetaPart )
	{
		RgnHandle tempRgn = NewRgn();
		if ( tempRgn == NULL )
			return eventNotHandledErr;
		
		for ( int i = 1; i <= kNumParts; i++ )
		{
			HIRect bounds;
			GetPartBounds( i, &bounds );
			
			SetRectRgn( tempRgn,
						(short) bounds.origin.x, (short) bounds.origin.y,
						(short) CGRectGetMaxX( bounds ), (short) CGRectGetMaxY( bounds ) );
			UnionRgn( tempRgn, outRgn, outRgn );
		}
		
		DisposeRgn( tempRgn );
		return noErr;
	}
	else
	{
		return eventNotHandledErr;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• HitTest
//	Hit-tests a point against the label item view.
//——————————————————————————————————————————————————————————————————————————————————————
HIViewPartCode
HILabelView::HitTest(
	const HIPoint&		inWhere )
{
	HIViewPartCode		part = kControlNoPart;
	
	// is the mouse on the item?
	if( CGRectContainsPoint( Bounds(), inWhere ) )
	{
		for ( int i = 1; i <= kNumParts; i++ )
		{
			HIRect bounds;
			GetPartBounds( i, &bounds );
			if ( CGRectContainsPoint( bounds, inWhere ) )
			{
				part = i;
				break;
			}
		}
	}
	
	return part;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• Track
//	Handles control tracking. We only use it to clear our rollover state.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::Track( TCarbonEvent& inEvent, ControlPartCode* outPartHit )
{
	//
	// Clear rollover state so that it doesn't interfere with proper part drawing during tracking.
	// Otherwise, when you move the mouse out of the part that you clicked on, the part remains
	// drawn in its rollover state because fRollover still contains that part.
	//
	ClearRollover();
	
	// let the Control Manager's standard control tracking occur
	return eventNotHandledErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetSizeConstraints
//	Returns our minimum and maximum sizes. Required for views that are used as custom
//	toolbar item views.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::GetSizeConstraints( HISize* outMin, HISize* outMax )
{
	GetOptimalSize( outMin, NULL );
	*outMax = *outMin;
	return noErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetOptimalSizeSelf
//	Returns our optimal size. Required for views that are used as custom toolbar item
//	views.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::GetOptimalSizeSelf( HISize* outSize, float* outBaseLine )
{
	// get the right edge of the last part
	HIRect partBounds;
	GetPartBounds( kNumParts, &partBounds );
	outSize->width = CGRectGetMaxX( partBounds );
	
	// add the origin of the first part, so that the same space is on the right side of the last part
	GetPartBounds( 1, &partBounds );
	outSize->width += partBounds.origin.x;
	
	HIThemeTextInfo textInfo = { kHIThemeTextInfoVersionZero, kThemeStateActive, kThemeMenuItemFont,
								 kHIThemeTextHorizontalFlushLeft, kHIThemeTextVerticalFlushTop,
								 0, kHIThemeTextTruncationNone, 0 };
	HIThemeGetTextDimensions( CFSTR("Sample"), 0, &textInfo, NULL, &fPartYOffset, outBaseLine );
	
	//
	// Calculate the bounds of the first part. This will take the new value of fPartYOffset
	// into account. The bottom of the part bounds is our view height.
	//
	GetPartBounds( 1, &partBounds );
	outSize->height = CGRectGetMaxY( partBounds ) + kLabelNameHeight;
	
	return noErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• VisibilityChanged
//	Our visibility has changed. Effectively, this is called when the menu or toolbar
//	containing us has changed visibility.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::VisibilityChanged()
{
	//
	// Whenever the label view becomes visible, we install a handler on our owning window
	// to track mouse-moved and mouse-dragged events; this lets us show rollover feedback as
	// the mouse passes over the label parts.
	//
	if ( IsVisible() )
	{
		if ( fWindowHandler == NULL )
		{
			InstallEventHandler( GetWindowEventTarget( GetControlOwner( GetViewRef() ) ), EventHandler,
								 GetEventTypeCount( kWindowEvents ),
								 kWindowEvents, this, &fWindowHandler );
		}
		
		// start out with no rollover highlighting
		ClearRollover();
	}
	else
	{			
		if ( fWindowHandler != NULL )
		{
			RemoveEventHandler( fWindowHandler );
			fWindowHandler = NULL;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• EventHandler
//	Handles various events for the view.
//——————————————————————————————————————————————————————————————————————————————————————
pascal OSStatus
HILabelView::EventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	TCarbonEvent	event( inEvent );
	OSStatus		result = eventNotHandledErr;
	HILabelView*	pView = (HILabelView*) inRefcon;

	if ( event.GetClass() == kEventClassMouse )
	{
		check( event.GetKind() == kEventMouseMoved || event.GetKind() == kEventMouseDragged );
		
		if ( pView->GetOwner() == NULL || !IsWindowVisible( pView->GetOwner() ) )
			return eventNotHandledErr;
		
		HIPoint pt;
		event.GetParameter<HIPoint>( kEventParamMouseLocation, typeHIPoint, &pt );
		_HIViewConvertFromGlobalPoint( &pt, pView->GetViewRef() );
		
		pView->fNewRollover = pView->HitTest( pt );
		if ( pView->fRollover != pView->fNewRollover )
		{
			HIRect	firstLabelBounds;
			HIRect	lastLabelBounds;
			
			pView->GetPartBounds( 2, &firstLabelBounds );
			pView->GetPartBounds( kNumParts, &lastLabelBounds );
			
			//
			// If we are now over a label part, immediately invalidate the label name for redraw.
			// Also do this if the mouse passed outside of the area used by the label parts, so
			// that we immediately erase the name in this case. By passing fRolloverTimer (which
			// may NULL or may be a valid timer) to RolloverChangedTimer, we re-use its behavior
			// to remove any installed timer.
			//
			// If we are not over a label part, delay the invalidation just a little to see if the
			// user moves the mouse over another label part. If so, then we'll get back here again
			// and we'll invalidate the old and new parts, but never completely erase the label name;
			// this avoids flicker. If the user doesn't quickly move the mouse over another label,
			// then the timer will fire and we'll invalidate and erase the label name.
			//
			if ( pView->fNewRollover != 0 || pt.x < firstLabelBounds.origin.x || pt.x > CGRectGetMaxX( lastLabelBounds ) )
				RolloverChangedTimer( pView->fRolloverTimer, pView );
			else if ( pView->fRolloverTimer == NULL )
				InstallEventLoopTimer( GetMainEventLoop(), kRolloverTimerDelay, 0, RolloverChangedTimer, pView, &pView->fRolloverTimer );
			else
				SetEventLoopTimerNextFireTime( pView->fRolloverTimer, kRolloverTimerDelay );
		}
			
		check( result == eventNotHandledErr );
	}
	else if ( event.GetClass() == kEventClassMenu )
	{
		switch ( event.GetKind() )
		{
			case kEventMenuMeasureItemWidth:
			{
				MenuItemIndex item = event.GetParameter<MenuItemIndex>( kEventParamMenuItemIndex, typeMenuItemIndex );
				if ( pView->GetMenuItemViewProperty( item ) == pView )
				{
					HISize size;
					pView->GetOptimalSize( &size, NULL );
					
					SInt16 width = (SInt16) size.width;
					event.SetParameter<SInt16>( kEventParamMenuItemWidth, typeShortInteger, width );
					result = noErr;
				}
				break;
			}
				
			case kEventMenuMeasureItemHeight:
			{
				MenuItemIndex item = event.GetParameter<MenuItemIndex>( kEventParamMenuItemIndex, typeMenuItemIndex );
				if ( pView->GetMenuItemViewProperty( item ) == pView )
				{
					HISize size;
					pView->GetOptimalSize( &size, NULL );
					
					SInt16 height = (SInt16) size.height;
					event.SetParameter<SInt16>( kEventParamMenuItemHeight, typeShortInteger, height );
					result = noErr;
				}
				break;
			}
				
			case kEventMenuDrawItem:
				// draw nothing
				result = noErr;
				break;
			
			//
			// When the menu is asked to calculate its size, let it do so, and then reposition
			// the label view to match the bounds of the menu item that we're implementing.
			//
			case kEventMenuCalculateSize:
				result = CallNextEventHandler( inCaller, inEvent );
				if ( result == noErr )
				{
					MenuItemIndex item = pView->FindItem();
					if ( item != 0 )
					{
						RgnHandle itemRgn = NewRgn();
						if ( itemRgn != NULL )
						{
							GetControlRegion( pView->fMenuContentView, item, itemRgn );
							
							Rect bounds;
							GetRegionBounds( itemRgn, &bounds );
							DisposeRgn( itemRgn );
							
							HIRect frame = { { bounds.left, bounds.top },
											{ bounds.right - bounds.left, bounds.bottom - bounds.top } };
							HIViewSetFrame( pView->GetViewRef(), &frame );
						}
					}
				}
				break;
		}
	}
	else if ( event.GetClass() == kEventClassControl )
	{
		switch ( event.GetKind() )
		{
			// a future version of TView will provide this handler automatically
			case kEventControlVisibilityChanged:
				pView->VisibilityChanged();
				break;
				
			case kEventControlSimulateHit:
			{
				ControlPartCode part = event.GetParameter<ControlPartCode>( kEventParamControlPart, typeControlPartCode );
				if ( pView->GetMenuItemViewProperty( part ) == pView )
				{
					// remember which item was under the mouse
					HIViewPartCode selectedPart = pView->fRollover;
					
					// detect the selected item even if the user releases the mouse before RolloverChangedTimer fires
					if ( selectedPart != 0 )
						selectedPart = pView->fNewRollover;
					
					// if we are over an item, flash it to show feedback, and invert its state
					if ( selectedPart != 0 )
					{
						enum { kFlashTicks = 5 };
						
						// turn off normal rollover effects
						pView->ClearRollover();
						
						// make sure that rollover text continues to be displayed (don't blink it on and off)
						pView->fTextRollover = selectedPart;
						
						// turn off part rollover
						pView->fPartRollover = 0;
						pView->RedrawWithDelay( kFlashTicks );
						
						// turn on part rollover
						pView->fPartRollover = selectedPart;
						pView->RedrawWithDelay( kFlashTicks );
						
						// turn off part rollover
						pView->fPartRollover = 0;
						pView->RedrawWithDelay( kFlashTicks );
		
						// invert selected state for this part
						pView->ControlHit( selectedPart, 0 );
						pView->RedrawWithDelay( kFlashTicks );
					}
					
					// we've handled this SimulateHit event; the standard menu view does not need to be called
					result = noErr;
				}
				break;
			}
			
			default:
				break;
		}
	}
	else if ( event.GetClass() == kEventClassCommand )
	{
		HICommand cmd;
		event.GetParameter( kEventParamDirectObject, &cmd );
		
		//
		// If the user is using non-sticky menu tracking and releases the mouse over our view, the Menu Manager
		// will send a kEventCommandProcess event for our menu item. We don't want this event to be propagated,
		// since it is meaningless for our view, so we just catch it here and return noErr to stop it.
		//
		if (	( cmd.attributes & kHICommandFromMenu ) != 0
			 && cmd.menu.menuRef == pView->fMenu
			 && pView->GetMenuItemViewProperty( cmd.menu.menuItemIndex ) == pView )
		{
			result = noErr;
		}
	}
	
	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• FindItem
//	Locates the menu item in the menu that we're currently using to display the view.
//	May return 0 if no item could be found.
//——————————————————————————————————————————————————————————————————————————————————————
MenuItemIndex
HILabelView::FindItem()
{
	//
	// We cache the menu item that we find, and reverify it each time that we need it.
	// If the menu item index has changed, then our property won't be there, and we
	// invalidate the cache.
	//
	if ( fMenuItem != 0 && GetMenuItemViewProperty( fMenuItem ) != this )
		fMenuItem = 0;
		
	// if we don't have a cached menu item, search the menu for an item with our property
	if ( fMenuItem == 0 )
	{
		for ( int i = 1, cItems = CountMenuItems( fMenu ); i <= cItems; i++ )
		{
			if ( GetMenuItemViewProperty( i ) == this )
			{
				fMenuItem = i;
				break;
			}
		}
	}
	
	return fMenuItem;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetMenuItemViewProperty
//	Returns the LabelView property attached to a menu item, if any.
//——————————————————————————————————————————————————————————————————————————————————————
HILabelView*
HILabelView::GetMenuItemViewProperty( MenuItemIndex inItem )
{
	HILabelView* itemView = NULL;
	GetMenuItemProperty( fMenu, inItem, kMenuPropertyLabelViewCreator, kMenuPropertyLabelViewTag,
						 sizeof( itemView ), NULL, &itemView );
	return itemView;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• RedrawWithDelay
//	Invalidates and redraws the view, and then delays a specified time.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::RedrawWithDelay( int inDelayTicks )
{
	HIViewRef		view = GetViewRef();
	HIWindowRef		window = GetControlOwner( view );
	UInt32			endTicks;
	
	HIViewSetNeedsDisplay( view, true );
	HIViewRender( view );
	HIWindowFlush( window );
	Delay( inDelayTicks, &endTicks );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• _HIViewConvertFromGlobalPoint
//	Converts a point from global coordinates to view coordinates.
//——————————————————————————————————————————————————————————————————————————————————————
static void
_HIViewConvertFromGlobalPoint( HIPoint* ioPoint, HIViewRef inDestView )
{
	Point			pt = { (short) ioPoint->y, (short) ioPoint->x };
	WindowRef		window = GetControlOwner( inDestView );
	GrafPtr			port = GetWindowPort( window );
	PixMapHandle	pixmap = GetPortPixMap( port );
	
	// convert to local coordinates
	QDGlobalToLocalPoint( port, &pt );
	
	// convert to window coordinates
	pt.h -= (*pixmap)->bounds.left;
	pt.v -= (*pixmap)->bounds.top;
	
	// convert to view coordinates
	ioPoint->x = pt.h;
	ioPoint->y = pt.v;
	HIViewConvertPoint( ioPoint, HIViewGetRoot( window ), inDestView );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• RolloverChangedTimer
//	A timer that's used to invalidate rolled-over parts after a short delay. We use a
//	timer to delay the invalidate to reduce flicker; if we invalidate immediately, what
//	tends to happen as you move horizontally is that the mouse passes from part A, to
//	the space between parts, to part B, and when the mouse is in the space between parts,
//	we erase the label name entirely, only to redraw it shortly when the mouse passes over
//	part B. This flickers. By delaying the invalidation slightly, we give the user time
//	to move into another part, and avoid erasing the label name at all.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::RolloverChangedTimer( EventLoopTimerRef inTimer, void* inRefcon )
{
	HILabelView*	pView = (HILabelView*) inRefcon;
	HIRect			bounds;
	Rect			r;
	RgnHandle		rgn = NewRgn();
	HIViewPartCode	oldRollover = pView->fRollover;
	HIViewPartCode	newRollover = pView->fNewRollover;
	
	// if either the old or new rollover part was not the "none" part, we need to update the label
	if ( oldRollover > 1 || newRollover > 1 )
	{
		pView->GetLabelBounds( &bounds );
		SetRect( &r, (short) bounds.origin.x, (short) bounds.origin.y,
				(short) CGRectGetMaxX( bounds ), (short) CGRectGetMaxY( bounds ) );
		RectRgn( rgn, &r );
		HIViewSetNeedsDisplayInRegion( pView->GetViewRef(), rgn, true );
	}

	if ( oldRollover != 0 )
	{
		pView->GetPartBounds( oldRollover, &bounds );
		SetRect( &r, (short) bounds.origin.x, (short) bounds.origin.y,
				(short) CGRectGetMaxX( bounds ), (short) CGRectGetMaxY( bounds ) );
		RectRgn( rgn, &r );
		HIViewSetNeedsDisplayInRegion( pView->GetViewRef(), rgn, true );
	}

	if ( newRollover != 0 )
	{
		pView->GetPartBounds( newRollover, &bounds );
		SetRect( &r, (short) bounds.origin.x, (short) bounds.origin.y,
				(short) CGRectGetMaxX( bounds ), (short) CGRectGetMaxY( bounds ) );
		RectRgn( rgn, &r );
		HIViewSetNeedsDisplayInRegion( pView->GetViewRef(), rgn, true );
	}

	pView->fRollover = newRollover;
	
	if ( inTimer != NULL )
	{
		RemoveEventLoopTimer( inTimer );
		pView->fRolloverTimer = NULL;
	}
	
	DisposeRgn( rgn );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetTitleBounds
//	Returns the bounds of the title text ("Color Label:").
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::GetTitleBounds( HIRect* outBounds )
{
	int leftEdge = GetLeftEdge();
	
	*outBounds = Bounds();
	outBounds->origin.x = leftEdge;
	outBounds->size.width -= leftEdge;
	outBounds->size.height = fPartYOffset;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetLabelBounds
//	Returns the bounds of the label name text ("Project 1").
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::GetLabelBounds( HIRect* outBounds )
{
	// start with view bounds so that we center the label name text on the view horizontally
	*outBounds = Bounds();
	
	// position the label name text vertically under the label buttons
	HIRect partBounds;
	GetPartBounds( 1, &partBounds );
	outBounds->origin.y = CGRectGetMaxY( partBounds );
	outBounds->size.height = CGRectGetMaxY( Bounds() ) - outBounds->origin.y;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetPartBounds
//	Returns the bounds of one of the label button parts.
//——————————————————————————————————————————————————————————————————————————————————————
void
HILabelView::GetPartBounds( int inPart, HIRect* outBounds )
{
	CGImageRef image = GetPartImage( 1 );
	float imageWidth = CGImageGetWidth( image );
	
	// calculate an origin for the "none" part that aligns the left side of the image with the item text
	HIRect bounds = Bounds();
	bounds.origin.x += GetLeftEdge();
	bounds.origin.x -= ( kLabelPartWidth - imageWidth ) / 2;
	
	// position each part at an offset of kLabelPartWidth + 1 from the last part
	outBounds->origin.x = bounds.origin.x + ( kLabelPartWidth + 1 ) * ( inPart - 1 );
	
	// if we're getting the bounds of a label part, add the space between the none and first label parts
	if ( inPart > 1 )
		outBounds->origin.x += kFirstLabelSpace;
	
	outBounds->origin.y = bounds.origin.y + fPartYOffset;
	outBounds->size.width = kLabelPartWidth;
	outBounds->size.height = kLabelPartHeight;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• HILabelView::GetLeftEdge
//	Returns the left edge (in view coordinates) of the "Color Label:" text and the "none" part.
//——————————————————————————————————————————————————————————————————————————————————————
int
HILabelView::GetLeftEdge()
{
	if ( fLeftEdge == 0 )
	{
		SInt32 metric;
		MenuAttributes attr = 0;
		
		if ( fMenu != NULL )
			GetMenuAttributes( fMenu, &attr );
		if ( ( attr & kMenuAttrExcludesMarkColumn ) != 0 )
			GetThemeMetric( kThemeMetricMenuExcludedMarkColumnWidth, &metric );
		else
			GetThemeMetric( kThemeMetricMenuMarkColumnWidth, &metric );
		
		fLeftEdge = metric;
	}
	
	return fLeftEdge;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetPartImage
//	Returns a CGImage for a label part.
//——————————————————————————————————————————————————————————————————————————————————————
CGImageRef
HILabelView::GetPartImage( int inPart )
{
	check( inPart > 0 );
	check( inPart <= kNumParts );
	
	// convert from 1-based part code to 0-based array index
	inPart--;
	
	if ( sImages[ inPart ] == NULL )
		LoadPartImages();
		
	return sImages[ inPart ];
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetSelectedBackgroundImage
//——————————————————————————————————————————————————————————————————————————————————————
CGImageRef
HILabelView::GetSelectedBackgroundImage()
{
	return GetBackgroundImage( CFSTR("item"), &sSelectedBackgroundImage );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetPressedBackgroundImage
//	Returns a CGImageRef for the background of a pressed label button.
//——————————————————————————————————————————————————————————————————————————————————————
CGImageRef
HILabelView::GetPressedBackgroundImage()
{
	return GetBackgroundImage( CFSTR("itempressed"), &sPressedBackgroundImage );
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetSelectedBackgroundImage
//	Returns a CGImageRef for the background of a selected label button.
//——————————————————————————————————————————————————————————————————————————————————————
CGImageRef
HILabelView::GetBackgroundImage( CFStringRef inName, CGImageRef* ioImage )
{
	if ( *ioImage == NULL )
	{
		CFURLRef url = CFBundleCopyResourceURL( CFBundleGetMainBundle(), inName, CFSTR("png"), NULL );
		CGDataProviderRef data = CGDataProviderCreateWithURL( url );
		CFRelease( url );
		
		*ioImage = CGImageCreateWithPNGDataProvider( data, NULL, true, kCGRenderingIntentDefault );
		CFRelease( data );
	}
	
	return *ioImage;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• LoadPartImages
//	Loads the CGImages for the label parts.
//——————————————————————————————————————————————————————————————————————————————————————
OSStatus
HILabelView::LoadPartImages()
{
	check( sImages[0] == NULL );
	
	static const char* kImageNames[ kNumParts ] =	{
														"none",
														"red",
														"orange",
														"yellow",
														"green",
														"blue",
														"grape",
														"grey"
													};
	
	CFBundleRef bundle = CFBundleGetMainBundle();
	for ( int i = 0; i < kNumParts; i++ )
	{
		CFStringRef str = CFStringCreateWithCString( NULL, kImageNames[ i ], kTextEncodingMacRoman );
		CFURLRef url = CFBundleCopyResourceURL( bundle, str, CFSTR("png"), NULL );
		CGDataProviderRef data = CGDataProviderCreateWithURL( url );
		CFRelease( url );
		
		// create our image
		sImages[ i ] = CGImageCreateWithPNGDataProvider( data, NULL, true, kCGRenderingIntentDefault );
		CFRelease( data );
	}
	
	return noErr;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• LabelIndexFromPart
//	The ordering of the classic Mac OS labels is not the same as the order in which we
//	draw them in this view. The classic Mac OS label colors were (in order from 1 to 7):
//	brown, green, dark blue, light blue, pink, red, and orange. This function maps from
//	the label part code space to the label index space.
//——————————————————————————————————————————————————————————————————————————————————————
int
HILabelView::LabelIndexFromPart( int inPart )
{
	static const int kMap[] = { 6, 7, 5, 2, 4, 3, 1 };
	
	// part code 1 is the "none" part; first label part is 2; last is 8
	check( inPart >= 2 );
	check( inPart <= 8 );
	
	return kMap[ inPart - 2 ];
}
