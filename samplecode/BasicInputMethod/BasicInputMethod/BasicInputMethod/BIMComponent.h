/*
    File:	 BIMComponent.h

    Description: Main entry points for our text service routines. These routines are called by
                 the Component Manager and the Text Services Manager. Here we do some basic
                 housekeeping, but for the actual functionality of our text service, we will call
                 our core implmentation routines in BIM.c.

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

                 2001/04/18	SC	Renamed component routines to start with BIM for consistency
                 2000/07/28	SC	Changed to include Carbon.h
                 2000/06/01	SC	Created

*/

#ifndef BIMComponent_h
#define BIMComponent_h

#include <Carbon/Carbon.h>

//  These first four entry points are standard for any component.

pascal ComponentResult BIMOpenComponent( ComponentInstance inComponentInstance );
pascal ComponentResult BIMCloseComponent( Handle inSessionHandle, ComponentInstance inComponentInstance );
pascal ComponentResult BIMCanDo( SInt16 inSelector );
pascal ComponentResult BIMGetVersion( void );

//  The next entry points are required for any text service component.

pascal ComponentResult BIMGetScriptLangSupport( Handle inSessionHandle,
                                              ScriptLanguageSupportHandle *outScriptHandle );
pascal ComponentResult BIMInitiateTextService( Handle inSessionHandle );
pascal ComponentResult BIMTerminateTextService( Handle inSessionHandle );
pascal ComponentResult BIMActivateTextService( Handle inSessionHandle );
pascal ComponentResult BIMDeactivateTextService( Handle inSessionHandle );
pascal ComponentResult BIMTextServiceEventRef( Handle inSessionHandle, EventRef inEventRef );
pascal ComponentResult BIMGetTextServiceMenu( Handle inSessionHandle, MenuHandle *outMenuHandle );
pascal ComponentResult BIMFixTextService( Handle inSessionHandle );
pascal ComponentResult BIMHidePaletteWindows( Handle inSessionHandle );

//  The following constants are used to create universal procedure pointers for our dispatch routines.
//  Each uppXXXProcInfo constant defines the parameters passed to (and returned from) one of our component
//  routines; for example, uppInitiateTextServiceProcInfo describes the parameters passed to the
//  _InitiateTextService() routine, and what it returns to the caller.

enum {
    uppOpenComponentProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ComponentInstance))),

    uppCloseComponentProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
    | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ComponentInstance))),

    uppCanDoProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))),

    uppGetVersionProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult))),

    uppGetScriptLangSupportProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
    | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ScriptLanguageSupportHandle *))),

    uppInitiateTextServiceProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),

    uppTerminateTextServiceProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),

    uppActivateTextServiceProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),

    uppDeactivateTextServiceProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),

    uppTextServiceEventRefProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
    | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EventRef))),

    uppGetTextServiceMenuProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
    | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MenuHandle *))),

    uppFixTextServiceProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),

    uppHidePaletteWindowsProcInfo = kPascalStackBased
    | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
    | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
};

#endif
