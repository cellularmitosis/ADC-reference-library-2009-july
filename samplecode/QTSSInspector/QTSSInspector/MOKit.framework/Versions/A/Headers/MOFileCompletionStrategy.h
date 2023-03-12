// MOFileCompletionStrategy.h
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

// ABOUT MOCompletionStrategy
//
// This completion strategy does normal file/path completion.

#import <MOKit/MOKitDefines.h>
#import <MOKit/MOCompletionStrategy.h>

@interface MOFileCompletionStrategy : MOCompletionStrategy {
    @private
    struct __fcsFlags {
        unsigned int appendsSpaceOnFileMatch:1;
        unsigned int appendsSlashOnDirectoryMatch:1;
        unsigned int _reserved:30;
    } _fcsFlags;
}

- (BOOL)appendsSpaceOnFileMatch;
- (void)setAppendsSpaceOnFileMatch:(BOOL)flag;
    // If the completion yields a single, non-ambiguous file match and this is turned on, then a space character will be appended to the match.  This is off by default because it is unsuitable if the result is to be used as a simple path.

- (BOOL)appendsSlashOnDirectoryMatch;
- (void)setAppendsSlashOnDirectoryMatch:(BOOL)flag;
    // If the completion yields a single, non-ambiguous directory match and this is turned on, then a slash character will be appended to the match (always a forward slash, run the path through stringByStandardizingPath if you want backslash on windows).  This is on by default since the presence of the slash doesn't confuse anything I'm aware of.


@end
