/*
	File:		MoreOSL.h

	Contains:	What OSL should have been.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSL.h,v $
Revision 1.8  2002/11/08 23:48:43         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL.

Revision 1.7  2001/11/07 15:54:10         
Tidy up headers, add CVS logs, update copyright.


         <6>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>     27/3/00    Quinn   Expanded comments. Dinked with some error numbers.
         <3>     20/3/00    Quinn   Replaced debug Boolean with individual flags.  Implemented
                                    formRange.  Exposed routines for "make" event handlers to
                                    support "with properties".  General access by name and ID. 
                                    "coerceToken" object primitive.
         <2>      9/3/00    Quinn   Changed how we do access by name to support strings beyond
                                    Str255.
         <1>      6/3/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <AppleEvents.h>
#endif

// MIB Prototypes

#include "MoreOSLTokens.h"

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// Debugging

// Bits in the gDebugFlags variable.

enum {
	kMOSLNoLogging          = 0,				// no debug logging
	kMOSLLogOSLMask         = 0x0001,			// log MOSL�s interaction with OSL
	kMOSLLogCallbacksMask 	= 0x0002,			// log MOSL�s class callbacks
	kMOSLLogGeneralMask 	= 0x0004,			// log MOSL�s general class event handlers
	kMOSLLogDispatchMask 	= 0x0008			// log MOSL�s Apple event dispatcher
};

#if MORE_DEBUG

	extern UInt32 gDebugFlags;

#endif

/////////////////////////////////////////////////////////////////
// Error Numbers

// DTS Q&A OV 02 says that errors in the range 1000..9999 are legal
// for developers to use.  MOSL is stealing some of those from you.

enum {
	kMOSLDirectObjectRequiredErr   			= 5300,
	kMOSLDirectObjectNotAllowedErr			= 5301,
	kMOSLUnrecognisedOperatorErr   			= 5302,
	kMOSLCantRelateObjectsErr      			= 5303,
	kMOSLStringOperatorNotSupported 		= 5304,
	kMOSLClassHasNoElementsOfThisTypeErr 	= 5305,
	kMOSLBoundaryNotInSameContainerErr		= 5306,
	kMOSLBoundaryMustBeObjectErr			= 5307,
	kMOSLBoundaryMustBeCompatibleErr		= 5308
};

/////////////////////////////////////////////////////////////////
// Data Access

// The following routines are designed to let your Apple event handlers
// access parameters whose contents might be an object specifier.  In
// your handler, you should call AEGetParamDesc with typeWildCard to
// get the parameter data, and pass the resulting descriptor to 
// one of the following routines.  If the descriptor is an object specifier,
// the routine will resolve the object specifier and call your class getter
// to get the actual data.

extern pascal OSStatus MOSLCoerceObjDesc(const AEDesc *desc, DescType desiredType, AEDesc *coercedDesc);
	// Coerce the specified descriptor to the specified type, resolving and getting
	// the data for an object specifier if necessary.
	
extern pascal OSStatus MOSLCoerceObjDescToPtr(const AEDesc *desc, DescType desiredType, void *buf, Size bufSize);
	// Coerces the specified descriptor to the specified type, resolving and getting
	// the data for an object specifier if necessary.  The data is stored in the
	// variable pointed to by buf, whose size is bufSize.  The buffer size must
	// exactly match the size of the data (except for typeStr255, as described next).
	//
	// typePString is used as a pseudo-type that can be passed to MOSLCoerceObjDescToPtr to
	// extract a Pascal string from a descriptor.  You must pass the size of the string
	// (including the length byte) to the bufSize parameter.  MOSLCoerceObjDescToPtr
	// coerces the descriptor type typeText and then copies the text data into the string
	// pointed to by buffer, setting up the length byte appropriately.

/////////////////////////////////////////////////////////////////
// Event Table

// When you initialise MOSL, you pass in two global static structures.
// The first of these is the event table, which gives the set of Apple events
// which you�re prepared to handle and some basic information about those events.
// The second structure, the class table, is defined later in this header file.

// MOSLEventIndex is used whenever we declare a integer that represents a
// an index into the event table.  

typedef UInt32 MOSLEventIndex;

// MOSLDirectObjectRequirement is used as a field of the event table entries
// to determine how MOSL handles the direct object for that event.

typedef UInt32 MOSLDirectObjectRequirement;
enum {
	kMOSLDOOptional = 0,				// the direct object parameter is optional for this event
	kMOSLDORequired,					// the direct object parameter is required for this event
										// if not present you get a kMOSLDirectObjectRequiredErr error
	kMOSLDOIllegal,						// no direct object parameter is allowed for this event
										// if a direct object is present, you get a kMOSLDirectObjectNotAllowedErr error
	kMOSLDOBadOK						// a bad direct object parameter is allowed for this event
										// typically used by the "exists" event
};

// MOSLResultAction is used as a field of the event table entries
// to determine how MOSL handles the reply for that event.

typedef UInt32 MOSLResultAction;
enum {
	kMOSLReturnNone = 0, 				// this event has no reply
	kMOSLReturnDefault, 				// this event�s reply is handled in the default way, 
										// contrast with below
	kMOSLReturnCountList, 				// when the direct object is a list, 
										// this event�s reply is the count of the list
										// typically used for the "count" event
	kMOSLReturnCollapseBooleanList		// when the direct object is a list of Booleans, 
										// this event�s reply collapses the list to a single Boolean by ANDing each entry
										// typically used for the "exists" event
};

// The event table is made up of MOSLEventEntry structures.  Each
// structure contains the class and event ID, and a specification
// of how to handle the direct object parameter and reply for this event.
// MOSL registers an Apple event handler for each event in this table.

struct MOSLEventEntry {
	AEEventClass 				classID;
	AEEventID 					eventID;
	MOSLDirectObjectRequirement dirObjReq;
	MOSLResultAction 			resultAction;
};
typedef struct MOSLEventEntry MOSLEventEntry, *MOSLEventEntryPtr;
typedef const MOSLEventEntry *MOSLEventTablePtr;

/////////////////////////////////////////////////////////////////
// Class Table

// The class table is the second core data structure you must supply when
// initialising MOSL.  The class table lists all of the classes native to 
// your application, the properties for those classes, and how to handle 
// events for those classes.

// The first component of a class table entry is the property table for the
// class.  The property table is an array of entries of type MOSLPropEntry.
// Each entry contains the property name (the four character code that uniquely
// identifies the property) and a data long.  The contents of the data
// long are dependent on the property type.  For standard properties, the
// long can contain either kMOSLPropReadOnly or kMOSLPropReadWrite to indicate
// whether the property is read only or read/write.  The meaning of the property
// data for special properties is discussed below.
//
// There are three special property name values:
// 
// o pProperties -- If you include this property in your property table,
//   MOSL will automatically handle getting (or setting) all the properties of
//   an object by iterating through the class�s property list.  The property
//   had the standard meaning in this case.
//
// o pInherits -- This value indicates that this class inherits the properties
//   of another class.  The property data long must contain the class ID of
//   the class whose properties are to be inherited.  If present, this property
//   must be the last property in the property table.
//
//   The class you inherit properties from can also inherit properties from
//   another class.
//
//   The pInherits property need not be related to the similar property in your
//   application�s 'aete', nor to the inheritance scheme used by your native
//   objects.  It is only used by MOSL to determine whether a particular class
//   has a particular property.
//
// o kMOSLPropNameLast -- This value indicates the end of the property table.
//   The property data long must be kMOSLPropDataLast.  This property is only
//   needed if the property table doesn�t end with a pInherits.

typedef UInt32 MOSLPropIndex;

struct MOSLPropEntry {
	DescType propName;			// property name or pInherits
	UInt32   propData;			// kMOSLPropReadOnly, kMOSLPropReadWrite (or class to inherit from for pInherits)
};
typedef struct MOSLPropEntry MOSLPropEntry, *MOSLPropEntryPtr;
typedef const MOSLPropEntry *MOSLPropTablePtr;

enum {
	kMOSLPropReadOnly  	= 0,				// this property is read-only
	kMOSLPropReadWrite 	= 1,				// this property is read/write
	kMOSLPropNameLast	= 0xFFFFFFFF,		// this is the last property in the property table
	kMOSLPropDataLast   = 0xFFFFFFFF		// when propName is kMOSLPropNameLast, propData must be kMOSLPropDataLast
};

// The second component of a class table entry is a table of event handlers.
// A class�s event handler table is very much like the system Apple event dispatch
// table, except that there is a class event handler table for each class known
// to MOSL.  When MOSL receives an Apple event, it resolves the direct object
// of the event and then determines the class of that object.  It then calls
// the corresponding handler in the class�s event handler table.
//
// In the OSL biz this is known as "class first" dispatching (-:
//
// The event handler table is indexed by the same indexes used for the event
// table (ie MOSLEventIndex).  For example, if you put the 'core'/'getd' event
// first in your event table, you must put each class�s "get data" handler
// first in its event handler table.  Also, the event handler array must
// have an entry for each event in the event table.
//
// If your class doesn�t handle a specific event, you should set the corresponding
// entry in the event handlers table to NULL.  MOSL will return the appropriate
// error (errAECantHandleClass) if someone tries to dispatch an event to a class
// that doesn�t understand that event.
//
// For the common 'core' events, you can point your event handler to a generic
// implementation that call�s your classes object primitives.  See the comment
// above MOSLGeneralCount for details.

typedef pascal OSStatus (*MOSLClassEventHandler)(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result);
	// Each element of the class�s event handler table is a pointer
	// to a routine of type MOSLClassEventHandler.  This routine is much like
	// an Apple event handler except:
	//
	// a) it receives a token for its direct object, and
	// b) the result is an AEDesc, not an AppleEvent
	//
	// This is because MOSL handles lists of direct objects automatically for you.
	// Your handler just does the event on a single token and returns the corresponding
	// result and, if the direct object is a list, MOSL takes care of calling you 
	// multiple times and assembling the results.
	//
	// Just like an Apple event handler, your handler can get its parameters from
	// the Apple event by calling AEGetParamDesc.

// The MOSLClassEventTablePtr type is used specifically to reference the first
// element of the class�s event handler table.
	
typedef const MOSLClassEventHandler *MOSLClassEventTablePtr;

// MOSL provides a generic implementation of a number of core Apple events.  These
// implementations are layered on top of your classes object primitives (see below).
// You can take advantage of these generic implementations by simply setting the
// appropriate entry in your class�s event handler table to point to the generic
// routine.

extern pascal OSStatus MOSLGeneralCount(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result);
	// This routine implements the "count" Apple event based on your class�s
	// counter object primitive (MOSLClassCounter).  It handles the "each"
	// parameter.
	
extern pascal OSStatus MOSLGeneralExists(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result);
	// This routine implements the "exists" Apple event by simply looking to
	// see whether the direct object is valid.  To use this routine, the
	// "exists" Apple event must have its direct object requirement set to
	// kMOSLDOBadOK in the event table.
	
extern pascal OSStatus MOSLGeneralGetData(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result);
	// This routine implements the "get data" Apple event based on your class�s
	// getter object primitive (MOSLClassGetter).  It handles the "as" parameter.
	
extern pascal OSStatus MOSLGeneralSetData(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result);
	// This routine implements the "set data" Apple event based on your class�s
	// setter object primitive (MOSLClassSetter).

extern pascal OSStatus MOSLSetObjectProperties(const MOSLToken *objTok, const AERecord *data);
	// This routine is intended to be called as part of your "make" event handler
	// when the event contains a "with properties" parameter. It allows you to
	// specify a record containing property values.  For each settable property,
	// the routine will call your class�s "setter" object primitive to set the
	// actual property value.
	//
	// objTok must be a token for your object, not for any specific property.
	// data must be a valid AERecord (typeAERecord).

extern pascal OSStatus MOSLGeneralAccessByName(const MOSLToken *containerTok, DescType elementType, const AEDesc *name, MOSLToken *valueTok);
	// This routine implements a generic "accessByName" object primitive
	// in terms of the "counter", "accessByIndex", and "getter" primitives.
	// The routine calls "counter" to count the number of elements of type
	// elementType within containerTok, then repeatedly calls "accessByIndex"
	// to get tokens for each element, then calls "getter" to get the value
	// for the name property of that token, then internally compares the name
	// to the name parameter.  If they match, it returns the token for the
	// matching element in valueTok.
	//
	// You can put this routine as your "accessByName" primitive in your
	// class table entry as long as your class implements "counter", 
	// "accessByIndex", and "getter" for pName.

extern pascal OSStatus MOSLGeneralAccessByUniqueID(const MOSLToken *containerTok, DescType elementType, SInt32 uniqueID, MOSLToken *valueTok);
	// This routine is like MOSLGeneralAccessByName except that it implements
	// support for the "accessByUniqueID" object primitive.
	//
	// You can put this routine as your "accessByUniqueID" primitive in your
	// class table entry as long as your class implements "counter", 
	// "accessByIndex", and "getter" for pID.

// The third component of a class table entry is the object primitives for the
// class.  These primitives are like various callbacks you would otherwise supply
// to OSL except that:
//
// a) there are a set of callbacks for each class.
// b) the callbacks are much more specific
// c) the callbacks use MOSLTokens instead of generic descriptors
		
typedef pascal OSStatus (*MOSLClassCounter)(const MOSLToken *containerTok, DescType elementType, SInt32 *result);
	// Given a reference to an object (containerTok), count the number of elements
	// of type elementType.
	//
	// o primitive may be NULL if class doesn�t have elements
	// o containerTok is always an object, never a property
	// o elementType may be cObject, which you should treat as your default element type
	// o you should return kClassHasNoElementsOfThisTypeErr if element type isn�t one of your elements
	// o you should return errAENoSuchObject if the container doesn�t contain the specified object

typedef pascal OSStatus (*MOSLClassAccessByUniqueID)(const MOSLToken *containerTok, DescType elementType, SInt32 uniqueID, MOSLToken *valueTok);
	// Given a reference to an object (containerTok), return the element (of type
	// elementType) whose unique ID is uniqueID.
	//
	// o primitive may be NULL if class doesn�t support accessing objects by unique ID
	// o containerTok is always an object, never a property
	// o elementType may be cObject, which you should treat as your default element type
	// o you should return kClassHasNoElementsOfThisTypeErr if element type isn�t one of your elements
	// o you should return errAENoSuchObject if the container doesn�t contain the specified object\
	// o for polymorphic objects, return a token of the "best type"
	// o you may want to take advantage of MOSLGeneralAccessByUniqueID

typedef pascal OSStatus (*MOSLClassAccessByIndex)(const MOSLToken *containerTok, DescType elementType, SInt32 index, MOSLToken *valueTok);
	// Given a reference to an object (containerTok), return the element (of type
	// elementType) with the specified index.
	//
	// o primitive may be NULL if class doesn�t have elements
	// o containerTok is always an object, never a property
	// o elementType may be cObject, which you should treat as your default element type
	// o index is always a positive number, 1-based index starting from the beginning of
	//   the container (if the object specifier included a negative index, MOSL has already
	//   called your "counter" primitive to turn it into a positive number)
	// o you should return kClassHasNoElementsOfThisTypeErr if element type isn�t one of your elements
	// o you should return errAENoSuchObject if the container doesn�t contain the specified object
	// o for polymorphic objects, return a token of the "best type"

typedef pascal OSStatus (*MOSLClassAccessByName)(const MOSLToken *containerTok, DescType elementType, const AEDesc *name, MOSLToken *valueTok);
	// Given a reference to an object (containerTok), return the element (of type
	// elementType) with the specified name.  The name is given as an AEDesc because
	// there are many ways string formats it can come in to MOSL as.  You should use
	// the MOSLStringEqualsXxx routines (above) to compare the name against the
	// names of your objects.
	//
	// o primitive may be NULL if class doesn�t support accessing objects by name
	// o containerTok is always an object, never a property
	// o elementType may be cObject, which you should treat as your default element type
	// o you should return kClassHasNoElementsOfThisTypeErr if element type isn�t one of your elements
	// o you should return errAENoSuchObject if the container doesn�t contain the specified object
	// o for polymorphic objects, return a token of the "best type"
	// o you may want to take advantage of MOSLGeneralAccessByName

typedef pascal OSStatus (*MOSLClassGetter)(const MOSLToken *tok, AEDesc *desc);
	// Given a reference to an object or property, return the data for that reference.
	//
	// o routine is required for all classes
	// o called for both objects and properties
	// o objects should return an object specifier for the object
	// o properties should return the actual property data (for data properties)
	//   or a token (for properties whose value is a reference to another object)
	// o only called for properties in your class�s property table (getter 
	//   infrastructure checks property table and returns errAENoSuchObject)

typedef pascal OSStatus (*MOSLClassSetter)(const MOSLToken *tok, const AEDesc *desc);
	// Given a reference to a property, set the property to the data in desc.
	//
	// o only called for properties, never for objects
	// o only called for properties in your class�s property table (setter 
	//   infrastructure checks property table and returns errAENoSuchObject)
	// o only called for read/write properties (setter infrastructure uses
	//   property table to return errAENotModifiable for read-only properties)
	// o routine is optional, may be NULL if no properties are read/write
	// o use MOSLCoerceObjDesc to extract data from desc

typedef pascal OSStatus (*MOSLClassCoerceToken)(const MOSLToken *tok, DescType toClass, MOSLToken *coercedTok);
	// Coerce from a subclass to one of its superclass.  You must supply
	// this routine if your access by index/id/name object primitives return
	// a subclass.
	//
	// This routine is not necessary if your access object primitives always
	// return a token of the same type of the requested type (the elementType
	// parameter to the access object primitive) or you are the base class.
	//
	// o tok is never a property
	// o toClass can be any of the superclasses for that subclass
	// o toClass can also be cObject, in which case you should coerce
	//   to the base class
	// o coercedTok can be NULL
	// o return errAECoercionFail if toClass isn�t a superclass of tok

// MOSLClassIndex is used whenever we declare a integer that represents a
// an index into the class table.  

typedef UInt32 MOSLClassIndex;

// Finally, we define the structure use for each entry in the class table.
// For information about each component, see the corresponding comments
// above.

struct MOSLClassEntry {
	OSType						classID;
	
	// first component -- property table

	MOSLPropTablePtr		    properties;

	// second component -- event handler table

	MOSLClassEventTablePtr 		eventHandlers;
	
	// third component -- object primitives
	
	MOSLClassCounter			counter;
	MOSLClassAccessByUniqueID	accessByUniqueID;
	MOSLClassAccessByIndex		accessByIndex;
	MOSLClassAccessByName		accessByName;
	MOSLClassGetter				getter;
	MOSLClassSetter				setter;
	MOSLClassCoerceToken		coerceToken;
};
typedef struct MOSLClassEntry MOSLClassEntry, *MOSLClassEntryPtr;
typedef const MOSLClassEntry *MOSLClassTablePtr;

// IMPORTANT:
// The first entry of the class table *must* be the application�s class
// (cApplication).

enum {
	kMOSLCApplicationIndex = 0
};

/////////////////////////////////////////////////////////////////
// Globals and Initialisation

typedef pascal OSStatus (*MOSLDefaultEventHandler)(const AEDesc *dirObj, const AppleEvent *theEvent, MOSLEventIndex eventIndex, AEDesc *result);
	// Your application may optionally choose to provide a default handler.
	// This handler is called if, after direct object resolution, we find
	// that the class of the direct object is not one of the classes in the
	// class table.  This is useful for handling events on classes that
	// you don�t otherwise recognise, such as the "open" event on class
	// typeFSS.
	//
	// Note that your default handler is invoked after object resolution
	// and list breakdown, so it will be passed the actual elements of a
	// list, not the list itself.

typedef pascal void (*MOSLErrorToStringProc)(OSStatus errNum, Str255 errStr);
	// You may choose to supply this routine to map error numbers to strings.
	// When MOSL encounters an error processing an Apple event, it calls this
	// routine to determine the error message and then, if the string is not
	// empty, sets the keyErrorString parameter of the reply to this string.
	//
	// This routine is optional for prototype implementations but is required
	// for real applications.  The reason is that MOSL has some internal errors
	// (see the top of the file) that must be mapped to sensible error strings.
	// MOSL does not provide these strings because their specification is tied
	// to how your application performs localisation.
	//
	// At a minimum, your error to string routine must be able to map these
	// MOSL errors to sensible values.  You must also be prepared to map any
	// errors that your various MOSL callbacks might return.  OTOH, you don�t
	// have to supply strings for common Mac OS error numbers because AppleScript
	// contains built-in error strings.
	
extern pascal OSStatus InitMoreOSL(MOSLEventTablePtr eventTableBase, MOSLEventIndex eventTableSize,
						 MOSLClassTablePtr classTableBase, MOSLClassIndex classTableSize,
						 MOSLDefaultEventHandler defaultHandler,
						 MOSLErrorToStringProc   errorToStringProc);
	// This routine initialises the More Object Support Library (MOSL) module.
	// You must supply a pointer to the base of your event and class table,
	// and the number of elements in each table.  Neither table is optional (ie
	// the pointer must not be NULL) although they may be empty (ie the size can
	// be zero).  You may also provide a pointer to a default event handler
	// and an error to string mapping routine.
	//
	// For a description of the event table format and semantics, see "Event Table" above.
	// For a description of the class table format and semantics, see "Class Table" above.
	//
	// Note that there is no TermMoreOSL routine.  This module assumes that you
	// will initialise it at the beginning of your application and that the termination
	// will be done by the Process Manager when the application quits.  Further note
	// that this routine expects that, if InitMoreOSL fails, your application will
	// refuses to launch.
	//
	// The only likely failures are:
	//
	// o insufficient system sofware -- The routine checks for the presence of
	//   a good ObjectSupportLib and returns an error gracefully without modifying
	//   any state if this check fails.  Your application may then choose to revert
	//   to its non-scripting Apple event support.
	//
	// o insufficient memory -- If the routine runs out of memory during the initialisation
	//   process, it makes no attempt to clean up gracefully.
	
#ifdef __cplusplus
}
#endif
