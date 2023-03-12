/*
    File: InkSampleUtils.cpp
        
    Contains: Utility functions
    
    Version: InkSample 1.1

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
        ("Apple") in consideration of your agreement to the following terms, and your
        use, installation, modification or redistribution of this Apple software
        constitutes acceptance of these terms.  If you do not agree with these terms,
        please do not use, install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and subject
        to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
        
    Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
*/

#include <iostream> // for std::cerr
#include "InkSampleUtils.h"
#include "InkDefines.h"

void
OutputCFStringToStdErr( CFStringRef cfString )
{
    CFShow( cfString );
    #if 0
    if( cfString != NULL )
    {
        CFIndex len = CFStringGetLength( cfString );
        if( len > 1 )
        {
            std::cerr << CFStringGetCStringPtr( cfString, kCFStringEncodingMacRoman );
            std::cerr << " (len=" << len << ")";
        }
        else if( len == 1 && CFStringGetCStringPtr( cfString, kCFStringEncodingMacRoman ) != NULL )
        {
            char c = *CFStringGetCStringPtr( cfString, kCFStringEncodingMacRoman );
            std::cerr << c;
        }
        else
            std::cerr << "-- can't output CFString --" << std::endl;
    }
    #endif
}

char*
GetStringForInkUISignal( OSType signalInkEventKind, char* string )
{
    // Used to convert OSType to zero terminated c string
    UInt32 signalString[2] = { signalInkEventKind, 0 };
    
    switch( signalInkEventKind )
    {
        /* Context-independent (non-tentative) gesturesÉ*/
        case kInkGestureUndo: //'undo'
        case kInkGestureCut: //'cut '
        case kInkGestureCopy: //'copy'
        case kInkGestureTab: //'tab '
        case kInkGestureJoin: //'join'
        
            // the OSType four char code for these cases
            // contains the full word
            // so we'll just use it directly to set the
            // string for the static text indicator
            sprintf(string, "%4s", (char*)signalString);
            break;
            
        case kInkGesturePaste: //'past'
            sprintf(string, "%s", "paste");
            break;
        case kInkGestureClear: //'cler'
            sprintf(string, "%s", "clear");
            break;
        case kInkGestureSelectAll: //'sall'
            sprintf(string, "%s", "select all");
            break;
        case kInkGestureEscape: //'esc '
            sprintf(string, "%s", "escape (cancel)");
            break;
        case kInkGestureLeftSpace: //'lspc'
            sprintf(string, "%s", "left space");
            break;
        case kInkGestureRightSpace: //'rspc'
            sprintf(string, "%s", "right space");
            break;
        case kInkGestureLeftReturn: //'lrtn'
            sprintf(string, "%s", "left return");
            break;
        case kInkGestureRightReturn: //'rrtn'
            sprintf(string, "%s", "right return");
            break;
            break;
        case kInkGestureDelete: //'del '
            sprintf(string, "%s", "delete");
            break;
        
        // custom flag
        case kMyInkTextEventSignal: //'InkT'
            sprintf( string, "%s", "ink text event" );
            break;
        case kMyInkShortcutEventSignal: //'ShrT'
            sprintf( string, "%s", "ink generated keyboard shortcut" );
            break;
        default:
            sprintf(string, "%s", "unidentified gesture???");        
            break;
    }
    return string;
}

