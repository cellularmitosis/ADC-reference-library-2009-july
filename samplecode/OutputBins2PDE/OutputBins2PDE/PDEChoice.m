/*

File: PDEChoice.m

Abstract: The PDEChoice class is used to manage user preferences and
	ease translation of those preferences between internal and user
	representations.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import "PDEChoice.h"
#import "PDELogging.h"
#import "Utilities.h"

@implementation PDEChoice

static NSStringEncoding ppdEncoding = 0;

+(void)initialize
{
	if(RunningOn105OrLater()) {
		// On 10.5 or later CUPS always uses UTF-8 encoding,
		// so all localized text from a PPD will be in this encoding.
		ppdEncoding = NSUTF8StringEncoding;
	} else {
		// On 10.4 the encoding that CUPS uses is the same as
		// the underlying PPD file. The easiest thing that you
		// can do is to encode your PPDs as UTF-8 on 10.4,
		// and that is what is assumed here, but if that is
		// not possible then you should use an appropriate
		// encoding for your PPDs (your logic may need to be
		// more complicated than this).
		// Bottom line - encode your PPDs as UTF-8 to avoid this issue.
		ppdEncoding = NSUTF8StringEncoding;
	}
	FLog(@"Encoding set as %i", ppdEncoding);
}

+(id)choiceWithChoice:(ppd_choice_t*)c
{
	return [[[PDEChoice alloc] initWithChoice:c] autorelease];
}

-(id)initWithChoice:(ppd_choice_t*)c
{
	if((self = [super init]) != nil) {
		userTitle = nil;
		ppdTitle = nil;
		choice = c;
		if(choice->text == NULL) {
			PDELog(@"choice->text is NULL!");
		}
		if(choice->choice == NULL) {
			PDELog(@"choice->choice is NULL!");
		}
	}
	return self;
}

-(NSString*)userTitle
{
	if(userTitle == nil) {
		// Localized text uses either UTF-8 on 10.5+ or the file's encoding on 10.4.
		// See +initialize for more details.
		userTitle = [[NSString alloc] initWithCString:choice->text encoding:ppdEncoding];
		if(userTitle == nil) {
			PDELog(@"could not create userTitle!");
		}
	}
	return userTitle;
}

-(NSString*)ppdTitle
{
	if(ppdTitle == nil) {
		// The title string, which is not user-visible, must be pure ASCII.
		ppdTitle = [[NSString alloc] initWithCString:choice->choice encoding:NSASCIIStringEncoding];
		if(ppdTitle == nil) {
			PDELog(@"could not create ppdTitle!");
		}
	}
	return ppdTitle;
}

-(ppd_choice_t*)choice
{
	return choice;
}

-(BOOL)isDefaultChoice
{
	return strncmp(choice->choice, choice->option->defchoice, PPD_MAX_NAME) == 0;
}

-(NSString*)description {
	return [NSString stringWithFormat:@"<%@ %p userTitle:\"%@\" ppdTitle:\"%@\" marked:%s choice:%i of %i>", [[self class] className], self, [self userTitle], [self ppdTitle], choice->marked ? "YES" : "NO", (choice - choice->option->choices) + 1, choice->option->num_choices];
}

- (void) dealloc
{
	[userTitle release];
	[ppdTitle release];
	[super dealloc];
}

@end
