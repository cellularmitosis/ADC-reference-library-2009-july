/*

File: CookingWithFractionsTransformer.m

Abstract: NSValueTransformer subclass used to convert fractional to
decimal notation (and vice-versa) for displaying "amount" information.

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

#import "CookingWithFractionsTransformer.h"

static const double epsilon = (1.0 / 57600.0);
static const long epsilonReciprocal = (57600);


static void reduceFraction(long* fractionalResult, long* denominator);


static void reduceFraction(long* fractionalResult, long* denominator) {

    long f, d;
    BOOL recurse = NO;
    
    f = *fractionalResult;
    d = *denominator;

    while (((f % 2) == 0) && ((d % 2) == 0)) {
        f = (f / 2);
        d = (d / 2);
        recurse = YES;
    }

    while (((f % 3) == 0) && ((d % 3) == 0)) {
        f = (f / 3);
        d = (d / 3);
        recurse = YES;
    }

    while (((f % 5) == 0) && ((d % 5) == 0)) {
        f = (f / 5);
        d = (d / 5);
        recurse = YES;
    }

    if (recurse) {
        reduceFraction(&f, &d);
    }
    
    *fractionalResult = f;
    *denominator = d;
}

@implementation CookingWithFractionsTransformer


+ (void)initialize {

    if ( self == [CookingWithFractionsTransformer class] ) {

        [NSValueTransformer setValueTransformer:[[[CookingWithFractionsTransformer alloc] init] autorelease] forName:@"CookingWithFractions"];

        id transformer = [[CookingWithFractionsTransformer alloc] init];
        [transformer setUsesAttributedStrings:NO];
        [NSValueTransformer setValueTransformer:transformer forName:@"CookingWithFractionsPlain"];
        [transformer release];
    }
}


+ (BOOL)allowsReverseTransformation {
    return YES;
}


- (id)init {

    self = [super init];
    if (self != nil) {

        coreFormatter = [[NSNumberFormatter alloc] init];
        useAttributedStrings = YES;
    }

    return self;
}


- (void)dealloc {
    [coreFormatter release];
    [super dealloc];
}


-(BOOL)usesAttributedStrings {
    return useAttributedStrings;
}


-(void)setUsesAttributedStrings:(BOOL)state {
    useAttributedStrings = state;
}


- (id)transformedValue:(id)value {

    if (value == nil) {
        return [self usesAttributedStrings] ? [[[NSAttributedString alloc] initWithString:@"0"] autorelease] : @"0";
    }

    if (![value isKindOfClass:[NSNumber class]]) {
        return value;
    }

    double d = [(NSNumber*)value doubleValue];

    double fractionalIntermediate;
    long integerResult, fractionalResult, denominator;
    NSMutableString* intermResult = [[NSMutableString alloc] init];
    NSRange numeratorRange, denominatorRange;
    denominator = epsilonReciprocal;
    
    fractionalIntermediate = fmod(d, 1.0);
    integerResult = lrint(d - fractionalIntermediate);
    
    fractionalResult = lrint((fractionalIntermediate / epsilon));

    if (integerResult > 0) {
        [intermResult appendFormat:@"%d", integerResult];
    }

    if (fractionalResult > 0) {
        if (integerResult > 0) {
            [intermResult appendString:@" "];
        }
        
        reduceFraction(&fractionalResult, &denominator);
        
        numeratorRange.location = [intermResult length];
        [intermResult appendFormat:@"%d", fractionalResult];
        numeratorRange.length = [intermResult length] - numeratorRange.location;
        [intermResult appendString:@"/"];
        denominatorRange.location = [intermResult length];
        [intermResult appendFormat:@"%d", denominator];
        denominatorRange.length = [intermResult length] - denominatorRange.location;
    }
    
    if ([intermResult length] == 0) {
        [intermResult appendString:@"0"];
    }
    
    if ([self usesAttributedStrings]) {
        NSMutableAttributedString* temp = [[NSMutableAttributedString alloc] initWithString:intermResult];
        if (fractionalResult > 0) {
            [temp superscriptRange:numeratorRange];
            [temp subscriptRange:denominatorRange];
        }
        [intermResult release];
        intermResult = (NSMutableString*)temp;
    }
    
    return [intermResult autorelease];
}


- (id)reverseTransformedValue:(id)value {

    if (value == nil) {
        return [NSNumber numberWithDouble:0.0];
    }
    
    NSString* text;
    if ([value isKindOfClass:[NSAttributedString class]]) {
        text = [(NSAttributedString*)value string];
    } else if (![value isKindOfClass:[NSString class]]) {
        return value;
    } else {
        text = (NSString*)value;
    }
    
    if ([text length] == 0) {
        return [NSNumber numberWithDouble:0.0];
    }
    
    NSArray* pieces = [text componentsSeparatedByString:@"/"];
    double result = 0.0;
    if ([pieces count] == 1) {
        result = [[coreFormatter numberFromString:text] doubleValue];
    } else {
        NSString *integerText, *numeratorText, *denominatorText;
        int integerValue = 0;
        double numeratorValue = 0.0;
        double denominatorValue = 1.0;
        
        denominatorText = (NSString*)[pieces objectAtIndex:1];
        if ([denominatorText length] > 0) {
            denominatorValue = [[coreFormatter numberFromString:denominatorText] doubleValue];
        }
        
        NSArray* integerAndNumerator = [[(NSString*)[pieces objectAtIndex:0] 
                                        stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] 
                                            componentsSeparatedByString:@" "];
        if ([integerAndNumerator count] > 1) {
            // grab integer part
            integerText = [integerAndNumerator objectAtIndex:0];
            numeratorText = [integerAndNumerator objectAtIndex:1];
        } else {
            integerText = nil;
            numeratorText = [integerAndNumerator objectAtIndex:0];
        }

        if ([integerText length] > 0) {
            integerValue = [[coreFormatter numberFromString:integerText] intValue];
        }
        if ([numeratorText length] > 0) {
            numeratorValue = [[coreFormatter numberFromString:numeratorText] doubleValue];
        }
    
        result = (double)integerValue + (numeratorValue / denominatorValue);
    }

    double integerResult, fractionalResult;
    
    fractionalResult = fmod(result, 1.0);
    integerResult = round(result - fractionalResult);
    fractionalResult = round((fractionalResult / epsilon)) * epsilon;
    
    return [NSNumber numberWithDouble:(integerResult + fractionalResult)];
}


@end
