/*
*	File:		TypeServicesForUnicode.h
* 
*	Contains:	This is a sample program that demonstrates how to use
*					the Apple Text Services For Unicode Imaging (ATSUI) technology
*					in a Carbon Events based HIView architectured application.
*
*					The TypeForServices code handles:
*						- showing how to set the text
*						- showing how to set the text and styles
*						- showing how to rotate
*						- showing how to handle font variants
*						- showing how to handle font selection through the standard Font menu
*						- showing how to handle font selection through the standard Font panel
*						- showing how to instantiate the HIATSUIView from a nib
*						- showing how to read a text file and set the HIATSUIView content
*						- showing how to handle the clipboard
*						- showing how to handle background pictures
*
*  Note:		The project is set up so that the DEBUG macro is set to one when the "Development"
*				build style is chosen and not at all when the "Deployment" build style is chosen.
*				Thus, all the require asserts "fire" only in "Development".
*	
*	Version:	2.0
* 
*	Created:	11/4/04
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÍs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

extern const HIViewID kAtsuiID;

enum
	{
	kEventClassATSUI = 'AtsC'
	};

enum
	{
	kEventATSUISetFont = 1					// parameter: the name of the font as a CFString
	};

//****************************************************
#pragma mark -
#pragma mark * exported globals *

OSStatus Do_NewSingleLineSingleStyleWindow(void);
OSStatus Do_NewSingleLineMultipleStyleWindow(void);
OSStatus Do_NewSingleVerticalLineMultipleStyleWindow(void);
OSStatus Do_NewParagraphsWindow(void);
OSStatus Do_NewHighlightingWindow(void);
OSStatus Do_NewHitTestingWindow(void);
pascal OSStatus Handle_AppActivated(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
OSStatus Do_ShowClipboardWindow(void);
OSStatus Do_NewTextBoxWindow(IBNibRef nibRef);
OSStatus Do_NewUsingFontVariantsWindow(void);
OSStatus Do_NewScrollingWindow(void);
OSStatus Do_CreateWithTextWindow(void);
OSStatus Do_CreateFromNibWindow(IBNibRef nibRef);
OSStatus Do_OpenATextFile(void);

OSStatus Handle_FontMenuSelection(const HICommand * hiCommand);
OSStatus Handle_SelectAll(void);
OSStatus Handle_Copy(void);
void Get_FrontWindowAndATSUIView(WindowRef * outWindow, HIViewRef * outATSUIView, Boolean deep);

#ifdef __cplusplus
}
#endif
