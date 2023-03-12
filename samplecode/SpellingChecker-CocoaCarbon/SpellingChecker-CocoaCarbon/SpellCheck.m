/*
	File:		SpellCheck.m
	
	Contains:	Contains wrappers to cocoa objective-c routines and exports the wrapper
                functions as C.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2003 - 2004 Apple Computer, Inc., All Rights Reserved
*/
#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include "SpellCheck.h"

#define	DebugSpellCheck	0


/*	
	NSApplicationLoad() is an API introduced in 10.2 which is a startup function to call when
	running Cocoa code from a Carbon application.
*/

void	InitializeCocoa()
{		
	(void) NSApplicationLoad();
	return;
}

/*
	Returns a guaranteed unique tag to use as the spell document tag for a document.
	You should use this method to generate tags, if possible, to avoid collisions
	with other objects that can be spell checked.
*/
int	UniqueSpellDocumentTag()
{
    int					tag;
    
	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];
    
    tag		=[NSSpellChecker uniqueSpellDocumentTag];
    
    [pool release];
    
    return( tag );
}

/*	
	When a document closes, it should notify the NSSpellChecker via closeSpellDocumentWithTag:
	so that its ignored word list gets cleaned up.
*/
void	CloseSpellDocumentWithTag( int tag )
{
	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];
    
    [[NSSpellChecker sharedSpellChecker] closeSpellDocumentWithTag:tag];
    
    [pool release];
}


/*
	Initiates a spell-check of a string.  Returns the range of the first misspelled
	word (and optionally the wordCount by reference).
*/
CFRange	CheckSpellingOfString( CFStringRef stringToCheck, int startingOffset )
{
    NSRange		range	= {0,0};

	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];

    range		=[[NSSpellChecker sharedSpellChecker] checkSpellingOfString:(NSString *)stringToCheck startingAt:startingOffset];

#if( DebugSpellCheck )    
    NSLog( @"%u is the location", range.location );
    NSLog( @"%u is the length", range.length );
#endif    

    [pool release];

    return( *(CFRange*)&range );
}

/*	
	Initiates a spell-check of a string.  Returns the range of the first misspelled
	word (and optionally the wordCount by reference).
*/
CFRange	CheckSpellingOfStringWithOptions(	CFStringRef stringToCheck,	// the string whose spelling we wish to check
											int startingOffset,			// 0-based offset in stringToCheck at which checking starts
											CFStringRef language,		// pass an empty string CFSTR("") to use the default language
											Boolean wrapFlag,			// if true, checking will wrap round to the start of stringToCheck
																		// if false, checdking will stop at the end of stringToCheck
											int tag,					// tag indicating a list of words to be ignored
																		// use UniqueSpellDocumentTag to get a new tag
											int wordCount 
										)
{
    NSRange		range	= {0,0};

	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];

	range		=[[NSSpellChecker sharedSpellChecker] checkSpellingOfString:(NSString *)stringToCheck startingAt:startingOffset language:(NSString *)language wrap:(BOOL)wrapFlag inSpellDocumentWithTag:tag wordCount:&wordCount];

    [pool release];

    return( *(CFRange*)&range );	//	If there are no misspellings then it will get a length of zero
}

/*
	Add inWord to the list of words to be ignored when using CheckSpellingOfString with tag
	This method should be called from the client's implementation of -ignoreSpelling:
*/
void	IgnoreWord( CFStringRef wordToIgnore, int tag )
{
	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];

    [[NSSpellChecker sharedSpellChecker] ignoreWord:(NSString *)wordToIgnore inSpellDocumentWithTag:tag];

    [pool release];
}

/*
	Set the list of ignored words for tag, replacing any existing list
*/
void	SetIgnoredWords( CFArrayRef inWords, int tag )
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    [[NSSpellChecker sharedSpellChecker] setIgnoredWords:(NSArray*)inWords inSpellDocumentWithTag:tag];
    
    [pool release];
}

/*
	Get the list of ignored words for tag
	Caller must release the result with CFRelease
*/
CFArrayRef	CopyIgnoredWordsInSpellDocumentWithTag( int tag )
{
    NSArray				*array;

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init]; // to keep memory requirements down
    
    array	= [[[NSSpellChecker sharedSpellChecker] ignoredWordsInSpellDocumentWithTag:tag] retain];
    
    [pool release];
    
    return( (CFArrayRef)array );
}

/*
	Get a list of guesses for a misspelled words
	Caller must release the result with CFRelease
*/
CFArrayRef	GuessesForWord( CFStringRef word )
{
    NSArray				*array;

	NSAutoreleasePool* pool	= [[NSAutoreleasePool alloc] init];

    array	= [[[NSSpellChecker sharedSpellChecker] guessesForWord:(NSString *)word] retain];

    [pool release];

    return( (CFArrayRef)array );
}
