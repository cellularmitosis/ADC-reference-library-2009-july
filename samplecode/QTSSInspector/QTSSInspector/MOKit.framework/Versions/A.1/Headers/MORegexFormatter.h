// MORegexFormatter.h
// MOKit
//
// Copyright © 1996-2001, Mike Ferris.
// All rights reserved.

// ABOUT MOKit
//
// MOKit is a collection of useful and general objects.  Permission is
// granted by the author to use MOKit in your own programs in any way
// you see fit.  All other rights to the kit are reserved by the author
// including the right to sell these objects as part of a LIBRARY or as
// SOURCE CODE.  In plain English, I wish to retain rights to these
// objects as objects, but allow the use of the objects as pieces in a
// fully functional program.  NO WARRANTY is expressed or implied.  The author
// will under no circumstances be held responsible for ANY consequences to
// you or others from the use of these objects.  Since you don't have to pay for
// them, and full source is provided, I think this is perfectly fair.

// ABOUT MORegexFormatter
//
// MORegexFormatter is, like, uh, really cool.

#import <Foundation/Foundation.h>
#import <MOKit/MOKitDefines.h>

@class MORegularExpression;

@interface MORegexFormatter : NSFormatter <NSCopying, NSCoding> {
  @private
    NSMutableArray *_expressions;
    NSString *_formatPattern;
    unsigned _lastMatchedExpressionIndex;
    struct __rfFlags {
        unsigned int allowsEmptyString:1;
        unsigned int RESERVED:31;
    } _rfFlags;
    void *_reserved;
}

// *** Initialization

- (id)initWithRegularExpressions:(NSArray *)expressions;
    // DI.

- (id)initWithRegularExpression:(MORegularExpression *)expression;

- (NSArray *)regularExpressions;
- (void)insertRegularExpression:(MORegularExpression *)expression atIndex:(unsigned)index;
- (void)addRegularExpression:(MORegularExpression *)expression;
- (void)removeRegularExpressionAtIndex:(unsigned)index;
- (void)replaceRegularExpressionAtIndex:(unsigned)index withRegularExpression:(MORegularExpression *)expression;

- (BOOL)allowsEmptyString;
- (void)setAllowsEmptyString:(BOOL)flag;

- (NSString *)formatPattern;
- (void)setFormatPattern:(NSString *)pattern;

- (NSString *)stringForObjectValue:(id)obj;
    // MORegexFormatter's implementation uses the description of the object.

- (BOOL)getObjectValue:(id *)obj forString:(NSString *)string errorDescription:(NSString **)error;
    // If string matches any of the formatter's MORegularExpressions, returns YES and sets the object value to a copy of the string.  Else returns NO and sets an appropriate error string.

- (MORegularExpression *)lastMatchedExpression;
    // This is particularly useful for getting at the subexprerssion matches in a subclass that will go on to parse the value into a more convenient class.

@end
