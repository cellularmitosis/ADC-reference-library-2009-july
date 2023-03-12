// MOFoundationExtras.h
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

// ABOUT MOFoundationExtras
//
// This is a collection of categories on Foundation objects.  It has become a 
// time honored tradition for any framework to have categories which add 
// various useful non-primitive methods to Foundation.  Well, who is MOKit 
// to buck this tradition?  Of course, for maximum utility, such non-primitive 
// methods should always be implemented in terms of the primitives for the 
// class.  Especially if it is a class cluster.  (This is generally the 
// natural way to implement them anyway.)

#import <Foundation/Foundation.h>
#import <MOKit/MOKitDefines.h>

@interface NSString (MOFoundationExtras)

- (NSString *)MO_stringByReplacingBackslashWithSlash;

@end

@interface NSMutableString (MOFoundationExtras)

- (void)MO_standardizeEndOfLineToLF;
- (void)MO_standardizeEndOfLineToCRLF;
- (void)MO_standardizeEndOfLineToCR;
- (void)MO_standardizeEndOfLineToParagraphSeparator;
- (void)MO_standardizeEndOfLineToLineSeparator;

@end

@interface NSArray (MOFoundationExtras)

- (NSString *)MO_longestCommonPrefixForStrings;

@end
