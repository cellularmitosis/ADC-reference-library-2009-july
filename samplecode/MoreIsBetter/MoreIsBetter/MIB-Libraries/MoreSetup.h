/*
	File:		MoreSetup.h

	Contains:	Sets up conditions etc for MoreIsBetter.

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

	Change History (most recent first):

$Log: MoreSetup.h,v $
Revision 1.14  2002/11/08 22:43:02         
A bunch of key changes. We no longer include <Carbon/Carbon.h> in the Mach-O builds, which means that individual MoreIsBetter modules can decide what frameworks they're dependent on. Also changed MoreIsBetter to use the standard C assert mechanism.

Revision 1.13  2001/11/07 15:55:15         
Tidy up headers, add CVS logs, update copyright.


        <12>     21/9/01    Quinn   Fix CF framework includes workaround.
        <11>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <10>     15/2/01    Quinn   MoreIsBetter now requires UI 3.4 (pre-release) or later.
         <9>      8/2/01    Quinn   Modern versions of MIB require UI 3.3.2 or higher.
         <8>    22/11/00    Quinn   Switch to "MacErrors.h".
         <7>     22/4/99    Quinn   Add a check for the Universal Interfaces version.  MoreIsBetter
                                    requires Universal Interfaces 3.2 or higher (because many of its
                                    component parts do).
         <6>     2/11/99    PCG     add comments because Andy rightfully pointed out my original
                                    attempts sucked; also, remove some excessive TARGET_CARBON
                                    gymnastics
         <5>     1/22/99    PCG     TARGET_CARBON
         <4>    11/11/98    PCG     fix header
         <3>    11/10/98    PCG     separate change histories
         <2>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <1>    10/11/98    Quinn   Changed name from "MorePrefix.h" to "MoreSetup.h".

	Start of "MorePrefix.h" change history (most recent first):

         <3>     5/11/98    Quinn   Use MoreAssertQ instead of MoreAssert.
         <2>     7/24/98    PCG	    rid of triplet #includes
         <2>     6/23/98    PCG     add copyright disclaimer stuff
         <1>     6/23/98    PCG     initial checkin
*/

#pragma once

	//
	//	We never want to use old names or locations.
	//	Since these settings must be consistent all the way through
	//	a compilation unit, and since we don't want to silently
	//	change them out from under a developer who uses a prefix
	//	file (C/C++ panel of Target Settings), we simply complain
	//	if they are already set in a way we don't like.
	//

#ifndef OLDROUTINELOCATIONS
	#define OLDROUTINELOCATIONS 0
#elif OLDROUTINELOCATIONS
	#error OLDROUTINELOCATIONS must be FALSE when compiling MoreIsBetter.
#endif

#ifndef OLDROUTINENAMES
	#define OLDROUTINENAMES 0
#elif OLDROUTINENAMES
	#error OLDROUTINENAMES must be FALSE when compiling MoreIsBetter.
#endif

	//
	//	The next statement sets up the various TARGET_xxx 
	//	variables needed later in this file.
	//
	
#include <TargetConditionals.h>

	//
	//  "TargetConditionals.h" on Mac OS X doesn't define 
	//  TARGET_API_MAC_CARBON, which is kinda annoying.  So, 
	//  if we're building for Mach-O and it hasn't been set, 
	//  set it.  All Mach-O builds are inherently Carbon builds.
	//
	//  Note that we *have* to set TARGET_API_MAC_CARBON because 
	//  MoreIsBetter code tests it.  However, we also want to set 
	//  the other values because otherwise GCC won't use its 
	//  precompiled header.  I thought about fixing this by 
	//  including <ConditionalMacros.h> via <CoreServices/CoreServices.h>, 
	//  but the whole goal of using <TargetConditionals.h> was to 
	//  allow MoreIsBetter projects to have no dependencies on 
	//  frameworks beyond System.framework.
	//

#if TARGET_RT_MAC_MACHO
	#define TARGET_API_MAC_OS8 0
	#define TARGET_API_MAC_CARBON 1
	#define TARGET_API_MAC_OSX 1
#endif

	//
	//	We need a master conditional to determine whether to use 
	//	framework notation to reference include files.  There is 
	//  no good way to determine whether the source tree 
	//  is using framework includes or not, so we make a guess and 
	//  say that Mach-O implies framework includes.  However, 
	//  if you're building CFM with framework includes (which is 
	//  possible, although somewhat tricky), you'll have to 
	//  override this.
	//

#if !defined(MORE_FRAMEWORK_INCLUDES)
	#if TARGET_RT_MAC_MACHO
		#define MORE_FRAMEWORK_INCLUDES 1
	#else
		#define MORE_FRAMEWORK_INCLUDES 0
	#endif
#endif

	//
	//	The following works around a problem in <CoreFoundation/CFBase.h>
	//	in Mac OS X.  CF is assuming that the CW Mach-O compiler 
	//	would not use framework includes, which is not the case for the 
	//	CWPro7 and later compilers.  We only do this for the CW build 
    //	because otherwise the presence of the define prevents Project Builder 
    //	from using Carbon's precompiled header.
	//
	
#if MORE_FRAMEWORK_INCLUDES
	#if defined(__MWERKS__)
		#if !defined(__CF_USE_FRAMEWORK_INCLUDES__)
		    #define __CF_USE_FRAMEWORK_INCLUDES__ 1
		#endif
	#endif
#endif

	// 
	//	Previouly we would bring in <MacErrors.h> at this point.
	//	However, that's somewhat counter to the idea behind flat 
	//	includes, so I've eliminated it.
	//

	//
	//	Now that we've included a Mac OS interface file,
	//	we know that the Universal Interfaces environment
	//	is set up.  MoreIsBetter requires Universal Interfaces
	//	3.4 or higher.  Check for it.  Of course, "TargetConditionals.h" 
	//  on Mac OS X doesn't set UNIVERSAL_INTERFACES_VERSION, so 
	//  we only check this if we're not using framework includes.
	//

#if !MORE_FRAMEWORK_INCLUDES
	#if !defined(UNIVERSAL_INTERFACES_VERSION) || UNIVERSAL_INTERFACES_VERSION < 0x0341
		#error MoreIsBetter requires Universal Interfaces 3.4.1 or higher.
	#endif
#endif

	//
	//	We usually want assertions and other debugging code
	//	turned on, but you can turn it all off if you like
	//	by setting MORE_DEBUG to 0.  Also, now that we use 
	//  the standard C assertion mechanism, we default to 
	//  have MORE_DEBUG off if NDEBUG is set.
	//

#if !defined(MORE_DEBUG)
	#if defined(NDEBUG)
		#define MORE_DEBUG 0
	#else
		#define MORE_DEBUG 1
	#endif
#endif

	//
	//  We now use standard C assertions throughout MoreIsBetter.
	//  Previously we would declare two custom assertion macros, 
	//  MoreAssert and MoreAssertQ.
	//
	//	Our assertion macros compile down to nothing if
	//	MORE_DEBUG is not true. MoreAssert produces a
	//	value indicating whether the assertion succeeded
	//	or failed. MoreAssertQ is Quinn's flavor of
	//	MoreAssert which does not produce a value.
	//

#include <assert.h>

	//
	//  Chances are that if you're building CFM then you'd much 
	//  rather have assert triggered DebugStr that abort.  The 
	//  following redefines assert that way.  We don't do this 
	//  for Mach-O because you might be building a Mach-O tool that 
	//  doesn't have access to DebugStr.
	// 
	
#if TARGET_RT_MAC_CFM && !defined(NDEBUG)
	#undef assert
	#define assert(x) ((x) ? ((void) 0) : DebugStr("\pMoreIsBetter assertion failure: " #x))
#endif

	// 
	//  Finally, we define MoreAssertPCG, to accomodate some 
	//  older MoreIsBetter code that tests the result of its 
	//  assertions.  This doesn't fit in with the standard C 
	//  assert model, so I've deprecated the approach.  However
	//  there's a bunch of existing code that works that way 
	//  and reworking it would be likely to generate errors. 
	//  Fortunately, all of the old code that uses this is 
	//  Carbon based, so I don't have to worry about not having 
	//  access to DebugStr.
	// 
	
#if MORE_DEBUG
	#define MoreAssertPCG(x) \
		((x) ? true : (DebugStr ("\pMoreIsBetter assertion failure: " #x), false))
#else
	#define MoreAssertPCG(x) (true)
#endif
