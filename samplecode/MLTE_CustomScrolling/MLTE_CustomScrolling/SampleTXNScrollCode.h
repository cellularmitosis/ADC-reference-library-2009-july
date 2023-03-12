/*
*	File:		SampleTXNScrollCode.h of MLTECustomScrolling
* 
*	Created:	2003/12/05
*
*	Contains:	Demo declarations for functions called in main.cpp to set up an MLTE object
*              for custom scrolling.
*	
*	Copyright:  Copyright © 2004 Apple Computer, Inc., All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
 
#define _COMPILE_SAMPLE_SCROLL_CODE_ 1
#if _COMPILE_SAMPLE_SCROLL_CODE_

#include <Carbon/Carbon.h>

enum
{
    // arbitrary tag to identify scrollbar as vertical or horizontal
    // you could change these values to whatever you prefer
    kMyVerticalScrollBar = 'Vscr',
    kMyHorizontalScrollBar = 'Hscr',
    
    // arbitrary keys for storing pointers to MLTE related data in a window property
    // we use these keys to identify the pointer data stored in the window property,]
    // both for setting and getting the value back.
    // you could change these values to whatever you prefer
    kMyMLTEDataStructProperty = 'TXNd',
    kMyPropertyCreator = 'Demo'
};

// MyMLTEData is a custom data structure which holds data related to an
// instance of an MLTE object.  This data will be needed
// to support custom scrolling.  We'll store a pointer to an instance
// of this data in each window as a window property.
// You could add more MLTE related data to this struct, for example:
// you could add a FileSpec or a CFURLRef, which would be associated
// with the TXNObject.
struct
MyMLTEData
{
    TXNObject fTXNObj;
    SInt32 fVertScrollCache;
    SInt32 fHorizScrollCache;
    
    // FSSpec fFileSpec; // you could associate a FSSpec with the object and
                         // save it here.
};

// WindowStoreTXNObject will allocate the MyMLTEData structure
// and store it in the window.  We pass a key so we could associate
// more than one MyMLTEData instance with the window if want to at
// a later date.
OSStatus
WindowStoreTXNObject( WindowRef window, OSType key, TXNObject txnObj );

// Sometimes we just want to get the TXNObject from the window
// This is a simple accessor for that goal.
TXNObject
WindowGetTXNObj( WindowRef window, OSType key );

// Custom scrolling setup starts here
OSStatus
WindowRegisterTXNScrollProcs( WindowRef window, TXNObject txnObj );

// Tell window to draw its content.
OSStatus
WindowDrawContent( WindowRef window );

OSStatus
WindowInvalidateContent( WindowRef window );

OSStatus
WindowClose( WindowRef window );

// Text view should try to process the command or return eventNotHandledErr
OSStatus
TextViewProcessHICommand( WindowRef window, OSType key, const HICommand& hiCommand);

#endif //_COMPILE_SAMPLE_SCROLL_CODE_