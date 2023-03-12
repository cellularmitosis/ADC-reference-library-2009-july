/*

File: ContentFormatting.m

Abstract: Use the PubSub framework to create a simple RSS news reader.

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved.

*/

#import "ContentFormatting.h"
#import <PubSub/PSContent.h>


// Utility to quickly grab a localized string out of the application's main bundle
static NSString* LOC( NSString *key )
{
    return [[NSBundle mainBundle] localizedStringForKey: key value:@"" table:nil];
}


// Turns an NSDate into a string with locale-appropriate formatting (names of date and time elements, the
// order of their appearance in the string, the separators used, etc.). Relies upon the appropriate localizations
// existing in the application's main bundle.
NSString* ShortDateString( NSDate* date, BOOL showTime )
{
    if( ! date )
        return nil;
    NSCalendarDate *calDate = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate: [date timeIntervalSinceReferenceDate]];
    NSCalendarDate *today = [NSCalendarDate date];
    int days = [today dayOfCommonEra] - [calDate dayOfCommonEra];
    NSString *fmtName;
    if( days == 0 )
        fmtName = @"TodayFmt";
    else if( days == 1 )
        fmtName = @"YesterdayFmt";
    else if( days > 1 && days < 7 )
        fmtName = @"ThisWeekFmt";                  // weekday only
    else if( days < 365/2 || [calDate yearOfCommonEra] == [today yearOfCommonEra] )
        fmtName = @"ThisYearFmt";
    else
        fmtName = @"PastYearFmt";
    NSString *format = LOC(fmtName);
    if( showTime )
        format = [NSString stringWithFormat: @"%@%@%@",
                    format, LOC(@"DateTimeSep"), LOC(@"TimeFmt")];
    return [calDate descriptionWithCalendarFormat: format
                                           locale: [[NSUserDefaults standardUserDefaults] dictionaryRepresentation]];
}


// See note for ShortDateString(), above.
NSString* ShortDateTimeString( NSDate* date )
{
    return ShortDateString(date,YES);
}


// Returns an array of two objects: the ShortDateString(date, NO), and a 12-hour time string
NSArray *ShortDateAndTimeStrings( NSDate *date )
{
    NSCalendarDate *calDate = [NSCalendarDate dateWithTimeIntervalSinceReferenceDate: [date timeIntervalSinceReferenceDate]];
    NSString *timeStr = [calDate descriptionWithCalendarFormat: LOC(@"TimeFmt")
                                                        locale: [[NSUserDefaults standardUserDefaults] dictionaryRepresentation]];
    // We're always displaying 12-hour time, and so removing the leading 0 from the hour.
    if( [timeStr hasPrefix: @"0"] )
        timeStr = [timeStr substringFromIndex: 1];
    return [NSArray arrayWithObjects: ShortDateString(date,NO),timeStr, nil];
}


// Value transformer for an NSDate that always transforms it to a ShortDateTimeString (see above).
@implementation ShortDateTimeTransformer

+ (void) initialize
{
    [NSValueTransformer setValueTransformer: [[self alloc] init] 
                                    forName: @"ShortDateTimeTransformer"];
}

- (id)transformedValue:(id)value
{
    return ShortDateTimeString(value);
}

@end


// Value transformer for an NSError that always transforms it to a localized description (string)
@implementation ErrorTransformer

+ (void) initialize
{
    [NSValueTransformer setValueTransformer: [[self alloc] init] 
                                    forName: @"ErrorTransformer"];
}

- (id)transformedValue:(NSError*)value
{
    return [value localizedDescription];
}

@end


// Value transformer for an NSArray that transforms it into an NSArray of its objects' descriptions, comma-delimited
@implementation ArrayTransformer

+ (void) initialize
{
    [NSValueTransformer setValueTransformer: [[self alloc] init] 
                                    forName: @"ArrayTransformer"];
}

- (id)transformedValue:(NSArray*)array
{
    if( ! array )
        return nil;
    NSMutableArray *descs = [NSMutableArray array];
    for( unsigned i=0; i<[array count]; i++ )
        [descs addObject: [[array objectAtIndex: i] description]];
    return [descs componentsJoinedByString: @", "];
}

@end


// Value transformer for a PSContent that transforms it to a string appropriate to the MIMEType of the original object
@implementation PSContentTransformer

+ (void) initialize
{
    [NSValueTransformer setValueTransformer: [[self alloc] init] 
                                    forName: @"PSContentTransformer"];
}

- (id)transformedValue:(PSContent*)content
{
    if( ! content )
        return nil;
    NSString *type = [[content MIMEType] lowercaseString];
    if( [type hasPrefix: @"text/html"] || [type hasPrefix: @"application/xhtml+xml"] )
        return [content HTMLString];
    else if( [type hasPrefix: @"text/"] )
        return [content plainTextString];
    else
        return [NSString stringWithFormat: @"{%@}",type];
}

@end


@implementation PSEntry (NewsReader)

- (NSString*) nr_GUID
{
    NSXMLElement *xml = self.XMLRepresentation;
    NSArray *guids = [xml elementsForName: @"id"];          // Atom
    if( [guids count] > 0 )
        return [[guids objectAtIndex: 0] stringValue];
    guids = [xml elementsForName: @"guid"];                 // RSS
    if( [guids count] > 0 )
        return [[guids objectAtIndex: 0] stringValue];
    return nil;
}

@end
