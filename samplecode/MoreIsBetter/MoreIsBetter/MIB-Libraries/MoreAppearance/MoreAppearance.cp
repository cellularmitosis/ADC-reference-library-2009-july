/*
	File:		MoreAppearance.cp

	Contains:	Appearance Manager utilities.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreAppearance.cp,v $
Revision 1.7  2002/11/08 22:44:39         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.6  2001/11/07 15:50:43         
Tidy up headers, add CVS logs, update copyright.


         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>     20/3/00    Quinn   The various "have this feature" routines now auto-initialise. 
                                    Made this change so that other MIB modules can call them without
                                    worrying about initialising this module.
         <3>     1/22/99    PCG     TARGET_CARBON
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>     9/11/98    PCG     conditionalize calls to Appearance Manager 1.1 so they occur on
                                    PowerPC only and add sanity checks for Appearance Manager
                                    availability
         <4>      9/4/98    PCG     theme drawing state functions call real APIs if present
         <3>      9/1/98    PCG     clean up Gestalt cache logic; MoreGetThemeDrawingState inits
                                    stateResult to NIL
         <2>      9/1/98    PCG     add theme drawing state functions
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreSetup.h"

#include "MoreAppearance.h"

#include "MoreQuickDraw.h"

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Appearance.h>
	#include <Gestalt.h>
	#include <MacMemory.h>
#endif

static Boolean gHaveInited;
static UInt32  gAppearanceAttributes;
static UInt32  gAppearanceVersion;

pascal long GetAppearanceVersion (void)
{
	OSStatus junk;
	
	//
	//	Simply returns our cached variable. See InitMoreAppearance
	//	for how this variable is initialized.
	//

	if (!gHaveInited) {
		junk = InitMoreAppearance();
		assert(junk == noErr);
	}
	return gAppearanceVersion;
}

pascal Boolean HaveAppearance (void)
{
	OSStatus junk;

	//
	//	Tests a bit in our cached variable and promotes it to a byte
	//	indicating whether the Appearance APIs are present.
	//

	if (!gHaveInited) {
		junk = InitMoreAppearance();
		assert(junk == noErr);
	}
	return (gAppearanceAttributes & (1 << gestaltAppearanceExists)) != 0;
}

pascal Boolean AppearanceInCompatibilityMode (void)
{
	OSStatus junk;

	//
	//	Tests a bit in our cached variable and promotes it to a byte
	//	indicating whether Appearance is running in compatibility mode.
	//

	if (!gHaveInited) {
		junk = InitMoreAppearance();
		assert(junk == noErr);
	}
	return (gAppearanceAttributes & (1 << gestaltAppearanceCompatMode)) != 0;
}

pascal OSStatus InitMoreAppearance (void)
{
	//
	//	Caches some Gestalt values so we don't have to keep calling Gestalt.
	//	If Appearance is present, registers us as an Appearance-savvy app.
	//	Second-guesses the version advertised by Gestalt.
	//

	gHaveInited = true;
	
	OSStatus err = Gestalt (gestaltAppearanceAttr, (SInt32 *) &gAppearanceAttributes);

	if (err == gestaltUndefSelectorErr)
	{
		gAppearanceAttributes = 0;
		err = noErr;
	}
	else if (!err)
	{
		err = Gestalt (gestaltAppearanceVersion, (SInt32 *) &gAppearanceVersion);

		if (err == gestaltUndefSelectorErr)
		{
			gAppearanceVersion = 0x0100;
			err = noErr;
		}
	}

	if (!err && HaveAppearance ( ))
	{
		//
		// We don't need to do any of these sanity checks for the 
		// Carbon build because these symbols are always available 
		// to Carbon builds.
		//
		
#if ! TARGET_API_MAC_CARBON

		//
		//	Perform sanity checking for potential weak links.
		//	Even though Gestalt claims a certain version of
		//	Appearance is available, we still may not have
		//	successfully resolved all weak links to it. So
		//	we second-guess Gestalt's claim and fall back to
		//	progressively less useful versions of Appearance.
		//	This is not an extensible approach; Appearance
		//	Manager 2.0 weak links might be unresolved and
		//	this code would never know to check. Still, this
		//	code is only for ADDITIONAL paranoia/safety, and
		//	a new Appearance Manager will not render it
		//	invalid, just a little less canonical.
		//

#if TARGET_RT_MAC_CFM

#if TARGET_CPU_PPC

		if (gAppearanceVersion >= 0x0110 && !GetTheme)
			gAppearanceVersion = 0x0101;

#endif // TARGET_CPU_PPC

		if (gAppearanceVersion >= 0x0101 && !DrawThemeModelessDialogFrame)
			gAppearanceVersion = 0x0100;

		if (gAppearanceVersion >= 0x0100 && !RegisterAppearanceClient)
		{
			gAppearanceVersion		= 0;
			gAppearanceAttributes	= 0;
		}

#endif // TARGET_RT_MAC_CFM

		if (HaveAppearance ( ))
			err = RegisterAppearanceClient ( );

#endif // ! TARGET_API_MAC_CARBON
	}

	return err;
}

#pragma mark -

#if ! TARGET_API_MAC_CARBON

struct MoreThemeDrawingStateTag
{
	//
	//	stolen from
	//		{CommonSystem}:Toolbox:ToolboxUtils:CommonUtilities:ColorPenState.c
	//

	Boolean			colorPort;
	Boolean			bkPatternIsValid;
	RGBColor		foreColor;
	RGBColor		backColor;
	PenState		pen;
	SInt16			textMode;
	PixPatHandle	pnPixPat;
	PixPatHandle	bkPixPat;
	Pattern			bkPat;
	UInt32			fgColor;
	UInt32			bkColor;
};

#endif

pascal OSStatus MoreGetThemeDrawingState (MoreThemeDrawingState *stateResult)
{
	//
	//	mostly stolen from
	//		{CommonSystem}:Toolbox:ToolboxUtils:CommonUtilities:ColorPenState.c
	//

	*stateResult = NULL;

	OSStatus err = noErr;

	// The remaining uses of TARGET_CPU_PPC in this file are pretty 
	// cheesy, but they're actually correct.  We're using them to 
	// test for a weak link.  This works for CFM builds.  It also 
	// compiles and runs for Mach-O builds (it uses a few extra 
	// CPU cycles).  However, it won't compile for 68K builds 
	// because the functions are either not defined for 68K builds 
	// or they're defined as an inline A-trap, which you can't take
	// the address of.  So, testing for TARGET_CPU_PPC is OK.
	
#if TARGET_CPU_PPC

	if (::GetAppearanceVersion ( ) >= 0x0110 && ::GetThemeDrawingState)
		err = ::GetThemeDrawingState ((ThemeDrawingState *) stateResult);
	else

#endif

#if TARGET_API_MAC_CARBON
		err = unimpErr;
#else

	{
		MoreThemeDrawingState state = MoreThemeDrawingState (::NewPtrClear (sizeof (*state)));

		if (!state)
			err = ::MemError ( );
		else
		{
			GrafPtr curPort;
			
			::GetPort( &curPort );
			
			state->pnPixPat = NULL;
			state->bkPixPat = NULL;

			state->colorPort = ::IsColorGrafPort( curPort );
			
			// Save the black and white information always
			
			state->bkPatternIsValid = true;
			state->bkPat = curPort->bkPat;
			state->bkColor = curPort->bkColor;
			state->fgColor = curPort->fgColor;

			if ( state->colorPort )
			{
				::GetForeColor( &state->foreColor );
				::GetBackColor( &state->backColor );
				
				//
				// If the pen pattern is not an old style pattern,
				// copy the handle. If it is an old style pattern,
				// GetPenState below will save the right thing.
				//

				PixPatHandle	penPixPat = ((CGrafPtr)curPort)->pnPixPat;
				PixPatHandle	backPixPat = ((CGrafPtr)curPort)->bkPixPat;
				
				if ( penPixPat != NULL )
					if( (**penPixPat).patType != 0 )
						state->pnPixPat = penPixPat;

				//
				// If the back pattern is not an old style pattern,
				// copy the handle, else get the old pattern into
				// bkPat for restoring that way.
				//
				
				if( backPixPat != NULL )
				{
					if ( (**backPixPat).patType != 0 )
						state->bkPixPat = backPixPat;
					else
						state->bkPat = *(PatPtr)(*(**backPixPat).patData);
					
					state->bkPatternIsValid = true;
				}
				else
				{
					state->bkPatternIsValid = false;
				}
				
			}
			
			::GetPenState( &state->pen );
			state->textMode = curPort->txMode;

			*stateResult = state;
		}
	}

#endif

	return err;
}

pascal OSStatus MoreNormalizeThemeDrawingState (void)
{
	//
	//	mostly stolen from
	//		{CommonSystem}:Toolbox:ToolboxUtils:CommonUtilities:ColorPenState.c
	//

	OSStatus err = noErr;

#if TARGET_CPU_PPC

	if (::GetAppearanceVersion ( ) >= 0x0110 && ::NormalizeThemeDrawingState)
		err = NormalizeThemeDrawingState ( );
	else

#endif

#if TARGET_API_MAC_CARBON
		err = unimpErr;
#else

	{
		static Pattern	whitePat = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		GrafPtr			curPort;
		
		::GetPort( &curPort );
		::ForeColor( blackColor );
		::BackColor( whiteColor );
		::PenNormal ( );
		::BackPat( &whitePat );
		::TextMode( srcOr );
	}

#endif

	return err;
}

pascal OSStatus MoreSetThemeDrawingState (MoreThemeDrawingState state, Boolean disposeNow)
{
	//
	//	mostly stolen from
	//		{CommonSystem}:Toolbox:ToolboxUtils:CommonUtilities:ColorPenState.c
	//	and
	//		{CommonSystem}:Toolbox:Appearance:AppearanceCore:ThemeDrawingState.cp
	//

	OSStatus err = noErr;

	if (!state)
		err = paramErr;
	else

#if TARGET_CPU_PPC

	if (::GetAppearanceVersion ( ) >= 0x0110 && ::SetThemeDrawingState)
		err = ::SetThemeDrawingState (ThemeDrawingState (state), disposeNow);
	else

#endif

#if TARGET_API_MAC_CARBON
		err = unimpErr;
#else

	{
		GrafPtr curPort;
		
		::GetPort( &curPort );

		::SetPenState (&(state->pen));

		//
		// If we saved color information, and this port is a
		// color port, use the color stuff, else just use the
		// black and white information.
		//
	
		if ( ::IsColorGrafPort( curPort ) && state->colorPort )
		{
			::RGBForeColor( &state->foreColor );
			::RGBBackColor( &state->backColor );

			if ( state->pnPixPat != NULL )
				::PenPixPat( state->pnPixPat );
			
			if( state->bkPatternIsValid )
			{
				if ( state->bkPixPat != NULL )
					::BackPixPat( state->bkPixPat );
				else
					::BackPat( &state->bkPat );
			}
		}
		else
		{
			//
			// back pattern is always valid for monochrome ports
			//
		
			::BackPat( &state->bkPat );
			::ForeColor( state->fgColor );
			::BackColor( state->bkColor );
		}

		::TextMode( state->textMode );

		if (disposeNow)
		{
			::DisposePtr (Ptr (state));
			err = ::MemError ( );
		}
	}

#endif

	return err;
}

pascal OSStatus MoreDisposeThemeDrawingState (MoreThemeDrawingState state)
{
	//
	//	mostly stolen from
	//		{CommonSystem}:Toolbox:Appearance:AppearanceCore:ThemeDrawingState.cp
	//

	OSStatus err = noErr;

	if (!state)
		err = paramErr;
	else

#if TARGET_CPU_PPC

	if (::GetAppearanceVersion ( ) >= 0x0110 && ::DisposeThemeDrawingState)
		err = ::DisposeThemeDrawingState (ThemeDrawingState (state));
	else

#endif

#if TARGET_API_MAC_CARBON
		err = unimpErr;
#else

	{
		::DisposePtr (Ptr (state));
		err = ::MemError ( );
	}

#endif

	return err;
}
