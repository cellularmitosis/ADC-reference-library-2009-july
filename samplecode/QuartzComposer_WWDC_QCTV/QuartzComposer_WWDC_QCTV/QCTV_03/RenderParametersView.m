/*

File: RenderParametersView.m

Abstract: Implements the RenderParametersView class.

Version: 1.0

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/

#import "RenderParametersView.h"
#import "CompositionParametersView.h"

#define kHMargin				10
#define kVMargin				10
#define kHExtra					20
#define kVExtra					20

@implementation RenderParametersView

- (BOOL) isFlipped
{
	return YES;
}

- (id) initWithFrame:(NSRect)frameRect
{
	//Allocate the QCRenderer list
	if(self = [super initWithFrame:frameRect])
	_renderers = [NSMutableArray new];
	
	return self;
}

- (void) dealloc
{
	//Destroy QCRenderer list
	[_renderers release];
	
	[super dealloc];
}

- (void) _arrangeSubviews
{
	NSArray*						subviews = [self subviews];
	NSView*							view;
	unsigned						i;
	NSSize							size;
	
	//Set subviews origins and compute total size 
	_bestSize = NSZeroSize;
	for(i = 0; i < [subviews count]; ++i) {
		view = [subviews objectAtIndex:i];
		size = [view frame].size;
		if(size.width > _bestSize.width)
		_bestSize.width = size.width;
		[view setFrameOrigin:NSMakePoint(kHMargin, kVMargin + _bestSize.height)];
		_bestSize.height += size.height;
	}
	
	//Match subviews widths
	for(i = 0; i < [subviews count]; ++i) {
		view = [subviews objectAtIndex:i];
		[view setFrameSize:NSMakeSize(_bestSize.width, [view frame].size.height)];
	}
	
	//Add horizontal and vertical margins to total size
	if(_bestSize.width > 0)
	_bestSize.width += 2 * kHMargin;
	if(_bestSize.height > 0)
	_bestSize.height += 2 * kVMargin;
}

- (NSSize) bestSize
{
	return _bestSize;
}

- (void) addRenderer:(QCRenderer*)renderer title:(NSString*)title
{
	CompositionParametersView*		view;
	NSBox*							box;
	NSSize							size;
	
	//Make sure the QCRenderer is valid and not already in the list
	if(!renderer || ![title length] || [_renderers containsObject:renderer])
	return;
	
	//Create CompositionParametersView for QCRenderer, wrap it in an NSBox and add it as a subview
	view = [[CompositionParametersView alloc] initWithRenderer:renderer];
	[view setAutoresizingMask:NSViewWidthSizable];
	size = [view minimumSize];
	box = [[NSBox alloc] initWithFrame:NSMakeRect(0, 0, size.width + kHExtra, size.height + kVExtra)];
	[box setTitle:title];
	//[box setAutoresizingMask:NSViewWidthSizable];
	[box setContentView:view];
	[self addSubview:box];
	[box release];
	[view release];
	
	//Update QCRenderer list
	[_renderers addObject:renderer];
	
	//Rearrage all subviews
	[self _arrangeSubviews];
}

- (void) removeRenderer:(QCRenderer*)renderer
{
	unsigned						index = [_renderers indexOfObject:renderer];
	
	//Make sure QCRenderer is in the list
	if(index == NSNotFound)
	return;
	
	//Remove related subview
	[(NSView*)[[self subviews] objectAtIndex:index] removeFromSuperview];
	
	//Update QCRenderer list
	[_renderers removeObjectAtIndex:index];
	
	//Rearrage all subviews
	[self _arrangeSubviews];
}

- (void) removeAllRenderers
{
	//Remove renderers from list and remove their related subviews
	while([_renderers count]) {
		[(NSView*)[[self subviews] objectAtIndex:0] removeFromSuperview];
		[_renderers removeObjectAtIndex:0];
	}
	 
	//Rearrange all subviews
	[self _arrangeSubviews];
}

- (NSArray*) renderers
{
	return _renderers; //FIXME: Return a copy instead?
}

- (NSDictionary*) parameters:(BOOL)plistCompatible
{
	NSMutableDictionary*			dictionary = [NSMutableDictionary new];
	NSArray*						subviews = [self subviews];
	NSDictionary*					subDictionary;
	unsigned						i;
	NSBox*							box;
	
	//Iterate through QCRenderers
	for(i = 0; i < [subviews count]; ++i) {
		//Retrieve QCRenderer input parameters from its CompositionParametersView
		box = [subviews objectAtIndex:i];
		subDictionary = [(CompositionParametersView*)[box contentView] parameters:plistCompatible];
		
		//Add parameters to dictionary
		if([subDictionary count])
		[dictionary setObject:subDictionary forKey:[box title]];
	}
	
	return [dictionary autorelease];
}

- (void) setParameters:(NSDictionary*)parameters
{
	NSArray*						subviews = [self subviews];
	NSDictionary*					dictionary;
	unsigned						i;
	NSBox*							box;
	
	//Iterate through QCRenderers
	for(i = 0; i < [subviews count]; ++i) {
		//Retrieve parameters from dictionary
		box = [subviews objectAtIndex:i];
		dictionary = [parameters objectForKey:[box title]];
		
		//Set QCRenderer input parameters through its CompositionParametersView
		if([dictionary count])
		[(RenderParametersView*)[box contentView] setParameters:dictionary];
	}
}

@end
