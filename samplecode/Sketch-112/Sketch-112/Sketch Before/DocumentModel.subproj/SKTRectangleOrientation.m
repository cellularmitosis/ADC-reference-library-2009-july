// SKTRectangleOrientation.m
// Sketch Example
//

#import "SKTRectangleOrientation.h"
#import "SKTScriptingExtras.h"
#import "my.h"

#define	kSKTRectangleScriptingOrientationSquare		'squa'
#define	kSKTRectangleScriptingOrientationLandscape	'land'
#define	kSKTRectangleScriptingOrientationPortrait	'port'

@implementation SKTRectangle (SKTRectangleOrientation)

-(unsigned)orientation
{
	myLog1(@"ME SKTRectangleOrientation orientation");

	//ME	might be a square
	unsigned	returnValue = kSKTRectangleScriptingOrientationSquare;

	float	theHeight = [self height];
	float	theWidth = [self width];

	if ( theHeight > theWidth )
	{
		//ME	taller
		returnValue = kSKTRectangleScriptingOrientationPortrait;
	}
	else if ( theHeight < theWidth )
	{
		//ME	wider
		returnValue = kSKTRectangleScriptingOrientationLandscape;
	}

	return	returnValue;
}

-(void)setOrientation:(unsigned)orientation
{
	myLog2(@"ME SKTRectangleOrientation setOrientation: %d",orientation);

	SInt32		theError = noErr;
	unsigned	priorOrientation = [self orientation];

	switch ( orientation )
	{
	case	kSKTRectangleScriptingOrientationLandscape:
	case	kSKTRectangleScriptingOrientationPortrait:

		if ( orientation != priorOrientation )
		{
			switch ( priorOrientation )
			{
			case	kSKTRectangleScriptingOrientationLandscape:
			case	kSKTRectangleScriptingOrientationPortrait:
				//ME	invert height and width
				{
					float	theHeight = [self height];
					float	theWidth = [self width];

					[self setWidth:theHeight];
					[self setHeight:theWidth];
				}
				break;

			case	kSKTRectangleScriptingOrientationSquare:
				//ME	this is already a square, so there's nothing to do
				break;

			default:
				//ME	this can't happen
				theError = errAECorruptData;
				break;
			}
		}
		break;

	case	kSKTRectangleScriptingOrientationSquare:

		if ( orientation != priorOrientation )
		{
			//ME	this is not a square, so don't force it
			theError = errAEPropertiesClash;
		}
		break;

	default:
		//ME	this is not a valid orientation
		theError = errAECoercionFail;
		break;
	}

	if ( theError != noErr )
	{
		//ME	report the error, if any
		[[[NSAppleEventManager sharedAppleEventManager] currentReplyAppleEvent] setDescriptor:[NSAppleEventDescriptor descriptorWithInt32:theError] forKeyword:keyErrorNumber];
	}
}

@end

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
