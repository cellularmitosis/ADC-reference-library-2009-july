/*
	File:		MyObject.m
	
	Description: MyObject class file. Contains code showing how to bring up the 
                QuickTime Standard Effects Parameter dialog.
    
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

    8/15/03		srk		first

*/

#import "MyObject.h"

#define BailErr(x) {if (x != noErr) goto bail;}

@implementation MyObject

static QTParameterDialog gParamDlg = nil;

static pascal Boolean effectDialogModalFilter(DialogRef dialog, EventRecord
*eventPtr, DialogItemIndex *itemHit);

//////////
//
// awakeFromNib
// Invoked to unarchive objects from our nib file
// We'll use this opportunity to initialize the Movie
// Toolbox.
//
//////////

- (void)awakeFromNib
{
    EnterMovies();	// initialize Movie Toolbox
}

//////////
//
// effectDialogModalFilter
// Filter procedure which allows us to trap events for
// the dialog. We'll look for ok or cancel button presses
// and pass these back to our event loop.
//
//////////

static pascal Boolean effectDialogModalFilter(DialogRef dialog, EventRecord
*eventPtr, DialogItemIndex *itemHit)
{
  OSErr   dlgResult = QTIsStandardParameterDialogEvent ( eventPtr, gParamDlg);

  switch (dlgResult)
  {
    case codecParameterDialogConfirm:
        *itemHit = ok;
    break;
    
    case userCanceledErr:
        *itemHit = cancel;
    break;
    
    default:
        *itemHit = 0;
  }

  return true;
}

//////////
//
// prepareNullEvent
// Feed the dialog some null events to make sure initialization 
// is complete. We need at least one to make the dialog appear.
// Additional null events may be unnecessary but can't hurt.
//
//////////

void prepareNullEvent(EventRecord *evt)
{
    // modifier key IDs
    // use these to construct our null event
    // (see below for details)
    #define cmdCode       55
    #define shiftCode     56
    #define capsLockCode  57
    #define optionCode    58
    #define controlCode   59
    
    KeyMap keys;
    
    // We need to construct a "fake" null event 
    // from scratch and pass this to the standard
    // parameter dialog to give it time to initialize.
    
    // We'll simply grab the current mouse position
    // and use that for the "where" field of the
    // EventRecord. For the current time (EventRecord 
    // "when" field) we'll just grab the TickCount value.
    evt->what 		= nullEvent;
    evt->message 	= 0;
    evt->when 		= TickCount();
    GetMouse(&evt->where);
    LocalToGlobal(&evt->where);
    
    // Get the current state of the keyboard, and 
    // use the modifier key information to fill in 
    // the "modifiers" field of the EventRecord. 
    // More specifically, we'll check for the shift, 
    // caps lock, option or control modifier keys 
    GetKeys(keys);
    evt->modifiers = 0;
    if (keys[cmdCode]) evt->modifiers |= cmdKey;
    if (keys[shiftCode]) evt->modifiers |= shiftKey;
    if (keys[capsLockCode]) evt->modifiers |= alphaLock;
    if (keys[optionCode]) evt->modifiers |= optionKey;
    if (keys[controlCode]) evt->modifiers |= controlKey;
}


//////////
//
// showQTEffectsDialog
// Bring up the QT Effects dialog.
//
//////////

- (void) showQTEffectsDialog 
{
    QTAtomContainer      fxList = nil;
    long                 minSrcs = 2;
    long                 maxSrcs = 2;
    QTEffectListOptions  options = 0;
    ModalFilterUPP       modalFilterUPP = nil;
    short                itemHit = -1;
    short                i;
    
    // Returns a QT atom container holding a list of the 
    // currently installed effects components.
    BailErr(QTGetEffectsList ( &fxList, minSrcs, maxSrcs, options ));
    
    // An effect description containing the default parameter 
    // values for the effect. Pass in an empty atom container 
    // to have the dialog box display the first effect in the list, 
    // set to its default parameters
    QTAtomContainer fxDesc = nil;
    BailErr(QTNewAtomContainer ( &fxDesc ));
    
    // dialog box should be modal
    QTEffectListOptions dOpts = pdOptionsModalDialogBox;
    
    // Creates a dialog box that allows the user to choose an
    // effect from the list of effects passed to the function
    BailErr(QTCreateStandardParameterDialog ( fxList, fxDesc, dOpts, &gParamDlg ));
    
    for (i = 0; i < 3; i++)
    {
        EventRecord event;
        
        // Feed the dialog some null events to make sure initialization is complete.
        // We need at least one to make the dialog appear.
        // Additional null events may be unnecessary but can't hurt.
        prepareNullEvent(&event);
        
        QTIsStandardParameterDialogEvent ( &event, gParamDlg );
    }
    
    // filter proc allows us to trap dialog events
    modalFilterUPP = NewModalFilterUPP(effectDialogModalFilter);
    if (modalFilterUPP)
    {
        do 
        {
            // handle dialog events
            ModalDialog(modalFilterUPP, &itemHit);
            
            // exit when ok or cancel buttons pressed
        } while ( itemHit != ok && itemHit != cancel );
    }
    
    // fxDesc now holds an effect description for the effect selected by the user
    
    //  ... your code here ... 
    
bail:

    // clean up...
    
    if (gParamDlg)
    {
        QTDismissStandardParameterDialog(gParamDlg);
        gParamDlg = nil;
    }

    if (modalFilterUPP)
        DisposeModalFilterUPP(modalFilterUPP);
    
    if (fxDesc)
        QTDisposeAtomContainer(fxDesc);

}

//////////
//
// buttonPressAction
// When the user presses our button, we will
// create and bring up the effects dialog
//
//////////

- (IBAction)buttonPressAction:(id)sender
{
    // button press brings up the QT Effects dialog
    [self showQTEffectsDialog];
}

@end
