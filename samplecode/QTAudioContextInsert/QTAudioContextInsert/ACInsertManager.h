/*
	File:		ACInsertManager.h	

	Abstract:	Implements the Audio Context Insert Manager which functions as
	            a factory for creating and configuring Audio Context insert 
				processor objects. At any given time, the ACInsertManager has 
				one processor instance associated with it as the 'current processor'.
				This is the instance that responds to the AC Insert	window controller
				UI. The Audio Context Manager may also have an 'extraction processor'
				instance which is used for insert processing during extraction
				and extraction preview. 
	
	Version:	1.0

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
	Computer, Inc. ("Apple") in consideration of your agreement to the
	following terms, and your use, installation, modification or
	redistribution of this Apple software constitutes acceptance of these
	terms.  If you do not agree with these terms, please do not use,
	install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple's copyrights in this original Apple software (the
	"Apple Software"), to use, reproduce, modify and redistribute the Apple
	Software, with or without modifications, in source and/or binary forms;
	provided that if you redistribute the Apple Software in its entirety and
	without modifications, you must retain this notice and the following
	text and disclaimers in all such redistributions of the Apple Software. 
	Neither the name, trademarks, service marks or logos of Apple Computer,
	Inc. may be used to endorse or promote products derived from the Apple
	Software without specific prior written permission from Apple.  Except
	as expressly stated in this notice, no other rights or licenses, express
	or implied, are granted by Apple herein, including but not limited to
	any patent rights that may be infringed by your derivative works or by
	other works in which the Apple Software may be incorporated.

	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
	MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
	THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
	OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
	AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
	STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2006-2008 Apple Inc. All Rights Reserved.
*/

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>

#include "MovieDocument.h"
#import "ACInsertWindowController.h"
#import "ACInsertProcessor.h"

@class MovieDocument;
@class ACInsertWindowController;

@interface ACInsertManager : NSObject  
{
		// movie document
	MovieDocument			*mMovieDocument;
	
	ACInsertProcessor		*mCurrentProcessor;
	Component				mCurrentProcessorComponent;
	ACInsertProcessor		*mCurrentExtractionProcessor;
	id						mCurrentInsertDestination;
}

- (id) initWithMovieDocument:(MovieDocument *)movieDocument;

	// getters
- (MovieDocument *)document;
- (AudioUnit)currentAU;

- (void) createInsertProcessorForAU:(Component)ComponentRecord;
- (void) setInsertInputLayout:(AudioChannelLayoutTag)layoutTag;
- (void) setInsertOutputLayout:(AudioChannelLayoutTag)layoutTag;
- (void) setInsertBypassed:(UInt32)isBypassed;

- (void) insertIsBypassable:(BOOL*)isBypassable currentBypassState:(UInt32*)bypassState;
- (BOOL) insertCanDoInputChannels:(UInt32)inputNumChannels outputNumChannels:(UInt32)outputNumChannels;
- (BOOL) insertCanDoOutputChannels:(UInt32)numChannels;

- (void) createProcessorForExtractionAndGetRegistrationInfo:(InsertRegistryInfo*)regInfoRef;
- (void) cloneProcessor:(ACInsertProcessor *)newProcessor 
						fromProcessor:(ACInsertProcessor *)origProcessor
						setInputLayout:(BOOL)setInputLayout
						setOutputLayout:(BOOL)setOutputLayout;

- (OSStatus)attachDetachInsert:(BOOL)attach;
- (OSStatus)attachDetachMovieLevelInsert:(BOOL)attach;
- (OSStatus)attachDetachTrackLevelInsert:(BOOL)attach track:(QTTrack*)trackForInsert;
- (OSStatus)insertDestinationChangedFrom:(id)newDestination to:(id)oldDestination;

@end
