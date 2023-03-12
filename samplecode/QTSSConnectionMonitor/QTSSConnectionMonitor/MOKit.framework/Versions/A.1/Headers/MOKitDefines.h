// MOKitDefines.h
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

// ABOUT MOKitDefines.h
//
// This little piece of cruft has to do with making sure that public externs
// get exported in the MOKit.dll when building on Windows.  It would be nice
// if support for this crap was more automatic, but, oh well.  The deal with
// dll exports is that if things are declared MOKIT_EXTERN they will be
// exported in the dll.  This applies to functions and global variables.
// Objective-C classes are handled automatically by the Makefiles in Cocoa 
// for Windows 4.1 so all you have to worry about are functions and global 
// variables.  (It seems there is a bug in 4.1 that affects this automatic 
// def file generation when your framework has subprojects.  There is a 
// NextAnswer with a makefile workaround to this problem I think, but I don't know 
// the number.  You might search www.next.com for it.)
// Make sure that every non-static function or variable has a 
// header declaration labelled either MOKIT_EXTERN or MOKIT_PRIVATE_EXTERN.  
// Note that _MO_BUILDING_MOKIT must be defined during the build. 
// This is done in the Build Settings of the MOKit target.
//
// This stuff seems to work on Mach and Windows, but I have never tried it with 
// C++ or on Solaris. The C++ and Solraris related parts of this file are just 
// speculative.

#ifndef _MOKITDEFINES_H
#define _MOKITDEFINES_H

//
//  Platform specific defs for externs
//

#if defined(__MACH__)

//
// For MACH
//

#ifdef __cplusplus
// This isnt extern "C" because the compiler will not allow this if it has
// seen an extern "Objective-C"
#define MOKIT_EXTERN		extern
#define MOKIT_PRIVATE_EXTERN	__private_extern__
#else
#define MOKIT_EXTERN		extern
#define MOKIT_PRIVATE_EXTERN	__private_extern__
#endif


#elif defined(WIN32)

//
// For Windows
//

#ifndef _MO_BUILDING_MOKIT
#define _MO_WINDOWS_DLL_GOOP	__declspec(dllimport)
#else
#define _MO_WINDOWS_DLL_GOOP	__declspec(dllexport)
#endif

#ifdef __cplusplus
#define MOKIT_EXTERN		_MO_WINDOWS_DLL_GOOP extern "C"
#define MOKIT_PRIVATE_EXTERN	extern "C"
#else
#define MOKIT_EXTERN		_MO_WINDOWS_DLL_GOOP extern
#define MOKIT_PRIVATE_EXTERN	extern
#endif


#elif defined(SOLARIS)

//
//  For Solaris
//

// MF: MOKit does not work on Solaris.  The other framework I copied this crap from had this stuff in it and I just didnt rip it out.  Thats not to say it couldnt work on Solaris, but I have never tried it.  And they have a different project model.

#ifdef __cplusplus
#define MOKIT_EXTERN		extern "C"
#define MOKIT_PRIVATE_EXTERN	extern "C"
#else
#define MOKIT_EXTERN		extern
#define MOKIT_PRIVATE_EXTERN	extern
#endif


#endif

#endif // _MOKITDEFINES_H
