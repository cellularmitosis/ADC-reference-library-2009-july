// SKTAlignCommand.m
// Sketch Example
//

#import "SKTAlignCommand.h"
#import "SKTGraphic.h"
#import "my.h"

#define	kSKTAlignCommandEdgeTop			'top '
#define	kSKTAlignCommandEdgeBottom		'bott'
#define	kSKTAlignCommandEdgeVertical	'verc'
#define	kSKTAlignCommandEdgeLeft		'left'
#define	kSKTAlignCommandEdgeRight		'righ'
#define	kSKTAlignCommandEdgeHorizontal	'horc'

@implementation SKTAlignCommand : NSScriptCommand

-(id)performDefaultImplementation
{
	myLog1(@"ME SKTAlignCommand performDefaultImplementation");

	SInt32	theError = noErr;
	id		directParameter = [self directParameter];

	myLog2(@"ME SKTAlignCommand performDefaultImplementation directParameter = %@",directParameter);

	NSMutableArray*	returnValue = [NSMutableArray array];

	if ( [directParameter isKindOfClass:[NSArray class]] != NO )
	{
		NSEnumerator*	objectEnumerator = [directParameter objectEnumerator];
		id				anObject = nil;

		while ( ( anObject = [objectEnumerator nextObject] ) != nil )
		{
			if ( [anObject isKindOfClass:[NSScriptObjectSpecifier class]] != NO )
			{
				id	resolvedObject = [anObject objectsByEvaluatingSpecifier];

				myLog2(@"ME SKTAlignCommand performDefaultImplementation resolvedObject = %@",resolvedObject);

				if ( [resolvedObject isKindOfClass:[NSArray class]] != NO )
				{
					[returnValue addObjectsFromArray:resolvedObject];
				}
				else
				{
					[returnValue addObject:resolvedObject];
				}
			}
		}
	}

	NSDictionary*	theArgs = [self evaluatedArguments];
	NSNumber*		theEdgeObject = [theArgs objectForKey:@"toEdge"];

	if ( [theEdgeObject isKindOfClass:[NSNumber class]] != NO )
	{
		long	theEdgeValue = [theEdgeObject longValue];

		unsigned	j, m;
		NSRect		firstBounds = [[returnValue objectAtIndex:0] bounds];

		switch ( theEdgeValue )
		{
		case	kSKTAlignCommandEdgeTop:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( curBounds.origin.y != firstBounds.origin.y )
				{
					curBounds.origin.y = firstBounds.origin.y;
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		case	kSKTAlignCommandEdgeBottom:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( NSMaxY( curBounds ) != NSMaxY( firstBounds ) )
				{
					curBounds.origin.y = NSMaxY( firstBounds ) - curBounds.size.height;
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		case	kSKTAlignCommandEdgeVertical:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( NSMidY( curBounds ) != NSMidY( firstBounds ) )
				{
					curBounds.origin.y = NSMidY( firstBounds ) - ( curBounds.size.height / 2.0 );
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		case	kSKTAlignCommandEdgeLeft:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( curBounds.origin.x != firstBounds.origin.x )
				{
					curBounds.origin.x = firstBounds.origin.x;
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		case	kSKTAlignCommandEdgeRight:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( NSMaxX( curBounds ) != NSMaxX( firstBounds ) )
				{
					curBounds.origin.x = NSMaxX( firstBounds ) - curBounds.size.width;
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		case	kSKTAlignCommandEdgeHorizontal:

			for ( j = 0, m = [returnValue count]; m > 0; j++, m-- )
			{
				SKTGraphic*	curGraphic = [returnValue objectAtIndex:j];
				NSRect		curBounds = [curGraphic bounds];

				if ( NSMidX( curBounds ) != NSMidX( firstBounds ) )
				{
					curBounds.origin.x = NSMidX( firstBounds ) - ( curBounds.size.width / 2.0 );
					[curGraphic setBounds:curBounds];
				}
			}
			break;

		default:
			theError = errAECoercionFail;
			break;
		}
	}

	if ( theError != noErr )
	{
		//ME	report the error, if any
		[self setScriptErrorNumber:theError];
	}

	return	returnValue;
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
