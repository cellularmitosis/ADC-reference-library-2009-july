
#include <stdlib.h>
#include <stdarg.h>
#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

// Developers writing their own custom data formatters should be sure to read the information in DataFormatterPlugin.h
#import "/Developer/Applications/Xcode.app/Contents/PlugIns/GDBMIDebugging.xcplugin/Contents/Headers/DataFormatterPlugin.h"

_pbxgdb_plugin_function_list *_pbxgdb_plugin_functions = NULL;

static char *nullPluginFunctions = "ManagedObject Data Formatter error: _pbxgdb_plugin_functions not set!";

char *SampleCode_ManagedObjectSummary(NSManagedObject *managedObject, int identifier)
{
    char *result;
    if (_pbxgdb_plugin_functions) {
        NSAutoreleasePool *autoreleasePool = [[NSAutoreleasePool alloc] init];
        
        NSEntityDescription *entity = [managedObject entity];
        NSString *entityName = [entity name];
        NSArray *properties = [entity properties];
        NSString *faultString = [managedObject isFault] ? @", is fault" : @"";
        
        NSString *summaryString = [NSString stringWithFormat:@"Entity: %@, %u properties%@.", entityName, [properties count], faultString];
        result = _pbxgdb_plugin_functions->message(identifier, [summaryString UTF8String]);

        [autoreleasePool release];
    } else {
        result = nullPluginFunctions;
    }
    return result;
}

/*

File: ManagedObjectDataFormatter.m

Abstract: Custom data formatter that displays interesting information about a NSManagedObject

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