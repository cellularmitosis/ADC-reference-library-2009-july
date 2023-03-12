/*

File: Controller.h

Abstract: Handles initialization of the ImageView as well as communication
	between other controls and the ImageView's Image

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Inc. ("Apple") in consideration of your agreement to the
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

Copyright © 2007 Apple Inc., All Rights Reserved

*/

#import <Cocoa/Cocoa.h>
#import "ImageUtils.h"

@class ImageView;

@interface Controller : NSObject
{
	IBOutlet ImageView *imageView;
	IBOutlet NSStepper *scaleYView;
	IBOutlet NSTextField * textScaleYView;
	IBOutlet NSTextField * textLabelXView;
	IBOutlet NSTextField * textLabelYView;
	float rotation;
	float scaleX;
	float scaleY;
	float translateX;
	float translateY;
	BOOL preserveAspectRatio;
	NSMutableArray * openImageIOSupportedTypes;
}

-(IBAction)changeScaleX:(id)sender;
-(IBAction)changeScaleY:(id)sender;
-(IBAction)changeTranslateX:(id)sender;
-(IBAction)changeTranslateY:(id)sender;
-(IBAction)reset:(id)sender;
-(IBAction)openDocument:(id)sender;
-(IBAction)saveDocumentAs:(id)sender;
-(void)openImageDidEnd:(NSOpenPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo;
-(void)saveImageDidEnd:(NSSavePanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo;

-(void)setRotation:(float)r;
-(void)setScaleX:(float)x;
-(void)setScaleY:(float)y;
-(void)setScaleX:(float)x andY:(float)y;
-(void)setPreserveAspectRatio:(BOOL)preserve;
-(void)setTranslateX:(float)x;
-(void)setTranslateY:(float)y;
-(void)setTranslateX:(float)x andY:(float)y;
-(void)resetTransformations;
-(float)rotation;
-(float)scaleX;
-(float)scaleY;
-(BOOL)preserveAspectRatio;
-(float)translateX;
-(float)translateY;
@end
