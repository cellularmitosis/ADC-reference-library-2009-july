/*
	File:		ProxyDialog.cp

	Contains:	Demonstrates window proxy installation

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

	Copyright © 1998-2001 Apple Computer, Inc., All Rights Reserved
*/


#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "ProxyDialog.h"

const ControlID	kImageWell 			= { 'PRXY', 1 };
const ControlID	kModifiedCheckBox 	= { 'PRXY', 2 };

enum
{
	kFileSpecCmd		= 'FSPC',
	kAliasCmd			= 'ALIS',
	kIconCmd			= 'ICON',
	kRemoveCmd			= 'RMVE',
	kModifiedCmd		= 'MODI'
};

static Boolean GetAFile(FSSpec *fileSpec);

ProxyDialog::ProxyDialog()
	: TWindow( appNibName, CFSTR( "Proxy" ) )
{
	::SetThemeWindowBackground( GetWindowRef(), kThemeBrushDialogBackgroundActive, false );
	Show();
}

ProxyDialog::~ProxyDialog()
{
}

static Boolean GetAFile( FSSpec *fileSpec )
{
	NavReplyRecord		reply;
	OSErr				err;
	NavDialogOptions	options;
	Boolean				result = false;
	
	NavGetDefaultDialogOptions( &options );

	options.dialogOptionFlags &= ~kNavAllowMultipleFiles;

	err = NavGetFile( NULL, &reply, &options,
								NULL, NULL, NULL, NULL, NULL);
	
	if ( (err == noErr) && reply.validRecord )
	{
		AEDesc		fileSpecDesc = { typeNull, nil };
		AEKeyword	unusedKeyword;

		err = AEGetNthDesc( &reply.selection, 1, typeFSS, &unusedKeyword, &fileSpecDesc );

		if ( err == noErr )
			AEGetDescData(&fileSpecDesc, fileSpec, sizeof(FSSpec) );

		AEDisposeDesc( &fileSpecDesc );
	
		result = true;
	}

	NavDisposeReply( &reply );

	return result;
}

void
ProxyDialog::UpdateContentIcon()
{
	ControlHandle 				theImageWell;
	IconRef 					theIcon;
	ControlButtonContentInfo 	myContentInfo;
	OSStatus 					theError = noErr;

	::GetControlByID( GetWindowRef(), &kImageWell, &theImageWell );

	theError = ::GetWindowProxyIcon( GetWindowRef(), &theIcon );

	if ( theError == noErr )
	{
		myContentInfo.contentType = kControlContentIconRef;
		myContentInfo.u.iconRef = theIcon;
		
		theError = ::SetImageWellContentInfo( theImageWell, &myContentInfo );
		check_noerr( theError );
		
		::DrawOneControl( theImageWell );
	}
	else
	{
		myContentInfo.contentType = kControlNoContent;
		
		theError = ::SetImageWellContentInfo( theImageWell, &myContentInfo );
		check_noerr( theError );
		
		::DrawOneControl( theImageWell );
	}
}

void
ProxyDialog::DoAddProxyIcon( Boolean fromAlias )
{
	FSSpec 	fileSpec = { 0, 0, "\p" };
	OSErr 	theError;
	
	if ( GetAFile( &fileSpec ) )
	{
		if ( !fromAlias )
		{
			theError = ::SetWindowProxyFSSpec( GetWindowRef(), &fileSpec );
			check_noerr( theError );
		}
		else
		{
			AliasHandle theAlias;
				
			// In the real world you'd already have an alias.. but we're just converting
			// the FSSpec we just got *into* an alias, and using the alternate call.	
			if ( ::NewAliasMinimal( &fileSpec, &theAlias ) == noErr )
			{
				theError = ::SetWindowProxyAlias( GetWindowRef(), theAlias );
				check_noerr( theError );
			}
		}

		UpdateContentIcon();
	}
}

Boolean
ProxyDialog::HandleCommand( UInt32 commandID )
{
	OSStatus 		err = noErr;
	ControlHandle 	modifiedCheckbox;
	Boolean			handled = true;
	
	switch( commandID )
	{
		case kRemoveCmd:
			err = ::RemoveWindowProxy( GetWindowRef() );
			check_noerr( err );
			UpdateContentIcon();
			break;
			
		case kFileSpecCmd:
			DoAddProxyIcon( false );
			break;
			
		case kAliasCmd:
			DoAddProxyIcon( true );
			break;
			
		case kIconCmd:
			err = ::SetWindowProxyCreatorAndType( GetWindowRef(), 'ttxt','TEXT', kOnSystemDisk );
			if ( err == noErr )
				UpdateContentIcon();
			break;
			
		case kModifiedCmd:
			::GetControlByID( GetWindowRef(), &kModifiedCheckBox, &modifiedCheckbox );
			::SetWindowModified( GetWindowRef(), ::GetControlValue( modifiedCheckbox ) );
			break;
		
		default:
			handled = TWindow::HandleCommand( commandID );
			break;
	}
	
	return handled;
}
