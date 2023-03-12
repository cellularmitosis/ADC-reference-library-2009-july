/*
	File:		CustomDataTypes.mm

	Abstract:	Implements custom objects used by the Audio Extraction
				Window Controller.
	
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

#import "CustomDataTypes.h"

@implementation InfoObject
+(id) infoObjectWithItem:(UInt32)item andItemName:(NSString *)itemName
{
	return [[(InfoObject *)[self alloc] initWithItem:item andItemName:itemName] autorelease];
}

- (id)initWithItem:(UInt32)item andItemName:(NSString *)itemName
{
	[self init];
	mItem = item;
	mItemName = [[NSString alloc] initWithString:itemName];
	return self;

}
- (id)init
{
	[super init];
	mItem = 0; 
	mItemName = nil;
	return self;
}

- (void)dealloc
{
	[mItemName release];
	[super dealloc];
}

#pragma mark
#pragma mark ---- Getters ---------

- (UInt32)item
{
	return mItem;
}

- (NSString *)itemName
{
	return mItemName;
}

#pragma mark
#pragma mark ---- Setters ---------

- (void)setItem:(UInt32)theItem
{
	mItem = theItem;
}

- (void)setItemName:(NSString*)theItemName
{
	[theItemName retain];
	[mItemName release];
	mItemName = theItemName;
}

- (void)setItem:(UInt32)theItem andItemName:(NSString*)theItemName
{
	mItem = theItem;
	[theItemName retain];
	[mItemName release];
	mItemName = theItemName;
}

@end

@implementation InfoForCallback

- (id) init
{
	[super init];
	bzero(&mASBD, sizeof (mASBD));
	mLayout	= nil;
	mLayoutSize = 0;
	mStartTime.timeValue = 0;
	mStartTime.timeScale = 0;
	mDiscrete = NO;
	mSamplesRemaining = 0;
	bzero(&mInsertRegInfo, sizeof(mInsertRegInfo));
	mLocationInFile = 0;
	mPlayerUnitStarted = NO;
	mSliceList = nil;
	mSampleTimeStamp = 0.;
	mExtractionSessionRef = nil;

	return self;
}

#pragma mark
#pragma mark ---- getters ----------------

- (AudioChannelLayout*)layout
{
	return mLayout;
}

- (UInt32)layoutSize
{
	return mLayoutSize;
}

- (QTTime)startTime
{
	return mStartTime;
}
- (Boolean)discrete
{
	return mDiscrete;
}
-(SInt64)samplesRemaining;
{
	return mSamplesRemaining;
}

- (InsertRegistryInfo)insertRegInfo
{
	return mInsertRegInfo;
}

- (AudioStreamBasicDescription)asbd
{
	return mASBD;
}

- (SInt64)locationInFile
{
	return mLocationInFile;
}

- (Boolean)playerUnitStarted
{
	return mPlayerUnitStarted;
}

- (ScheduledAudioSlice*)sliceList
{
	return mSliceList;
}

- (Float64)sampleTimeStamp
{
	return mSampleTimeStamp;
}

- (MovieAudioExtractionRef)extractionSessionRef
{
	return mExtractionSessionRef;
}

#pragma mark
#pragma mark ---- setters ----------------

- (void)setASBD:(AudioStreamBasicDescription)asbd
{
	mASBD = asbd;
}

- (void)setLayout:(AudioChannelLayout*)layout
{
	mLayout = layout;
}

- (void)setLayoutSize:(UInt32)layoutSize
{
	mLayoutSize = layoutSize;
}

- (void)setDiscrete:(Boolean)discrete
{
	mDiscrete = discrete;
}

- (void)setSamplesRemaining:(SInt64)samplesRemaining
{
	mSamplesRemaining = samplesRemaining;
}

- (void) setInsertRegInfo:(InsertRegistryInfo)regInfo
{
	mInsertRegInfo = regInfo;
}

- (void)setStartTime:(QTTime)startTime
{
	mStartTime = startTime;
}

-(void)setLocationInFile:(SInt64)location
{
	mLocationInFile = location;
}

- (void)setPlayerUnitStarted:(Boolean)playerUnitStarted
{
	mPlayerUnitStarted = playerUnitStarted;
}

- (void)setSliceList:(ScheduledAudioSlice*)sliceList
{
	mSliceList = sliceList;
}

- (void)setSampleTimeStamp:(Float64)sampleTimeStamp
{
	mSampleTimeStamp = sampleTimeStamp; 
}

-(void)setExtractionSession:(MovieAudioExtractionRef)extractionSessionRef
{
	mExtractionSessionRef = extractionSessionRef;
}

@end
