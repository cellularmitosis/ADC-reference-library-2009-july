//
//  NSImageDifference.m
//  Image Difference
//
//  Created by jcr on Fri Sep 13 2002.
//  Copyright (c) 2002/2003 Apple Computer, Inc. All rights reserved.
//

#import "NSImageDifference.h"
#import "Negative.h"

@implementation NSImage (Difference) 

- (NSImage *) differenceFromImage:(NSImage *) otherImage
	{
	if (otherImage)
		{
		NSSize mySize = [self size];
		NSImage *negA = [self negativeImage];
		NSImage *negB = [otherImage negativeImage];
		NSImage *scratch = [[NSImage alloc] initWithSize:mySize];
		NSImage *diffImage = [[NSImage alloc] initWithSize:mySize];
		
			// Generate the positive error image
		[scratch lockFocus];
			[self compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
			[negA compositeToPoint:NSZeroPoint operation:NSCompositePlusDarker];
		[scratch unlockFocus];
		
			// Draw the positive error image
		[diffImage lockFocus];
			[scratch compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
		[diffImage unlockFocus];
		
			// Generate the negative error image
		[scratch lockFocus];
			[negB compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
			[self compositeToPoint:NSZeroPoint operation:NSCompositePlusDarker];
		[scratch unlockFocus];
		
			// Add the negative error image to the result.
		[diffImage lockFocus];
			[scratch compositeToPoint:NSZeroPoint operation:NSCompositePlusLighter];
		[diffImage unlockFocus];
		
		return [diffImage autorelease];
		}
	return self;  // Difference from nil is self, by fiat.
	}

@end

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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
