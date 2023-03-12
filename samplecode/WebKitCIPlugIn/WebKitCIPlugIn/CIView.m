/*

File: CIView.m

Abstract:   Implemntation of the CoreImage WebKit Plugin.

Version: 1.1

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/ 

#import "CIView.h"

#import <WebKit/WebKit.h>

@implementation CIView

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
    CIView *ciView = [[[self alloc] init] autorelease];
    [ciView setArguments:arguments];
    return ciView;
}

- (void)dealloc
{   
    [_arguments release];
    [_theImage release];
    [super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
    [arguments copy];
    [_arguments release];
    _arguments = arguments;
}

- (void)webPlugInInitialize
{
    gammaValue = 0.75;
    if(!_currentFilter)
    {
	_currentFilter = [[CIFilter filterWithName:@"CIGammaAdjust"] retain];
	[_currentFilter setDefaults];
    }
}

- (void)loadImage
{
    if (imageURL != nil && [imageURL length] != 0) 
    {
	NSURL *URL = [NSURL URLWithString:imageURL];
	NSData	*theData = [NSData dataWithContentsOfURL:URL];
	[_theImage release];
	_theImage = [[CIImage imageWithData:theData] retain];
	[_currentFilter setValue:_theImage forKey:@"inputImage"];
    }
}

- (void)setupImage
{
    if(!_theImage)
	[self loadImage];
    if (_theImage) 
    {
	[_currentFilter setValue:[NSNumber numberWithFloat:gammaValue] forKey:@"inputPower"];
	[self setImage:[_currentFilter valueForKey:@"outputImage"]];
    }
}

- (void)webPlugInStart
{
    if (!_theImage) {
	NSDictionary *webPluginAttributesObj = [_arguments objectForKey:WebPlugInAttributesKey];
	imageURL = [webPluginAttributesObj objectForKey:@"src"];
	[_currentFilter setValue:_theImage forKey:@"inputImage"];
	[self setupImage];
    }
    
}

- (void)webPlugInStop
{
}

- (void)webPlugInDestroy
{
}

- (void)webPlugInSetIsSelected:(BOOL)isSelected
{
}

// Scripting support

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
    if (selector == @selector(setGamma) || selector == @selector(setImage)) {
        return NO;
    }
    return YES;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)property
{
    if (strcmp(property, "imageURL") == 0) {
        return NO;
    }
    if (strcmp(property, "gammaValue") == 0) {
        return NO;
    }
    return YES;
}

- (id)objectForWebScript
{
    return self;
}

- (void)setGamma
{
    [self setupImage];
}

- (void)setImage
{
    [self loadImage];
    [self setupImage];
}

@end
