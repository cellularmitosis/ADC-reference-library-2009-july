// MOCompletionManager.h
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

// ABOUT MOCompletionManager
//
// MOCompletionManager provides a small framework for implementing 
// various types of escape completion.  The delegate of an NSTextView 
// or editable NSControl can use an instance of MOCompletionManager 
// to handle escape completion merely by calling a couple of methods
// at appropriate times.
//
// To use MOCompletionManager just alloc/init one.  Usually one 
// instance will serve a single NSTextView or editable NSControl.  
// You will need to create an array of completion strategies and 
// pass it to setCompletionStrategies:.  MOKit comes with one 
// strategy: MOFilenameCompletionStrategy.  You can create others 
// by subclass MOCompletionStrategy.  If MOCompletionManager is 
// the delegate of an NSTextView, then everything will just work, 
// but usually this is not possible because you have a controller 
// object that needs to be delegate.  Usually the controller object 
// "has-a" MOCompletionManager.  In this case, you just need to 
// implement a couple of the delegate messages to call into the 
// MOCompletionManager.
//
//         textDidChange:
//         textDidEndEditing:
//         textView:doCommandBySelector:
//
// For the first two methods, you should call dumpCompletionState in 
// the MOCompletionManager.  For the last one, you should call 
// doCompletionInTextView:startLimit:basePath: if the command 
// selector is @selector(complete:).  For controls, of course, the
// control variants of these methods would be used.

#import <AppKit/AppKit.h>
#import <MOKit/MOKitDefines.h>

@class MOCompletionStrategy;

@interface MOCompletionManager : NSObject {
    @private
    NSTextView *_cachedTextView;
    NSRange _cachedSelectedRange;
    NSString *_cachedBasePath;
    NSMutableArray *_completionStrategies;
    MOCompletionStrategy *_completionStrategy;
    NSArray *_completionMatches;
    unsigned _lastMatchIndex;
    NSRange _completionRange;
    NSString *_completionPrefixString;
    BOOL _dumpCompletionsEnabled;
}

- (void)setCompletionStrategies:(NSArray *)strategies;
- (NSArray *)completionStrategies;

- (void)dumpCompletionState;
- (void)doCompletionInTextView:(NSTextView *)textView startLimit:(unsigned)startLimit basePath:(NSString *)basePath;

@end
