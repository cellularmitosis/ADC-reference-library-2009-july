/*

File: FCP_AppleEvents.h

Abstract:   Event definitions for Apple Event commands
			supported by Final Cut Pro.

Version: 1.1

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006-7 Apple Computer, Inc., All Rights Reserved

*/ 



/*!
 @header FCP Supported External Apple Events
 This header defines the AppleEvent Event class, Event IDs, and event parameters for all of the externally available
 events that FCP supports.
 @copyright Apple Computer, Inc.
 @updated 2007-10-30
*/

 /*!
 @defined kFCPEventClass
 @discussion Defines the main event class supported by Final Cut Pro. All of the different events IDs that can be sent to FCP use this event class
 */
 #define kFCPEventClass					'KeyG'

 /*!
 @discussion The following #define's give definition to each of the Apple Events IDs that FCP supports. Each of these event IDs uses the main kFCPEventClass event type defined above.
All the parameter values are defined below.
 */

/*!
@defined kFCPOpenProjectFile
@discussion This event allows the caller to make FCP open the specified project file. If that project file is already open, then it is brought to the foreground inside of FCP.
@param kFCPProjectFileKey Which project file to open.
*/
#define kFCPOpenProjectFile				'ofcP'	// the Open/Foreground a project file command
#define kAEFCPOpenProjectFile			kFCPOpenProjectFile //deprecated name

/*!
@defined kFCPSaveAndCloseProjectFile
@discussion Closes the specified project file, either saving any changes, or discarding them. If the project is not open then nothing happens.
If no project file parameter is sent, then the top-most open project will be closed.
@param kFCPProjectFileKey Which project file to close.
@param kFCPProjectFileCloseFlagsKey An optional flag which controls what behaviour is taken on the close action. The default is kFCPSaveAndCloseProject, save & close.
*/
#define kFCPSaveAndCloseProjectFile		'cfcP'	// the Close a project file command
#define kAEFCPSaveAndCloseProjectFile	kFCPSaveAndCloseProjectFile //deprecated name

/*!
@defined kFCPUpdateMediaFile
@discussion Allows an external application to request FCP to examine the file modification time and perform a silent reconnect/update if necessary. 
@param kFCPMediaFileKey Which media file to examine/update.
*/
#define	kFCPUpdateMediaFile				'udfF'	// the update/reconnect a media file command

/*!
@defined kFCPGetAllEffectsXML
@discussion Allows an application to request the XML representation of an all the installed effects. 
@param kFCPXMLDataVersion An optional value controlling which version of the XML spec to use. The default is the current version.
@result kFCPXMLDataKey	The XML data, represented as a UTF-8 string.
*/
#define	kFCPGetAllEffectsXML			'eFXX'	// the XML Export command

/*!
@defined kFCPGetAllOpenProjects
@discussion Returns a list of open project files as FSRefs.
@result  kFCPOpenProjectList The returned AEList of open FCP projects. The elements of the list (kFCPOneOpenProjectFile) are FSRefs
*/
#define kFCPGetAllOpenProjects			'fcLP'	// the Get List of Open Projects command

/*!
@defined kFCPGetDocumentXML
@discussion Allows an application to request the XML representation of an FCP Project. If the project is not open then it is first opened.
The project is always brought to the foreground inside of FCP. If no project file is supplied, then no XML will be returned.
@param kFCPProjectFileKey Which project file to operate on.
@param kFCPXMLDataVersion An optional value controlling which version of the XML spec to use. The default is the current version.
@result kFCPXMLDataKey	The XML data, represented as a UTF-8 string.
*/
#define	kFCPGetDocumentXML				'eXML'	// the XML Export command
#define	kKGAEGetDocumentXML				kFCPGetDocumentXML //deprecated name

/*!
@defined kFCPImportXMLToDocument
@discussion External applications can now feed XML directly into Final Cut Pro without requiring the user to import an XML file saved on disk. 
You can specify a specific project file to import the data into, or if no file is supplied, the XML will be imported into a new, untitled project.
Using the new features of FCP's XML language, you can control the results of the import and whether items in the project are replaced by the imported items,
or the new items are simply appended to the project.
@param kFCPProjectFileKey Which project file to operate on. If missing then create new untitled project otherwise import to this project.
@param kFCPXMLDataKey Actual XML text to import, represented as a UTF-8 string.
*/
#define	kFCPImportXMLToDocument			'iXML'	// the XML Import command
#define	kAEImportXMLToDocument			kFCPImportXMLToDocument //deprecated name

/*!
@defined kFCPOpenItemInProject
@discussion This event specifies that a particular item should be opened into the Viewer inside of FCP. The item to be opened is specified by it's UUID.
The UUID is encoded in a kFCPItemUUID parameter, discussed below.
@param kFCPProjectFileKey Which project file to operate on.
@param kFCPItemUUID UUID of the item to open into the Viewer.
*/
#define kFCPOpenItemInProject			'fcOI'	// Open the item specified in the passed in UUID

/*!
@defined kFCPSelectItemInBrowser
@discussion This event specifies that a particular item should be highlighted in the Browser window inside of FCP. Only one item can be selected at a time.
The item to be highlighted is specified by it's UUID. The UUID is encoded in a kFCPItemUUID parameter, discussed below.
You can optionally add the kFCPItemsToSelectList parameter, which is a AEDescList list of kFCPSelectItemInBrowser items. Use this if you want more than one item to be selected.
@param kFCPProjectFileKey Which project file to operate on.
@param kFCPItemUUID UUID of the item to select in the Browser.
@param kFCPItemsToSelectList (optional) list of UUIDs to select in the Browser.
*/
#define kFCPSelectItemInBrowser			'fcSI'	// Select the item specified in the passed in UUID

/*!
@defined kFCPGetItemXML
@discussion Allows an application to request the XML representation of a single item in an FCP Projecct by UUID.
If the project is not open then it is first opened.
The project is always brought to the foreground inside of FCP. If no project file or UUID is supplied, then no XML will be returned.
@param kFCPProjectFileKey Which project file to operate on.
@param kFCPItemUUID UUID of the item to open into the Viewer.
@param kFCPXMLDataVersion An optional value controlling which version of the XML spec to use. The default is the current version.
@result kFCPXMLDataKey	The XML data, represented as a UTF-8 string.
*/
#define	kFCPGetItemXML					'gXML'	// the XML Export command

/*!
@defined kFCPFindItemsInProject
@discussion Using the kFCPFindItemsInProject event, you can ask FCP to initiate a search for items in the Browser and have the results brought up in a new Find Results window to the user. 
This can be used to bring multiple items to the attention of the user at once.
The kFCPFindLogicMode parameter controls whether the criteria entries are combined with AND (all entries must apply) or OR (only one entry need apply).
The kFCPFindParameters list contains one or more entries that are the search criteria. You can use multiple entries to very precisely control what results are obtained.
For example, one entry can specify that the Name column of the items must start with a string 'Camera' and the next entry specifies that the Notes column must not contain the string 'bad take'.
Each criteria entry is made up of 4 elements: the string to search for, the way to match the string, the column to search, and whether to use matching or non-matching items.
Note that each criteria entry is an AERecord, so the list consists of records, each of which contains several parameters.
@param kFCPProjectFileKey Which project file to operate on.
@param kFCPFindLogicMode A flag specifying which logical operator to use, logical AND or logical OR. The constants for this are defined below.
@param kFCPFindParameters An AEDescList list of criteria AERecords. The list of criteria records has the following form:
						 <index>				a record holding one set of criteria
									kFCPFindMatchMode		which way to match, specified by one of the FCPFindMatchType values.
									kFCPFindOmitCriteria	A boolean that will reverse the sense of the criteria (i.e non-matches rather than matches are returned)	
									kFCPFindColumnName		A UTF-8 string that (optinally) restricts the criteria to a single FCP Browser column.
														If missing or unknown then any column otherwise specific column. The available columns are specified using
														the XML tags for clip properties. The full list is given below.
									kFCPFindSearchString	The text string to search for, specified as a UTF-8 string.
@discussion
 * These are the valid values for kFCPFindColumnName.
 * Each is the name of an XML tag for a clip property.
 * FCP will map them to browser column names.
 * The values are:
 *
 *		name
 *		duration
 *		in
 *		out
 *		start
 *		end
 *		scene
 *		shottake
 *		lognote
 *		good
 *		label
 *		label2
 *		mastercomment1
 *		mastercomment2
 *		mastercomment3
 *		mastercomment4
 *		clipcommenta
 *		clipcommentb
*/
#define kFCPFindItemsInProject			'ffcP'	// Find some item(s) in the specified FCP Project
#define kAEFindItemsInProject			kFCPFindItemsInProject //deprecated name

//*******************************************************************************************************************************
// These values define the parameter keys used to send data back and forth
//*******************************************************************************************************************************

/*!
@defined kFCPXMLDataKey
@discussion An XML data stream, in type typeUTF8Text
*/
#define kFCPXMLDataKey					'xmlD'
#define kXMLDataKey						kFCPXMLDataKey //deprecated name

/*!
@defined kFCPXMLDataVersion
@discussion The version of the XML being passed. (-1, 1.0, 2.0, 3.0, etc.), in type typeFloat
*/
#define kFCPXMLDataVersion				'xmlV'
#define kXMLDataVersion					kFCPXMLDataVersion //deprecated name

/*!
@defined kFCPProjectFileKey
@discussion The FCP Project file to export the XML from, in type typeFSRef
*/
#define kFCPProjectFileKey				'fcpP'

/*!
@defined kFCPItemUUID
@discussion The UUID of an FCP Project item, in type typeUTF8Text
*/
#define kFCPItemUUID					'fcIU'

/*!
@defined kFCPMediaFileKey
@discussion The media file to process, in type typeFSRef
*/
#define kFCPMediaFileKey				'fcpM'

/*!
@defined kFCPOpenProjectList
@discussion The returned AEList of open FCP projects. The elements of the list are FSRefs
// the list of returned Open Projects
@param kFCPOneOpenProjectFile The elements of the AEList of open FCP Projects. They are of type typeFSRef.
*/
#define kFCPOpenProjectList				'fcOP' 

/*!
@defined kFCPOneOpenProjectFile
@discussion The elements of the AEList of open FCP Projects. They are of type typeFSRef.
a single path to a project file. These are the items in the above list
*/
#define kFCPOneOpenProjectFile			'fcPf'

/*!
@defined kFCPItemsToSelectList
@discussion kFCPItemsToSelectList (optional) list of UUIDs to select in the Browser.
*/
#define kFCPItemsToSelectList			'fcSL'	// the list of items to be selected

//*******************************************************************************************************************************	
// These values are (optional) flags to control the behaviour of the events defined above
//*******************************************************************************************************************************

// Flags for kFCPSaveAndCloseProjectFile Event
/*!
@defined kFCPProjectFileCloseFlagsKey
@discussion Controls the behaviour of the close command, e.g. close & save, close & discard, in type typeSInt32
*/
#define kFCPProjectFileCloseFlagsKey	'fcCF'
#define kProjectFileCloseFlagsKey		kFCPProjectFileCloseFlagsKey //deprecated name

/*!
 @typedef FCPProjectCloseFlag
 @abstract used as part of the kFCPSaveAndCloseProjectFile event.
 @discussion This enum defines the optional flags to control what behaviour is performed on a kFCPSaveAndCloseProjectFile event.
 @constant kFCPSaveAndCloseProject Attempt to save the project file before closing. If no kFCPProjectFileKey was supplied as part of the event, then the Save UI WILL be shown to the user
								   and the event will not return until the user dismisses the dialog.
 @constant kFCPDiscardAndCloseProject Specifying this flag will cause the project to be closed without saving any outstanding changes, discarding those changes.
*/
typedef enum {
	kFCPSaveAndCloseProject = 0,
	kFCPDiscardAndCloseProject = 1
} FCPProjectCloseFlag;
	
// These values are defines for the data needed for the kFCPFindItemsInProject event
// They define what data we are searching for in the FCP Browser
/*!
@defined kFCPFindParameters
@discussion An AEDescList containing the parameter list of criteria records, in type typeAEList.
*/
#define	kFCPFindParameters				'fndP'
#define kFindParameters					kFCPFindParameters //deprecated name

/*!
@defined kFCPFindSearchString
@discussion The string to search for, in type typeUTF8Text.
*/
#define kFCPFindSearchString			'fndS'
#define kFindSearchString				kFCPFindSearchString //deprecated name

/*!
@defined kFCPFindMatchMode
@discussion The matching mode to use, in type typeSInt32. The values for this parameter are found in the FCPFindMatchType enum.
*/
#define kFCPFindMatchMode				'fndM'
#define kFindMatchMode					kFCPFindMatchMode //deprecated name

/*!
@defined kFCPFindOmitCriteria
@discussion A boolean that will reverse the sense of the criteria (i.e non-matches rather than matches are returned), in type typeSInt32.
*/
#define kFCPFindOmitCriteria			'fndO'
#define kFindOmitCriteria				kFCPFindOmitCriteria //deprecated name

/*!
@defined kFCPFindColumnName
@discussion The (optional) column to search for the string in, in type typeUTF8Text. Possible values for this string any of the XML tags for clip properties.
*/
#define kFCPFindColumnName				'fndC'
#define kFindColumnName					kFCPFindColumnName //deprecated name

/*!
@defined kFCPFindLogicMode
@discussion The logic to apply to multiple search terms, AND or OR, in type typeSInt32. The values for this parameter are found in the FCPFindLogicType enum.
*/
#define kFCPFindLogicMode				'fndL'
#define kFindLogicMode					kFCPFindLogicMode //deprecated name

/*!
 @typedef FCPFindMatchType
 @abstract Used as part of the kFCPFindItemsInProject event.
 @discussion This enum contains the defines for the type of matching behaviour used for a criteria record as part of the kFCPFindItemsInProject event.
 @constant kFCPFindMatchStartsWith Searches for strings that begin with the string given in the kFCPFindSearchString parameter of this criteria AERecord.
 @constant kFCPFindMatchContains Searches for strings that contain with the string given in the kFCPFindSearchString parameter of this criteria AERecord.
 @constant kFCPFindMatchEquals Searches for strings that exactly match the string given in the kFCPFindSearchString parameter of this criteria AERecord.
 @constant kFCPFindMatchEndsWith Searches for strings that terminate with the string given in the kFCPFindSearchString parameter of this criteria AERecord.
 @constant kFCPFindMatchLessThan Searches for strings that compare less than the string given in the kFCPFindSearchString parameter of this criteria AERecord.
 @constant kFCPFindMatchGreaterThan Searches for strings that greater than the string given in the kFCPFindSearchString parameter of this criteria AERecord.
*/
typedef enum {
	kFCPFindMatchStartsWith =		(1 << 0),
	kFCPFindMatchContains =			(1 << 1),
	kFCPFindMatchEquals =			(1 << 2),
	kFCPFindMatchEndsWith =			(1 << 3),
	kFCPFindMatchLessThan =			(1 << 4),
	kFCPFindMatchGreaterThan =		(1 << 5)
} FCPFindMatchType;
	
/*!
 @typedef FCPFindLogicType
 @abstract Used as part of the kFCPFindItemsInProject event.
 @discussion This enum defines the vvalues that control what logical operator is applied between the criteria records given in the kFCPFindParameters list.
 @constant kFCPFindLogicAnd Use logical AND.
 @constant kFCPFindLogicOr Use logical OR.
*/
typedef enum {
	kFCPFindLogicAnd = 0,
	kFCPFindLogicOr = 1
} FCPFindLogicType;
#define kFCPLogicAnd	kFCPFindLogicAnd //deprecated name
#define kFCPLogicOr		kFCPFindLogicOr //deprecated name

