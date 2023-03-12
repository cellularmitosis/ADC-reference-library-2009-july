/*
	File:		BevelButtonItems.h

	Contains:	Constants for our CDEF Tester

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

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

// Bevel Buttons

enum
{
	kBevelSizePopup			= 1,
	kBevelBehaviorPopup		= 3,
	kBevelOffsetCheck		= 4,
	
	kBevelMenuIDText		= 7,
	kBevelMultiMenuCheck	= 8,
	kBevelMenuOnRightCheck	= 9,
	
	kBevelContentPopup		= 11,
	kBevelContentIDText		= 13,
	
	kBevelGraphicAlignPopup	= 15,
	kBevelGraphicHOffsetText= 18,
	kBevelGraphicVOffsetText= 20,
	
	kBevelTextAlignPopup	= 22,
	kBevelTextOffsetText	= 24,
	kBevelTextPlacePopup	= 25,
	
	kBevelHeightText		= 27,
	kBevelWidthText			= 29,
	kBevelTitleText			= 30,

	kBevelCancelButton		= 32,
	kBevelOKButton			= 33
};

// Dividers

enum 
{
	kHorizontalRadio		= 6,
	kVerticalRadio			= 7,
	kLengthText				= 2,
	kDividerCancelButton	= 3,
	kDividerOKButton		= 4,
	kDividerRadioGroup		= 5
};

// Disclosure Triangles

enum
{
	kLeftFacingCheck		= 1,
	kAutoTrackCheck			= 2,
	kTriangleCancelButton	= 3,
	kTriangleOKButton		= 4
};

// Edit Text
enum
{
	kETOKButton				= 1,
	kETCancelButton			= 2,
	kETPasswordCheck		= 3
};

// Static Text
enum
{
	kSTOKButton				= 1,
	kSTCancelButton			= 2
};


// Slider
enum
{
	kSliderNonDirectional		= 3,
	kSliderDirectional		= 4,
	kSliderTickMarks		= 5,
	kSliderReverse			= 6,
        kSliderVertical			= 7,
        kSliderSmall			= 8
};
	
// Clock
enum
{
	kClockTime				= 3,
	kClockTimeSeconds		= 4,
	kClockDate				= 5,
	kClockMonthYear			= 6
};


// Finder Header

enum
{
	kFHHeightText			= 3,
	kFHWidthText			= 4,
	kFHListViewCheck		= 5,
	kFHCancelButton			= 6,
	kFHOKButton				= 7
};

// Icon/Picture

enum
{
	kIconResIDText			= 2,
	kIconOK					= 3,
	kIconCancel				= 4,
	kIconNoHitCheck			= 5,
	kIconHeightLabelText	= 6,
	kIconWidthLabelText		= 7,
	kIconHeightText			= 8,
	kIconWidthText			= 9,
	kIconUseRectCheck		= 10
};

// Progress Bar

enum
{
	kProgLengthText			= 2,
	kProgOKButton			= 3,
	kProgCancelButton		= 4,
	kProgIndeterminateCheck	= 5
};

// Group Box

enum
{
	kGroupOKButton			= 1,
	kGroupCancelButton		= 2,
	kGroupRadioGroup		= 3,
	kGroupPrimaryRadio		= 4,
	kGroupSecondaryRadio	= 5,
	kGroupHeightText		= 8,
	kGroupWidthText			= 9,
	kGroupVariantPopup		= 10,
	kGroupTitleText			= 12,
	kGroupMenuIDText		= 13,
	kGroupMenuIDLabelText	= 14
};

// Placard

enum
{
	kPlacardHeightText		= 3,
	kPlacardWidthText		= 4,
	kPlacardOKButton		= 5,
	kPlacardCancelButton	= 6
};

// Popup Arrow

enum
{
	kPopArrowDirPopup		= 1,
	kPopArrowSmallCheck		= 2,
	kPopArrowOKButton		= 3,
	kPopArrowCancelButton	= 4
};

// Scroll Bars

enum 
{
	kScrollOrientationRadioGroup	= 1,
	kScrollHorizontalRadio		= 2,
	kScrollVerticalRadio		= 3,
        kScrollSizeRadioGroup		= 4,
        kScrollLargeRadio		= 5,
        kScrollSmallRadio		= 6,
        kScrollLiveScrollCheck		= 7,
	kScrollLengthText		= 9,
        kScrollMinText			= 11,
        kScrollMaxText			= 13,
        kScrollViewSizeText		= 15,
	kScrollCancelButton		= 16,
	kScrollOKButton			= 17
};

// Image Well

enum
{
	kImageContentPopup		= 1,
	kImageResIDText			= 3,
	kImageOKButton			= 4,
	kImageCancelButton		= 5,
	kImageHeightText		= 8,
	kImageWidthText			= 9,
	kImageDragDestination	= 10
};

// Tabs
enum
{
	kTabUseIcon				= 3,
	kTabNumberTabs			= 5,
	kTabSizeGroup			= 6,
	kTabDirectionPopup		= 9
};
	
// Push Buttons
enum
{
	kPushButtonOKButton		= 1,
	kPushButtonCancelButton	= 2,
	kPushButtonTitleText	= 4,
	kPushButtonDefaultCheck	= 5,
        kPushButtonSmallCheck	= 6
};

enum
{
	kCheckBoxOKButton		= 1,
	kCheckBoxCancelButton	= 2,
	kCheckBoxTitleText		= 4,
        kCheckBoxAutoToggleCheck	= 5,
        kCheckBoxSmallCheck		= 6
};

//Disclosure Button
enum
{
        kDisclosureButtonAutoToggleCheck	= 1,
        kDisclosureButtonCancelButton		= 2,
        kDisclosureButtonOKButton		= 3
};

