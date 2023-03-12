/*
*	File:		RecentItems.h of Rails
* 
*	Created:	10/29/05
*
*	Contains:	Definition of the interfaces to <RecentItems.c>
*	
*	Copyright: Copyright � 2006 Apple Computer, Inc., All Rights Reserved
* 
*	Disclaimer:	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc. ( "Apple" ) in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms. If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software ( the "Apple Software" ), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple. Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __RecentItems__
#define __RecentItems__

//****************************************************
#pragma mark - includes & imports
//----------------------------------------------------

#include <Carbon/Carbon.h>

//****************************************************
#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif
	
#if PRAGMA_IMPORT
#pragma import on
#endif
	
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack( push, 2 )
#elif PRAGMA_STRUCT_PACK
#pragma pack( 2 )
#endif

//****************************************************
#pragma mark - typedef's, struct's, enums, defines, etc.
//----------------------------------------------------

//****************************************************
#pragma mark - exported globals
//----------------------------------------------------

//****************************************************
#pragma mark - exported function prototypes
//----------------------------------------------------

/*************************************************************************
*
* RecentItems_Update( inPrefKeyCFStringRef, inFSRef, inSynchronizePrefs )
*
* Purpose: Adds or updates a recent item to our prefs
*
* Inputs:	inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*			inFSRef					- RSRef for the recent file/folder
*			inSynchronizePrefs		- TRUE to CFPreferencesAppSynchronize (for best performance only on last item update)
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_Update( CFStringRef inPrefKeyCFStringRef, const FSRef* inFSRef, Boolean inSynchronizePrefs );

/*************************************************************************
*
* RecentItems_Clear( inPrefKeyCFStringRef )
*
* Purpose:	clears our recent pref
*
* Inputs:	inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_Clear( CFStringRef inPrefKeyCFStringRef );

/*************************************************************************
*
* RecentItems_PopulateMenu( inMenuRef, inPrefKeyCFStringRef, inMenuCommand )
*
* Purpose: Populates menu with recent docs from prefs
*
* Inputs:	inMenuRef				- MenuRef for the menu we're to populate
*			inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*			inMenuCommand			- the command to use for these items
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_PopulateMenu( MenuRef inMenuRef, CFStringRef inPrefKeyCFStringRef, MenuCommand inMenuCommand );

/*************************************************************************
*
* RecentItems_GetMenuItemFile( inMenuRef, inMenuItemIndex, ioFSRef )
*
* Purpose: returns the FSRef for the specified menu item
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*			inMenuItemIndex	- MenuItemIndex for the item in the menu
*			ioFSRef			- address where to put the FSRef for this menu item
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_GetMenuItemFile( MenuRef inMenuRef, MenuItemIndex inMenuItemIndex, FSRef* ioFSRef );

/*************************************************************************
*
* RecentItems_DisableOpenItems( inMenuRef )
*
* Purpose: disable any menu items that are currently open
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_DisableOpenItems( MenuRef inMenuRef );

/*************************************************************************
*
* RecentItems_SetMaxItems( inMenuRef, inMax )
*
* Purpose: Set the maximum number of recent items remembered for this menu
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*		  inMax			- the new maximum for this menu
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_SetMaxItems( MenuRef inMenuRef, UInt16 inMax );

/*************************************************************************
*
* RecentItems_GetMaxItems( inMenuRef, outMax )
*
* Purpose: Get the maximum number of recent items remembered for this menu
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*
* Outputs:	outMax		- where to store the max
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

extern OSStatus RecentItems_GetMaxItems( MenuRef inMenuRef, UInt16* outMax );
	
//****************************************************
#if PRAGMA_STRUCT_ALIGN

#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
#pragma pack( pop )
#elif PRAGMA_STRUCT_PACK
#pragma pack( )
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif	// __RecentItems__
