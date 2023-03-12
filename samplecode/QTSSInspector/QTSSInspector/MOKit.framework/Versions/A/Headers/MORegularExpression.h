// MORegularExpression.h
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

// ABOUT MORegularExpression
//
// MORegularExpression objects have a pattern string in unix-style regular
// expression syntax and know how match other strings against them.  They
// are immutable.  If you need to match another string, make another
// MORegularExpression.  The implementation is almost entirely provided by
// Henry Spencer's regular expression package which is used by this framework
// in a modified form and also included in its original and complete form.
// This class is NOT unicode aware.  Expressions and candidate strings should
// be ascii encodable.  This lack of internationalizability stems from the
// limitations of the underlying expression parsing code used.

#import <Foundation/Foundation.h>
#import <MOKit/MOKitDefines.h>

#define MO_REGEXP_MAX_SUBEXPRESSIONS 20

@interface MORegularExpression : NSObject <NSCopying, NSCoding> {
  @private
    NSString *_expressionString;
    NSString *_lastMatch;
    NSRange _lastSubexpressionRanges[MO_REGEXP_MAX_SUBEXPRESSIONS];
    void *_compiledExpression;
    BOOL _ignoreCase;
}

+ (BOOL)validExpressionString:(NSString *)expressionString;

+ (id)regularExpressionWithString:(NSString *)expressionString ignoreCase:(BOOL)ignoreCaseFlag;
+ (id)regularExpressionWithString:(NSString *)expressionString;

- (id)initWithExpressionString:(NSString *)expressionString ignoreCase:(BOOL)ignoreCaseFlag;
    // Designated Initializer.
    
- (id)initWithExpressionString:(NSString *)expressionString;
    // Calls initWithExpressionString:expressionString ignoreCase:NO.

- (NSString *)expressionString;

- (BOOL)matchesString:(NSString *)candidate;

- (NSRange)rangeForSubexpressionAtIndex:(unsigned)index inString:(NSString *)candidate;
    // Range of subexpression match.  Returns {NSNotFound, 0} if there was no subexpression for that index.
- (NSString *)substringForSubexpressionAtIndex:(unsigned)index inString:(NSString *)candidate;
    // Subexpression match substring.  Returns nil if there was no subexpression for that index.  Returns @"" if there was a subexpression at that index and it matched the empty string (ie if the range was zero-length).

- (NSArray *)subexpressionsForString:(NSString *)candidate;
    // This method is mostly for compatibility with prior versions.  The previous two methods are recommended instead.

@end
