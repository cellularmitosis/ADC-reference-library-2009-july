/*

File: OutputBins2PDEPlugin.m

Abstract: This class determines which PDEs are created, creates them,
	exports them, and tracks their lifetimes.

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

#import <Print/PMPrintingDialogExtensions.h>
#import "OutputBins2PDEPlugin.h"
#import "OutputBins2PDE.h"
#import "PasswordPDE.h"
#import "Utilities.h"
#import "PDELogging.h"

@implementation OutputBins2PDEPlugIn

- (BOOL)initWithBundle:(NSBundle*)theBundle
{
	OSStatus err;
	// the log macro is a no-op on release builds
	// so we mark 'err' unused so we avoid the warning
	#pragma unused(err)

	PDEProlog(@"initWithBundle:(%@, %p)", theBundle, theBundle);
	pluginBundle = theBundle;
	
	// Register our Help Book
	FSRef fsref;
	NSString * path = [pluginBundle bundlePath];
	NSURL * url = [NSURL fileURLWithPath:path];
	CFURLGetFSRef((CFURLRef)url, &fsref);
	err = AHRegisterHelpBook(&fsref);
	PDELog(@"Result %i registering helpbook at '%@'", err, path);
	
	PDEEpilog(@"initWithBundle:=BOOL(YES)");
	return YES;
}

- (NSArray*)PDEPanelsForType:(NSString*)pdeType withHostInfo:(id)host
{
	PDEProlog(@"PDEPanelsForType:(%@) withHostInfo:(%@)", pdeType, host);
	PDEPluginCallback* callback = (PDEPluginCallback*)host;

	PDEs = [[NSMutableArray alloc] initWithCapacity:2];
	if ([pdeType isEqual:(NSString*)kPrinterModuleTypeIDStr])
	{
		OutputBins2PDE* outputBinsPDE = [[[OutputBins2PDE alloc] initWithCallback:callback andBundle:pluginBundle] autorelease];
		PDELog(@"Creating PDE of type kPrinterModuleTypeIDStr(%@) OutputBins2PDE (%p)", kPrinterModuleTypeIDStr, outputBinsPDE);
		if(outputBinsPDE != nil)
		{
			[PDEs addObject:outputBinsPDE];
		}
		
		PasswordPDE* passwordPDE = [[[PasswordPDE alloc] initWithCallback:callback andBundle:pluginBundle] autorelease];
		PDELog(@"Creating PDE of type kPrinterModuleTypeIDStr(%@) PasswordPDE (%p)", kPrinterModuleTypeIDStr, passwordPDE);
		if(passwordPDE != nil)
		{
			[PDEs addObject:passwordPDE];
		}
		
		// If it turns out that we have no PDEs to return, then they can just go away now.
		if([PDEs count] == 0)
		{
			[PDEs release];
			PDEs = nil;
		}
	}
	// On 10.4 -dealloc will never be called for this class, so we need to clean up the PDEs array before then, thus we autorelease it now.
	// However, on 10.5, if we try to release it now, then the host application will crash.
	// For the best of both worlds, we autorelease here if we are not on 10.5 or later, and we'll release in -dealloc
	// if we are on 10.5 or later (by virtue of it being called).
	if(!RunningOn105OrLater())
	{
		[PDEs autorelease];
	}

	PDEEpilog(@"PDEPanelsForType:withHostInfo:=NSArray(%@)", PDEs);
	return PDEs;
}

// Due to a bug in Mac OS X 10.4, -dealloc is never called on objects of this class. Thus we have been careful to
// allocate/retain as little as possible for the class itself. Since it will cause a leak under 10.4, you should
// be careful to clean up as much as possible without relying on -dealloc on 10.4.
-(void)dealloc
{
	[PDEs release];
	
	[super dealloc];
}

@end
