/*
	File:		MenuKeyGlyphs.h

	Contains:	Menu keyboard equivalent glyph definitions.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

	Change History (most recent first):

$Log: MenuKeyGlyphs.h,v $
Revision 1.5  2002/11/08 23:34:55         
Include "MoreSetup.h".

Revision 1.4  2001/11/07 15:50:28         
Tidy up headers, add CVS logs, update copyright.


         <3>     21/9/01    Quinn   Get rid of wacky Finder label.
         <2>    11/11/98    PCG     fix header
         <1>    11/11/98    PCG     re-org at behest of Quinn

	Old Change History (most recent first):

         <1>     6/16/98    PCG     initial checkin
*/

#pragma once

#include "MoreSetup.h"

/*************************************************************************
**
**	Menu Key Glyph Constants
**
**	Apple Macintosh Developer Technical Support
**
**	revision history
**	----------------------------
**	MXM		??/??/97	created
**	PCG		02/09/98	assimilated
**
**	writers
**	----------------------------
**	MXM		Matthew Xavier Mora
**	PCG		Pete Gontier
**
**************************************************************************/

enum
{
	MoreMenus_kKeyGlyphNull						= 0x00, // Null(always glyph 1)
	MoreMenus_kKeyGlyphUnassigned				= 0x01,	// (reserved for 2 bytes)
	MoreMenus_kKeyGlyphLeftToRightTabKey		= 0x02,
	MoreMenus_kKeyGlyphRightToLeftTabKey		= 0x03,
	MoreMenus_kKeyGlyphEnterKey					= 0x04,
	MoreMenus_kKeyGlyphShiftKey					= 0x05,
	MoreMenus_kKeyGlyphControlKey				= 0x06,
	MoreMenus_kKeyGlyphOptionKey				= 0x07,
	MoreMenus_kKeyGlyphNull1					= 0x08,
	MoreMenus_kKeyGlyphSpaceKey					= 0x09,
	MoreMenus_kKeyGlyphDeleteKey				= 0x0A,
	MoreMenus_kKeyGlyphLeftToRightReturnKey		= 0x0B,
	MoreMenus_kKeyGlyphRightToLeftReturnKey		= 0x0C,
	MoreMenus_kKeyGlyphNonmarkingReturnKey		= 0x0D,
	MoreMenus_kKeyGlyphUnassigned1				= 0x0E,
	MoreMenus_kKeyGlyphPencilKey				= 0x0F,
	MoreMenus_kKeyGlyphDownwardDashedArrowKey	= 0x10,
	MoreMenus_kKeyGlyphCommandKey				= 0x11,
	MoreMenus_kKeyGlyphCheckmarkKey				= 0x12,
	MoreMenus_kKeyGlyphDiamondKey				= 0x13,
	MoreMenus_kKeyGlyphAppleLogoKey				= 0x14,
	MoreMenus_kKeyGlyphUnassigned2				= 0x15,	// (paragraph in Korean)
	MoreMenus_kKeyGlyphUnassigned3				= 0x16,
	MoreMenus_kKeyGlyphForwardDelete			= 0x17,
	MoreMenus_kKeyGlyphLeftwardDashedArrowKey	= 0x18,
	MoreMenus_kKeyGlyphUpwardDashedArrowKey		= 0x19,
	MoreMenus_kKeyGlyphRightwardDashedArrowKey	= 0x1A,
	MoreMenus_kKeyGlyphEscapeKey				= 0x1B,
	MoreMenus_kKeyGlyphClearKey					= 0x1C,
	MoreMenus_kKeyGlyphUnassigned4				= 0x1D, // (left double quotes in Japanese)
	MoreMenus_kKeyGlyphUnassigned5				= 0x1E, // (right double quotes in Japanese)
	MoreMenus_kKeyGlyphUnassigned6				= 0x1F, // (trademark in Japanese)
	MoreMenus_kKeyGlyphBlankKey					= 0x61,
	MoreMenus_kKeyGlyphPageUpKey				= 0x62,
	MoreMenus_kKeyGlyphCapsLockKey				= 0x63,
	MoreMenus_kKeyGlyphLeftArrowKey				= 0x64,
	MoreMenus_kKeyGlyphRightArrowKey			= 0x65,
	MoreMenus_kKeyGlyphNorthwestArrowKey		= 0x66,
	MoreMenus_kKeyGlyphHelpKey					= 0x67,
	MoreMenus_kKeyGlyphUpArrowKey				= 0x68,
	MoreMenus_kKeyGlyphSouthEastArrowKey		= 0x69,
	MoreMenus_kKeyGlyphDownArrowKey				= 0x6A,
	MoreMenus_kKeyGlyphPageDownKey				= 0x6B,
	MoreMenus_kKeyGlyphAppleLogoKeyOutline		= 0x6C,
	MoreMenus_kKeyGlyphContextualMenuKey		= 0x6D,
	MoreMenus_kKeyGlyphPowerKey					= 0x6E,
	MoreMenus_kKeyGlyphF1Key					= 0x6F,
	MoreMenus_kKeyGlyphF2Key					= 0x70,
	MoreMenus_kKeyGlyphF3Key					= 0x71,
	MoreMenus_kKeyGlyphF4Key					= 0x72,
	MoreMenus_kKeyGlyphF5Key					= 0x73,
	MoreMenus_kKeyGlyphF6Key					= 0x74,
	MoreMenus_kKeyGlyphF7Key					= 0x75,
	MoreMenus_kKeyGlyphF8Key					= 0x76,
	MoreMenus_kKeyGlyphF9Key					= 0x77,
	MoreMenus_kKeyGlyphF10Key					= 0x78,
	MoreMenus_kKeyGlyphF11Key					= 0x79,
	MoreMenus_kKeyGlyphF12Key					= 0x7A,
	MoreMenus_kKeyGlyphF13Key					= 0x87,
	MoreMenus_kKeyGlyphF14Key					= 0x88,
	MoreMenus_kKeyGlyphF15Key					= 0x89,
	MoreMenus_kKeyGlyphControlKey1				= 0x8A  // (ISO standard)
};

