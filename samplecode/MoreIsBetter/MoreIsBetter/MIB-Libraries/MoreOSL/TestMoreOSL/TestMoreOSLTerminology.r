/*
	File:		TestMoreOSLTerminology.r

	Contains:	Rez source for the 'aete' resource.

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

$Log: TestMoreOSLTerminology.r,v $
Revision 1.7  2002/11/08 23:51:06         
Framework includes in Rez!

Revision 1.6  2001/11/07 15:58:28         
Tidy up headers, add CVS logs, update copyright.


         <5>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <4>     25/4/00    Quinn   Comment that "suite info" etc are deprecated. Expanded the set
                                    of access forms we claim to support to cover the forms we
                                    actually do support. Fix some properties that were incorrectly
                                    marked as read/write.
         <3>     27/3/00    Quinn   Added enumSaveOptions. Some description changes.  Tidied
                                    comments.
         <2>     20/3/00    Quinn   Use "MoreSetup.r"  Added/fixed comments.  Added various
                                    properties ("best type", "next unique ID", "modified",
                                    "positition", "frontmost").  Fixed cFile and commented the
                                    problem.
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.r"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
    #include <Carbon.r>
#else
    #include "AEUserTermTypes.r"
    #include "AppleEvents.r"
    #include "AERegistry.r"
    #include "AEDataModel.r"
    #include "ASRegistry.r"
#endif

// Our Prototypes

#include "TestMoreOSLTerminology.h"

/////////////////////////////////////////////////////////////////

// Neat Trick:
// This 'aete' is largely derived from the standard 'aeut' resource, which
// is the terminology built in to AppleScript.  You can pull apart the standard
// terminology using the following MPW command:
//
//		DeRez "{SystemFolder}"Extensions:AppleScript �
//			"{RIncludes}"AEUserTermTypes.r �
//			-only 'aeut' �
//			-d aeut_RezTemplateVersion=0
//
// IMPORTANT: This trick only works on Mac OS 9.0 and later.

resource kAETerminologyExtension (0, purgeable)
{
	0x01,				// major version
	0x00,				// minor version
	english,			// language
	roman,				// script

	{					// suites array

		// *** Required Suite ***

		// By not including any information in the required event suite,
		// we tell clients that we support the entire suite.
		
		"Required Events",
		"Events that all applications must support.",
		kAERequiredSuite,
		1,
		1,
		{ },				// events
		{ },				// classes
		{ },				// comparison operators
		{ },				// enumerations

// When debugging, you might want to hit this switch so that the Script
// Editor shows you all of the standard events.  It�s good to be able to
// compare your implementation of the core suite with the standard.

#define MORE_OSL_INCLUDE_CORE_SUITE 0
#if MORE_OSL_INCLUDE_CORE_SUITE && MORE_DEBUG

		// *** Core Suite ***

		"Standard Events",
		"Common terms for most applications.",
		kAECoreSuite,
		1,
		1,
		{ },				// events
		{ },				// classes
		{ },				// comparison operators
		{ },				// enumerations

#endif

		// *** Limited Core Suite ***

		// If you implement the 'core' event suite in some limited fashion,
		// you�re supposed to include a copy in your dictionary under a
		// different name.  There doesn�t seem to be any standard name for this
		// (ClarisWorks, Finder and JPEGView use 'CoRe', PhotoFlash uses 'Core',
		// Tex-Edit uses 'CorE') so I just chose one at random.
		
		"Standard Events",
		"Common terms for most applications.",
		'CoRe',
		1,
		1,
		{					// events
		
		// open event
		
			"open",
			"Open the specified object(s)",
			kCoreEventClass,
			kAEOpenDocuments,

			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			typeObjectSpecifier,
			"list of objects to open",
			directParamRequired,
			singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters
			
			{ },

		// run event
		
			"run",
			"Run the application.  Will open an untitled document.",
			kCoreEventClass,
			kAEOpenApplication,

			// reply

			noReply,
			"",
			replyOptional,
			singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			noParams,
			"no direct parameter required",
			directParamOptional, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters
			
			{ },

		// reopen event

			"reopen",
			"Reactivate the application.  Will open an untitled document if no window is open.",
			kCoreEventClass,
			kAEReopenApplication,

			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			noParams,
			"no direct parameter required",
			directParamOptional, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters
			
			{ },
		
		// print (not supported by application)
		
		// quit
		
			"quit",
			"Quit application",
			kCoreEventClass,
			kAEQuitApplication,

			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters
			
			{					
				"saving",
				keyAESaveOptions,
				enumSaveOptions,
				"specifies whether to save currently open documents",
				optional, singleItem, enumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular
			},

		// close event
		
			"close",
			"Close an object",
			kAECoreSuite,
			kAEClose,

			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			typeObjectSpecifier,
			"the object to close",
			directParamRequired, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters

			{
				"saving",
				keyAESaveOptions,
				enumSaveOptions,
				"specifies whether changes should be saved before closing",
				optional, singleItem, enumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved,
				reserved,
				reserved,
				reserved,
				prepositionParam, notFeminine, notMasculine, singular,
#if 0
				// This parameter is currently not implemented but should be.
				
				"saving in",
				keyAEFile,
				typeAlias,
				"the file in which to save the object",
				optional, singleItem, notEnumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved,
				reserved,
				reserved,
				reserved,
				prepositionParam, notFeminine, notMasculine, singular,
#endif 
			},

		// count event

			"count",
			"Return the number of elements of an object",
			kAECoreSuite,
			kAECountElements,

			// reply

			typeLongInteger,
			"the number of elements",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			typeObjectSpecifier,
			"the object whose elements are to be counted",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters

			{
				"each",
				keyAEObjectClass,
				typeType,
				"if specified, restricts counting to objects of this class",
				optional, singleItem, notEnumerated, reserved, enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular
			},
		
		// delete event (not supported because it doesn�t make sense for this application)
		
			// Perhaps I should support this event for windows, but that seems
			// like a pointless endeavour given that to close windows you really
			// need to use close.
		
		// duplicate event (not supported because it doesn�t make sense for this application)
		
		// exists event
		
			"exists",
			"Verify if an object exists",
			kAECoreSuite,
			kAEDoObjectsExist,
			
			// reply

			typeBoolean,
			"true if it exists, false if not",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,
			
			// direct object

			typeObjectSpecifier,
			"the object in question",
			directParamRequired, singleItem, notEnumerated, doesntChangeState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters

			{
			},

		// make event
		
			"make",
			"Make a new element",
			kAECoreSuite,
			kAECreateElement,

			// reply

			typeObjectSpecifier,
			"to the new object(s)",
			replyRequired, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,

			// direct object

			noParams,
			"",
			directParamOptional, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,

			// other parameters

			{
				"new",
				keyAEObjectClass,
				typeType,
				"the class of the new element",
				required, singleItem, notEnumerated, 
				reserved, 
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular,

// Probably should support the "at" parameter eventually, but for now
// we just ignore it.

#if 0
				"at",
				keyAEInsertHere,
				cInsertionLoc,
				"the location at which to insert the element",
				optional, singleItem, notEnumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular,
#endif

				// "with data" parameter deleted because the document
				// only has properties

				"with properties",
				keyAEPropData,
				typeAERecord,
				"the initial values for the properties of the element",
				optional, singleItem, notEnumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular
			},

		// move event (not supported because it doesn�t make sense for this application)
		
		// save event
		
			"save",
			"Save an object",
			kAECoreSuite,
			kAESave,
			
			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,
			
			// direct object

			typeObjectSpecifier,
			"the object to save, a document or results window",
			directParamRequired, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
			
			// other parameters

			{
				"in",
				keyAEFile,
				typeAlias,
				"the file in which to save the object",
				optional, singleItem, notEnumerated,
				reserved,
				enumsAreConstants, enumListCanRepeat, paramIsValue, notParamIsTarget,
				reserved, reserved, reserved, reserved,
				prepositionParam, notFeminine, notMasculine, singular,

				// "as" parameter not an option because we can only
				// save document windows as summary documents and results
				// windows are text files
			},

// Should support select event and selection classes; on the todo list.

#if 0

		// select event
		
			"select",
			"Make a selection",
			kAEMiscStandards,
			kAESelect,

			// reply

			noReply,
			"",
			replyOptional, singleItem, notEnumerated, notTightBindingFunction, enumsAreConstants, enumListCanRepeat, replyIsValue,
			reserved, reserved, reserved, reserved, reserved,
			verbEvent,
			reserved, reserved, reserved,
			
			// direct object

			typeObjectSpecifier,
			"the object to select",
			directParamRequired, singleItem, notEnumerated, changesState, enumsAreConstants, enumListCanRepeat, directParamIsValue, directParamIsTarget,
			reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved,
			
			// other parameters

			{
			},

#endif

		// data size (not supported because I don�t understand it)
		
		// suite info
		// event info
		// class info
		// -- None of these are supported because AppleScript engineering
		//    has deprecated them.
		
			// I may integrate support for these events into MOSL at some
			// stage, but until I do the TestMOSL application won�t support
			// them.
		
		},					// end events

		{					// classes

		// applications class

			"application",
			cApplication,
			"An application program",

			// properties of cApplication

			{	
				"properties",
				pProperties,
				typeAERecord,
				"property that allows getting and setting of multiple properties",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"best type",
				keyAEBestType,
				typeType,
				"the most specific type of this object",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"name",
				pName,
				typeText,
				"the name of the application",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"frontmost",
				pIsFrontProcess,
				typeBoolean,
				"Is this the frontmost application?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

// Should support select event and selection classes; on the todo list.

#if 0

				"selection",
				pSelection,
				cSelection,
				"the selection visible to the user.  Use "
				"the �select� command to set a new selection; "
				"use �contents of selection� to get "
				"or change information in the document.",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

#endif

// I originally intended to support the "clipboard", then I realised
// exactly what that involved.  Originally I thought it just meant that
// you could just use the Scrap Manager to extract the current clipboard
// contents.  Then I looked at how the Finder implemented this and I
// got very confused.  So I just decided not to support it in this
// sample.

#if 0
				"clipboard",
				pClipboard,
				typeWildCard,
				"the contents of the clipboard for this application",
				reserved,
				listOfItems, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
#endif

				"version",
				pVersion,
				typeText,
				"the version of the application",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
			
				// The following properties are used by the test script
				// to control behaviour of TestMOSL.  They always have
				// a stub implementation in the application, but they only
				// appear in the dictionary in debug builds.
				
				#if MORE_DEBUG

					"debug",
					pMyDebug,
					typeLongInteger,
					"level of debug logging to BBEdit",
					reserved,
					singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
					reserved, reserved, reserved, reserved, reserved,
					noApostrophe, notFeminine, notMasculine, singular,
					
					"next unique ID",
					pMyNextUniqueID,
					typeLongInteger,
					"next unique ID to use for documents and windows",
					reserved,
					singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
					reserved, reserved, reserved, reserved, reserved,
					noApostrophe, notFeminine, notMasculine, singular,

				#endif
			},

			// elements of cApplication

			{
				cDocument,
				{
					formAbsolutePosition,
					formName,
					formUniqueID,
					formRange,
					formTest,
				},
				
				cWindow,
				{
					formAbsolutePosition,
					formName,
					formUniqueID,
					formRange,
					formTest,
				},
				
				cNodeWindow,
				{
					formAbsolutePosition,
					formName,
					formUniqueID,
					formRange,
					formTest,
				},
				
				cAboutWindow,
				{
					formAbsolutePosition,
					formName,
					formUniqueID,
					formRange,
					formTest,
				},
				
				cFile,
				{
					formName,
				},
			},
			
		// applications class

			"applications",
			cApplication,
			"",
			
			// properties of cApplications
			
			{	
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved, singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},
			
			// elements of cApplications
			
			{
			},

		// document class
		
			"document",
			cDocument,
			"A document of a scriptable application",
			
			// properties of cDocument
			
			{
				"properties",
				pProperties,
				typeAERecord,
				"property that allows getting and setting of multiple properties",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"best type",
				keyAEBestType,
				typeType,
				"the most specific type of this object",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"name",
				pName,
				typeText,
				"the document name",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"index",
				pIndex,
				typeLongInteger,
				"the number of this document, counting from the frontmost",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				"ID",
				pID,
				typeLongInteger,
				"a unique ID for this document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"file valid",
				pMyFileValid,
				typeBoolean,
				"does the document have a valid file?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// The existance of the "file" property in our 'aete' requires
				// us to implement the cFile class.  See the description of
				// the cFile class for details.
				
				"file",
				cFile,
				cFile,
				"an alias to the saved document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				#if MORE_DEBUG
				
					// The location on disk property is necessary to exercise
					// the FSSpec compare routine in MOSL.  You might think you
					// could do it using:
					//
					//     every document whose file is alias "Macintosh HD:Test"
					//
					// but this generates a really bogus object specifier because
					// AppleScript confuses the file property with the file class.
					// [2444528]  I don�t really care about this for scripters 
					// (it�s a pretty obscure edge case) but my test script needs 
					// to exercise the functionality so I added another property,
					// "location on disk", that�s the same as the cFile property
					// but in all but name.

					"location on disk",
					pMyLocationOnDisk,
					cFile,
					"an alias to the saved document",
					reserved,
					singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
					reserved, reserved, reserved, reserved, reserved,
					noApostrophe, notFeminine, notMasculine, singular,
				
				#endif

				// There doesn�t appear to be an AppleEvent standard property
				// to map a document to its primary window, so I had to use
				// a custom property.  Coming up with a meaningful but unique
				// name for that property was tricky.
				
				"node display",
				pMyNodeDisplay,
				cNodeWindow,
				"the node window for this document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,			

				"modified",
				pIsModified,
				typeBoolean,
				"Has the document been modified since the last save?",
				reserved,
				singleItem, notEnumerated, 
				
				#if MORE_DEBUG
					readWrite, 
				#else
					readOnly,
				#endif
				
				enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular
			},

			// elements of cDocument

			{
			},

		// documents class
		
			"documents",
			cDocument,
			"",

			// properties of cDocuments

			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},

			// elements of cDocuments

			{
			},


		// file class
		
			// Gotcha:
			// We have to implement a cFile class because we have a cFile
			// property.  If we don�t implement this class, attempts to
			// pass a file by name to the application, for example:
			//
			//     save document 1 in file "Macintosh HD:TestFile"
			//                           ^^^
			// fail with a compile error at the marked location because
			// AppleScript thinks "file" is a property and isn�t expecting 
			// data after the property name.  By adding cFile as a class,
			// we tell AppleScript that "file" may also be a class, which
			// prevents the problem.  Note that we don�t actually do anything
			// with the cFile class, it just has to be here to prevent
			// compilation problems.  Further note that the standard 'aeut'
			// definition of cFile isn�t enough to prevent these problems,
			// we have to declare cFile in our suite.
			//
			// The AppleScript compiler works in mysterious ways. [2444517]
		
			"file",
			cFile,
			"a file on a disk or server (or a file yet to be created)",
			{
		 	},
			{
			},

		// files class

			"files",
			cFile,
			"",
			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},
			{
			},

		// alias class (not supported, assume we get it from 'aetu')

// Should support select event and selection classes; on the todo list.

#if 0

		// selection-object class
		
			"selection-object",
			cSelection,
			"A way to refer to the state of the current "
			"of the selection.  Use the �select� command "
			"to make a new selection.",

			// properties of cSelection
			
			{
				"properties",
				pProperties,
				typeAERecord,
				"property that allows getting and setting of multiple properties",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"contents",
				pContents,
				typeWildCard,
				"the information currently selected.  Use "
				"�contents of selection� to get or change "
				"information in a document.",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular
			},

			// elements of cSelection

			{
			},
#endif

		// window class
		
			"window",
			cWindow,
			"A window",

			// properties of cWindow

			{
				"properties",
				pProperties,
				typeAERecord,
				"property that allows getting and setting of multiple properties",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"best type",
				keyAEBestType,
				typeType,
				"the most specific type of this object",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"name",
				pName,
				typeText,
				"the name of this window",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				"index",
				pIndex,
				typeLongInteger,
				"the number of this window, counting from the frontmost",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				"ID",
				pID,
				typeLongInteger,
				"a unique ID for this document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// "position" is not used this way in the built-in dictionary,
				// but it is in the Finder dictionary.  It�s a very useful
				// property, especially when dealing with windows that are
				// not resizable.
				
				"position",
				kAESetPosition,
				typeQDPoint,
				"the upper left position of the window",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// Note that bounds is read/write, although if the window is
				// not resizable you must ensure that the new height and width
				// match the old height and width.
				
				"bounds",
				keyAEBounds,
				typeQDRectangle,
				"the boundary rectangle for the window",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"closeable",
				pHasCloseBox,
				typeBoolean,
				"Does the window have a close box?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"titled",
				pHasTitleBar,
				typeBoolean,
				"Does the window have a title bar?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"floating",
				pIsFloating,
				typeBoolean,
				"Does the window float?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"modal",
				pIsModal,
				typeBoolean,
				"Is the window modal?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"resizable",
				pIsResizable,
				typeBoolean,
				"Is the window resizable?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"zoomable",
				pIsZoomable,
				typeBoolean,
				"Is the window zoomable?",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// "zoomed" is marked as a read/write property, but it�s
				// only read/write for windows that are resizable.  There�s
				// no obvious way to express this concept in the 'aete'.
				
				"zoomed",
				pIsZoomed,
				typeBoolean,
				"Is the window zoomed?",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"visible",
				pVisible,
				typeBoolean,
				"Is the window visible?",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// After discussions with AppleScript engineering, the consensus
				// is that windows should have a frontmost property that is read/write,
				// to allow clients to select the window easily.
				
				"frontmost",
				pIsFrontProcess,
				typeBoolean,
				"Is this the frontmost window?",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
			},

			// elements of cWindow

			{
			},
		
		// windows class
		
			"windows",
			cWindow,
			"",
			
			// properties of cWindows
			
			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},

			// elements of cWindows

			{
			},

		// insertion point class (not supported because I don�t think I need it)
				
		},					// end classes

		{					// comparison operators
		},					// end comparison operators

		{					// enumerations
			enumSaveOptions,
			{
				"yes",
				kAEYes,
				"Save objects now",

				"no",
				kAENo,
				"Do not save objects",

				"ask",
				kAEAsk,
				"Ask the user whether to save"
			},
		},					// end enumerations

		// *** TestMOSL Suite ***

		"TestMOSL Events",
		"Terms for the TestMOSL application.",
		kMyAESuite,
		1,
		1,

		{					// events
		},					// end events

		{					// classes

		// node window class
		
			"node window",
			cNodeWindow,
			"A window containing nodes",

			// properties of cNodeWindow

			{
				"<Inheritance>",
				pInherits,
				cWindow,
				"inherits properties from the window class",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				// There�s no standard property that maps from a display window
				// to the document that controls it, so I simply reuse the
				// AppleScript parent property.  Still not super-sure that this
				// is the right thing to do, but it works and no one has suggested
				// a better solution yet.
				
				"parent",
				pASParent,
				cDocument,
				"parent document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				// selection (not yet implement)
			},

			// elements of cNodeWindow

			{
				cNode,
				{
					formAbsolutePosition,
					formName,
					formRange,
					formTest,
				},
			},

		// node windows class
		
			"node windows",
			cNodeWindow,
			"",
			
			// properties of cNodeWindows
			
			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},

			// elements cNodeWindows

			{
			},

		// about window class
		
			"about window",
			cAboutWindow,
			"A window displaying information about the application",

			// properties of cAboutWindow

			{
				"<Inheritance>",
				pInherits,
				cWindow,
				"inherits properties from the window class",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"credits",
				pMyCredits,
				typeText,
				"application credits",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
			},

			// elements of cMyWindowWindow

			{
			},

		// about windows class
		
			"about windows",
			cAboutWindow,
			"",
			
			// properties of cAboutWindows
			
			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},

			// elements of cAboutWindows

			{
			},

		// node class
		
			"node",
			cNode,
			"Made up information",

			// properties of cNode

			{
				"properties",
				pProperties,
				typeAERecord,
				"property that allows getting and setting of multiple properties",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"best type",
				keyAEBestType,
				typeType,
				"the most specific type of this object",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,

				"parent",
				pASParent,
				typeObjectSpecifier,
				"parent item or document",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				"name",
				pName,
				typeText,
				"name of this item",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
				
				"index",
				pIndex,
				typeLongInteger,
				"the index of this item within the parent",
				reserved,
				singleItem, notEnumerated, readWrite, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, singular,
			},

			// elements of cNode

			{
				cNode,
				{
					formAbsolutePosition,
					formName,
					formRange,
					formTest,
				},
			},

		// nodes class
		
			"nodes",
			cNode,
			"",
			
			// properties of cNodes
			
			{
				"",
				kAESpecialClassProperties,
				typeType,
				"",
				reserved,
				singleItem, notEnumerated, readOnly, enumsAreConstants, enumListCanRepeat, propertyIsValue,
				reserved, reserved, reserved, reserved, reserved,
				noApostrophe, notFeminine, notMasculine, plural
			},

			// elements of cNodes

			{
			},
		},					// end classes

		{					// comparison operators
		},					// end comparison operators

		{					// enumerations
		},					// end enumerations

	},						// end suites
};
