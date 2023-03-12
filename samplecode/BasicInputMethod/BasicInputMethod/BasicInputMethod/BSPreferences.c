/*
    File:	 BSPreferences.c
	
    Description: Routines that implement preferences settings.

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

                 2001/03/09	SC	Added support for the send event palette.
                 2000/07/28	SC	Changed to include Carbon.h
                 2000/07/10	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BSPreferences.h"

//  Global variables

static Boolean gIsKeyboardPaletteHidden;
static Boolean gIsSendEventPaletteHidden;

/************************************************************************************************
*
*  BSInitializePreferences
*
*  This routine initializes our application preferences. Preferences are stored per user. In this
*  routine we read the preferences from disk (not implemented). If there are no preferences, then
*  initialize everything with default values.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSInitializePreferences( void )
{
    gIsKeyboardPaletteHidden = FALSE;
    gIsSendEventPaletteHidden = TRUE;
    return noErr;
}

/************************************************************************************************
*
*  BSGetPreferencesIsKeyboardPaletteHidden
*
*  This routine returns the current preference setting as to whether the keyboard palette
*  should be hidden or shown.
*
*	Out	Boolean			True if the current preference settings indicate that
*					the keyboard palette should be hidden, or false if the
*					keyboard palette should be shown.
*
************************************************************************************************/

Boolean BSGetPreferencesIsKeyboardPaletteHidden( void )
{
    return gIsKeyboardPaletteHidden;
}

/************************************************************************************************
*
*  BSSetPreferencesIsKeyboardPaletteHidden
*
*  This routine sets the current preference setting as to whether the keyboard palette
*  should be hidden or shown.
*
*	In	inIsHidden		True if the keyboard palette should be hidden, or false
*					if the keyboard palette should be shown.
*
************************************************************************************************/

void BSSetPreferencesIsKeyboardPaletteHidden( Boolean inIsHidden )
{
    gIsKeyboardPaletteHidden = inIsHidden;
}

/************************************************************************************************
*
*  BSGetPreferencesIsSendEventPaletteHidden
*
*  This routine returns the current preference setting as to whether the send event palette
*  should be hidden or shown.
*
*	Out	Boolean			True if the current preference settings indicate that
*					the send event palette should be hidden, or false if the
*					send event palette should be shown.
*
************************************************************************************************/

Boolean BSGetPreferencesIsSendEventPaletteHidden( void )
{
    return gIsSendEventPaletteHidden;
}

/************************************************************************************************
*
*  BSSetPreferencesIsSendEventPaletteHidden
*
*  This routine sets the current preference setting as to whether the send event palette
*  should be hidden or shown.
*
*	In	inIsHidden		True if the send event should be hidden, or false
*					if the send event should be shown.
*
************************************************************************************************/

void BSSetPreferencesIsSendEventPaletteHidden( Boolean inIsHidden )
{
    gIsSendEventPaletteHidden = inIsHidden;
}