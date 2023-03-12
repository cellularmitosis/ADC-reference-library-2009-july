/*
	File:		SlideShowImport.c
	
	Description: 	Slide Show Importer component routines.

	Author:		Apple Developer Support, original code by Jim Batson of QuickTime engineering

	Copyright: 	� Copyright 2001-2002 Apple Computer, Inc. All rights reserved.
	
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
	
	   <2>		3/22/02		SRK				Carbonized/Win32
	   <1>	 	4/01/01		Jim Batson		first file
*/

#if __APPLE_CC__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#else
    #include <ImageCodec.h>
    #include <TextUtils.h>
    #include <string.h>
    #include <Files.h>
    #include <Movies.h>
    #include <MediaHandlers.h>
#endif


#include "SlideShowImport.h"
#include "SlideShowImportConstants.h"
#include "SlideShowImportVersion.h"

// ---------------------------------------------------------------------------------
//		� ComponentDispatchHelper Definitions �
// ---------------------------------------------------------------------------------

#define MOVIEIMPORT_BASENAME()		SlideShowImport
#define MOVIEIMPORT_GLOBALS() 		SlideShowImportGlobals storage

#define CALLCOMPONENT_BASENAME()	MOVIEIMPORT_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		MOVIEIMPORT_GLOBALS()

#define COMPONENT_UPP_PREFIX()		uppMovieImport
#define COMPONENT_DISPATCH_FILE		"SlideShowImportDispatch.h"
#define COMPONENT_SELECT_PREFIX()  	kMovieImport

#include <Components.k.h>				// StdComponent's .k.h

#if __APPLE_CC__
    #include <QuickTime/QuickTimeComponents.k.h>
    #include <QuickTime/ComponentDispatchHelper.c>
#else
    #include <QuickTimeComponents.k.h>
    #include <ComponentDispatchHelper.c>	// make our dispatcher and cando
#endif

// ---------------------------------------------------------------------------------
//		� SlideShowImportOpen �
//
// Handle the Standard Component Manager Open Request
//
// Set up the components global data storage
// ---------------------------------------------------------------------------------
pascal ComponentResult SlideShowImportOpen(SlideShowImportGlobals store, ComponentInstance self)
{
	ComponentResult err;
	
	// Allocate and initialize all globals to 0 by calling NewPtrClear.
	store = (SlideShowImportGlobals)NewPtrClear(sizeof(SlideShowImportGlobalsRecord));
	err = MemError();
	if (err) goto bail;

	store->self = self;
	
	SetComponentInstanceStorage(self, (Handle)store);

bail:	
	return err;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportClose �
//
// Handle the Standard Component Manager Close Request
//
// Clean up the components global data storage
// ---------------------------------------------------------------------------------
pascal ComponentResult SlideShowImportClose(SlideShowImportGlobals store, ComponentInstance self)
{
#pragma unused(self)

	if (store) 
	{
		DisposePtr((Ptr)store);
	}
		
	return noErr;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportVersion �
//
// Handle the Standard Component Manager Version Request
//
// Return the components version information
// ---------------------------------------------------------------------------------
pascal long SlideShowImportVersion(SlideShowImportGlobals store)
{
#pragma unused(store)
	return kSlideShowImportVersion;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportFile �
//
// Import data from a file
//
// The file in this case will be our XML file, which we will use to construct our movie
// ---------------------------------------------------------------------------------
pascal ComponentResult SlideShowImportFile(SlideShowImportGlobals store,
											const FSSpec 	*theFile,
											Movie 			theMovie,
											Track 			targetTrack,
											Track 			*usedTrack,
											TimeValue 		atTime,
											TimeValue 		*addedDuration,
											long 			inFlags,
											long 			*outFlags)
{
	ComponentResult	err;
	AliasHandle		alias;

	err = QTNewAlias(theFile, &alias, true);
	if (err) goto bail;
	
	err = MovieImportDataRef(store->self,
							(Handle)alias,
							rAliasType,
							theMovie,
							targetTrack,
							usedTrack,
							atTime,
							addedDuration,
							inFlags,
							outFlags);
							
	DisposeHandle((Handle)alias);

bail:
	return err;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportDataRef �
//
// Import data from a data reference
//
// The data reference in this case will be our XML data, which we will use to
// construct our movie
// ---------------------------------------------------------------------------------
pascal ComponentResult SlideShowImportDataRef(	SlideShowImportGlobals 	store,
												Handle 					dataRef,
												OSType 					dataRefType,
												Movie 					theMovie,
												Track 					targetTrack,
												Track 					*usedTrack,
												TimeValue 				atTime,
												TimeValue 				*addedDuration,
												long 					inFlags,
												long 					*outFlags
												)
{
#pragma unused(store, usedTrack, atTime, addedDuration, inFlags)

	ComponentResult	err = noErr;
	SlideShowFile	theFile = nil;

	// target tracks not supported, only can import into an entire movie
	if (dataRef == nil || theMovie == nil || targetTrack != nil)
	{
		err = paramErr;
		goto bail;
	}

	if (outFlags)
	{
		*outFlags = 0;
	}
	
	// initialize
	theFile = (SlideShowFile)NewPtrClear(sizeof(SlideShowFileRecord));
	err = MemError();
	if (err) goto bail;
	
	// copy XML source data ref and other information needed during import
	err = HandToHand(&dataRef);
	if (err) goto bail;
	theFile->dataRef = dataRef;
	
	theFile->dataRefType = dataRefType;
	theFile->theMovie = theMovie;

	// parse the SlideShow file
	err	= ReadSlideShowFile(theFile);
	if (err) goto bail;

	// build a movie
	err	= BuildMovie(theFile);
	if (err) goto bail;

bail:
	if (theFile)
		DisposeSlideShowFile(theFile);
		
	return err;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportGetMIMETypeList �
//
// Returns a list of MIME types supported by the movie import component.
//
// ---------------------------------------------------------------------------------
pascal ComponentResult SlideShowImportGetMIMETypeList(SlideShowImportGlobals store, QTAtomContainer *mimeInfo)
{
	ComponentResult err = noErr;

	if (mimeInfo == nil)
	{
		err = paramErr;
	}
	else
	{
		err	= GetComponentResource((Component)store->self, kMimeInfoMimeTypeTag, kSlideShowImportThingRes, (Handle *)mimeInfo);
	}
    
    return err;
}

// ---------------------------------------------------------------------------------
//		� ReadSlideShowFile �
//
// Parse the XML file and get all the slideshow element attributes
// ---------------------------------------------------------------------------------
ComponentResult ReadSlideShowFile(SlideShowFile theFile)
{
	ComponentResult 	err 		= noErr;
	ComponentInstance	xmlParser 	= nil;
	XMLDoc				document 	= nil;
	long				index;
	short				elementType;

	// create a SlideShow parser	
	xmlParser = CreateXMLParserForSlideShow();

	if (xmlParser == nil)
	{
		err = -1;
		goto bail;
	}

	// do the parse
	err = XMLParseDataRef(xmlParser, theFile->dataRef, theFile->dataRefType, 0, &document);

	if (document->rootElement.identifier == element_slideshow && document->rootElement.contents)
	{
		// get the slideshow element attributes
		err = GetRootElementAttributes(document->rootElement.attributes, theFile);
		
		// scan through the contents looking for images 
		for (index = 0; document->rootElement.contents[index].kind != xmlContentTypeInvalid; index++)
		{
			if (document->rootElement.contents[index].kind == xmlContentTypeElement)
			{
				elementType = document->rootElement.contents[index].actualContent.element.identifier;
				
				switch (elementType)
				{
					case element_image:
					{
						// parse the image
						err = ParseAndSaveImage(&(document->rootElement.contents[index].actualContent.element), theFile);
						break;
					}
					case element_effect:
					{
						err = ParseEffectElement(&(document->rootElement.contents[index].actualContent.element), theFile);
						break;
					}
						break;
					case element_audio:
					{
						err = ParseAudioElement(&(document->rootElement.contents[index].actualContent.element), theFile);						
						break;
					}
				}
			}
		}
	}
	else
	{
		err = invalidMovie;
	}
	
bail:
	if (document)
	{
		XMLParseDisposeXMLDoc(xmlParser, document);
	}
	
	if (xmlParser)
	{
		CloseComponent(xmlParser);
	}
	
	return err;
}

// ---------------------------------------------------------------------------------
//		� CreateXMLParserForSlideShow �
//
// Locate the XML parser component
// Add the specified elements to the parser
// ---------------------------------------------------------------------------------
ComponentInstance CreateXMLParserForSlideShow()
{
	ComponentInstance	xmlParser = OpenDefaultComponent(xmlParseComponentType, xmlParseComponentSubType);
	UInt32				elementID, attributeID;

	if (xmlParser)
	{
		XML_ADD_ELEMENT(slideshow);
			XML_ADD_ATTRIBUTE_AND_VALUE(width, attributeValueKindInteger, nil);
			XML_ADD_ATTRIBUTE_AND_VALUE(height, attributeValueKindInteger, nil);

		XML_ADD_ELEMENT(image);
			XML_ADD_ATTRIBUTE(src);
			XML_ADD_ATTRIBUTE_AND_VALUE(dur, attributeValueKindInteger, nil);

		XML_ADD_ELEMENT(effect);
			XML_ADD_ATTRIBUTE(type);
			XML_ADD_ATTRIBUTE_AND_VALUE(dur, attributeValueKindInteger, nil);

		XML_ADD_ELEMENT(audio);
			XML_ADD_ATTRIBUTE(src);
	}

	return xmlParser;
}

// ---------------------------------------------------------------------------------
//		� GetRootElementAttributes �
//
// Get the slideshow movie width and height attributes
// ---------------------------------------------------------------------------------
ComponentResult GetRootElementAttributes(XMLAttribute *attributes, SlideShowFile theFile)
{
	ComponentResult err = noErr;
	
	// save root attribute values (both required)
	err = GetIntegerAttribute(attributes, attr_width, &theFile->movieWidth);
	err = GetIntegerAttribute(attributes, attr_height, &theFile->movieHeight);

    return err;
}

// ---------------------------------------------------------------------------------
//		� AddSlideShowElement �
//
// Add a SlideShowElementRecord record to our queue of elements
// ---------------------------------------------------------------------------------
void AddSlideShowElement(SlideShowFile theFile, SlideShowElement theElement)
{
	if (theFile->elements == nil)
	{
		theFile->elements = theElement;
	}
	else
	{
		SlideShowElement temp = theFile->elements;
		
		while (temp && temp->nextElement)
			temp = (SlideShowElement)(temp->nextElement);
		
		temp->nextElement = theElement;
	}
}

// ---------------------------------------------------------------------------------
//		� ParseAndSaveImage �
//
//  Build a SlideShowElementRecord record for the current image and add it to our list
// ---------------------------------------------------------------------------------
ComponentResult	ParseAndSaveImage(XMLElement *element, SlideShowFile theFile)
{
	ComponentResult err = noErr;
	XMLAttributePtr	attributes = element->attributes;
	SlideShowElement theTempImage = nil;
	
	// allocate and initialize using NewPtrClear
	theTempImage = (SlideShowElement)NewPtrClear(sizeof(SlideShowElementRecord));
	err = MemError();
	if (err) goto bail;
	
	theTempImage->elementKind = kSlideShowImageKind;
	
	// save image values
	err = GetIntegerAttribute(attributes, attr_dur, &(theTempImage->image.duration));
	if (err) goto bail;
	err = GetStringAttribute(attributes, attr_src, &(theTempImage->image.src));
	if (err) goto bail;

	// add the element to our list
	AddSlideShowElement(theFile, theTempImage);
	
	theFile->numImages++;
	
bail:

	if (err && theTempImage)
	{
		DisposeAllElements(theFile);
	}
	
	return err;
}


// ---------------------------------------------------------------------------------
//		� ParseEffectElement �
//
//  Build a SlideShowElementRecord record for the current effect and add it to our list
// ---------------------------------------------------------------------------------
ComponentResult	ParseEffectElement(XMLElement *element, SlideShowFile theFile)
{
	ComponentResult 	err 			= noErr;
	XMLAttributePtr		attributes 		= element->attributes;
	SlideShowElement 	theTempEffect 	= nil;
	
	// allocate and initialize using NewPtrClear
	theTempEffect = (SlideShowElement)NewPtrClear(sizeof(SlideShowElementRecord));
	err = MemError();
	if (err) goto bail;
	
	// save image values
	theTempEffect->elementKind = kSlideShowEffectKind;
	
	err = GetIntegerAttribute(attributes, attr_dur, &(theTempEffect->effect.duration));
	if (err) goto bail;
	err = GetStringAttribute(attributes, attr_type, &(theTempEffect->effect.type));
	if (err) goto bail;

	// add the element to our list
	AddSlideShowElement(theFile, theTempEffect);
	
	theFile->numEffects++;
	
bail:
	if (err && theTempEffect)
	{
		DisposeAllElements(theFile);
	}
	
	return err;
}
	

// ---------------------------------------------------------------------------------
//		� ParseAudioElement �
//
//  Build a SlideShowElementRecord record for the current audio element and add it to
//  our list
// ---------------------------------------------------------------------------------
ComponentResult	ParseAudioElement(XMLElement *element, SlideShowFile theFile)
{
	ComponentResult 	err 			= noErr;
	XMLAttributePtr		attributes 		= element->attributes;
	SlideShowElement	theTempEffect 	= nil;
	
	// allocate and initialize using NewPtrClear
	theTempEffect = (SlideShowElement)NewPtrClear(sizeof(SlideShowElementRecord));
	err = MemError();
	if (err) goto bail;
	
	// save image values
	theTempEffect->elementKind = kSlideShowAudioKind;

	err = GetStringAttribute(attributes, attr_src, &(theTempEffect->movie.src));
	if (err) goto bail;

	// add the element to our list
	AddSlideShowElement(theFile, theTempEffect);
	
	theFile->numAudioTracks++;
	
bail:
	if (err && theTempEffect)
	{
		DisposeAllElements(theFile);
	}
	
	return err;
}



// ---------------------------------------------------------------------------------
//		� BuildMovie �
//
//	Steps
//	- Add all the images to a single video track
//	- Determine where effects will occur
//	- Effects types are:
//
//		dissolve
//		wipe - direction
//		heart or star-wipe
//		
// - add audio element
// ---------------------------------------------------------------------------------
ComponentResult BuildMovie(SlideShowFile theFile)
{
	ComponentResult		err 		= noErr;
	SlideShowElement 	theElement 	= nil;
	TimeValue 			theTime 	= 0;
	Fixed 				maxWidth 	= 0;
	Fixed 				maxHeight 	= 0;
				
		
		// add the images to a single track -- theFile->theImageTrack
		for (theElement = theFile->elements; theElement; theElement = (SlideShowElement)(theElement->nextElement))
		{
			
			switch (theElement->elementKind)
			{
				case kSlideShowImageKind:
					if (theFile->thePendingEffect)
					{
						err = CreateEffectTrack(theFile, theFile->thePreviousSource, &(theElement->image), theFile->thePendingEffect, &theTime);
						theFile->thePendingEffect = nil;
						theFile->thePreviousSource = &(theElement->image);
						if (err) goto bail;
					}
					else
					{
						// add the image to the movie
						err = AddImageToMovie(theFile, &theElement->image, &theTime);
						theFile->thePreviousSource = &(theElement->image);
						if (err) goto bail;
							
						if (theElement->image.discoveredWidth > maxWidth)
							maxWidth = theElement->image.discoveredWidth;
						if (theElement->image.discoveredWidth > maxHeight)
							maxHeight = theElement->image.discoveredHeight;
					}
					break;
				
				case kSlideShowEffectKind:
				{
					
					// ����add the second image plus the effect tracks (2 source + effect)
					
					theFile->thePendingEffect = &(theElement->effect);
					
				}
				break;
				
				case kSlideShowAudioKind:
					break;
			}
		}

		if ((theFile->movieWidth == 0) || (theFile->movieHeight == 0)) 
		{
			Fixed 	oldWidth, oldHeight;
			Track 	nextVideoTrack 	= nil;
			long 	trackIndex 		= 1;
			
			GetTrackDimensions(theFile->theImageTrack, &oldWidth, &oldHeight);
			SetTrackDimensions(theFile->theImageTrack, (0 == theFile->movieWidth) ? maxWidth : oldWidth , (0 == theFile->movieHeight) ? maxHeight : oldHeight);

			// spin over other video tracks and set their dimensions, too
			
			while (nil != (nextVideoTrack = GetMovieIndTrackType(theFile->theMovie, trackIndex++, VideoMediaType, movieTrackMediaType)))
			{
				if (nextVideoTrack != theFile->theImageTrack)
				{
					GetTrackDimensions(theFile->theImageTrack, &oldWidth, &oldHeight);
					SetTrackDimensions(nextVideoTrack, oldWidth, oldHeight);
				}
			}
		}
		
		
		// Now look for audio elements
		for (theElement = theFile->elements; theElement; theElement = (SlideShowElement)(theElement->nextElement))
		{			
			switch (theElement->elementKind)
			{
				case kSlideShowAudioKind:
				{
					TimeValue addedTime = 0;
					
					err = AddAudioToMovie(theFile, &theElement->movie, &addedTime );
					goto bail;
				}
				break;
			}
		}
		
bail:
	return err;
}




// ---------------------------------------------------------------------------------
//		� AddImageToMovie �
//
// Locate the first track in the movie and add the image to the track's media
// ---------------------------------------------------------------------------------
OSErr AddImageToMovie(SlideShowFile theFile, SlideShowImage theImage, TimeValue *theTime)
{
	ComponentResult 		err 			= noErr;
	ComponentInstance		ci 				= nil;
	Movie					refMovie;
	Track					refTrack;	//, theTrack = nil;
	Media					refMedia;	//, theMedia;
	SampleDescriptionHandle	desc 			= nil;
	long					dataOffset, size;
	OSType					mediaType;
	TimeValue				duration;
	OSType					dataRefType;
	Handle					dataRef 		= nil;
	Ptr						theURL 			= nil;
	short					newDataRefIndex = 0;
	TimeValue				theSampleTime;
	
	// copy the url
	theURL = NewPtr((theImage->src)[0] + 1);
	if (theURL == nil) goto bail;
	BlockMove(theImage->src + 1, theURL, (theImage->src)[0]);
	theURL[(theImage->src)[0]] = '\0';
	
	// get the data ref and try to open the movie
	err = CreateDataRefFromURL(theURL, theFile->dataRef, theFile->dataRefType, &dataRef, &dataRefType);
	err = NewMovieFromDataRef(&refMovie, newMovieDontResolveDataRefs, nil, dataRef, dataRefType);
	if (err || refMovie == nil) goto bail;

	// get the first track's media
	refTrack = GetMovieIndTrack(refMovie, 1);
	refMedia = GetTrackMedia(refTrack);

	// allocate sample description
	desc = (SampleDescriptionHandle)NewHandle(0);
	err = MemError();
	if (err) goto bail;

	// get the ref media sample reference
	err = GetMediaSampleReference(refMedia, &dataOffset, &size, 0, nil, nil, desc, nil, 1, nil, 0);
	GetMediaHandlerDescription(refMedia, &mediaType, nil, nil);
	if (err || mediaType != VideoMediaType) goto bail;

	theImage->discoveredWidth = FixRatio((**(ImageDescriptionHandle)desc).width,1);
	theImage->discoveredHeight = FixRatio((**(ImageDescriptionHandle)desc).height,1);
	
	if (nil == theFile->theImageTrack)
	{
		// create a new track
		if (theFile->movieWidth == 0)
			theFile->movieWidth = theImage->discoveredWidth >> 16;
		if (theFile->movieHeight == 0)
			theFile->movieHeight = theImage->discoveredHeight >> 16;
			
		theFile->theImageTrack = NewMovieTrack(theFile->theMovie, theFile->movieWidth ? (theFile->movieWidth << 16) : theImage->discoveredWidth, 
									theFile->movieHeight ? theFile->movieHeight << 16 : theImage->discoveredHeight, 0);
		if (theFile->theImageTrack == nil) goto bail;
		
		// create a track media
		theFile->theImageMedia = NewTrackMedia(theFile->theImageTrack, VideoMediaType, GetMovieTimeScale(theFile->theMovie), nil, 0);
		if (theFile->theImageMedia == nil) goto bail;
	}
	
	// add data ref to media for this image's file. This may return an already added data ref.
	err = AddMediaDataRef(theFile->theImageMedia, &newDataRefIndex, dataRef, dataRefType);
	if (err) goto bail;
	
	theImage->dataRefIndex = (**desc).dataRefIndex = newDataRefIndex;
	
	// add a media sample reference to our movie
	duration = max(1, theImage->duration * 600);


	// �� we DO NOT ever write to the track so this is OK
	// by not calling the BeginMediaEdits/EndMediaEdits pair
	// we are adding only references
//	err	= BeginMediaEdits(theMedia);
//	if (err) goto bail;
	err = AddMediaSampleReference(theFile->theImageMedia, dataOffset, size, duration, desc, 1, 0, &theSampleTime);
	if (err) goto bail;
	theImage->sampleTime = theSampleTime;
//	err = EndMediaEdits(theMedia);
//	if (err) goto bail;
//	err	= InsertMediaIntoTrack(theTrack, *theTime, 0, duration, FixDiv(Long2Fix(GetMediaDuration(theMedia)), Long2Fix(duration)));
	err	= InsertMediaIntoTrack(theFile->theImageTrack, *theTime, theSampleTime, duration, fixed1);
	if (err) goto bail;

	// increment our movie time
	*theTime += duration;

bail:
	if (dataRef)
	{
		DisposeHandle(dataRef);
	}
	
	if (desc)
	{
		DisposeHandle((Handle)desc);
	}
	
	if (ci)
	{
		CloseComponent(ci);
	}	
	
	return err;
}


// ---------------------------------------------------------------------------------
//		� AddAudioToMovie �
//
//  Open our audio source file as a movie, then add the audio track from this source
//  movie to our destination movie.
//
// ---------------------------------------------------------------------------------
OSErr AddAudioToMovie(SlideShowFile theFile, SlideShowMovie theMovie, TimeValue *theTime)
{
	ComponentResult 		err 			= noErr;
	ComponentInstance		ci 				= nil;
	Movie					refMovie;
	SampleDescriptionHandle	desc 			= nil;
	OSType					dataRefType;
	Handle					dataRef 		= nil;
	Ptr						theURL 			= nil;
	
	// copy the url
	theURL = NewPtr((theMovie->src)[0] + 1);
	if (theURL == nil) goto bail;
	BlockMove(theMovie->src + 1, theURL, (theMovie->src)[0]);
	theURL[(theMovie->src)[0]] = '\0';
	
	// get the data ref and try to open the movie
	err = CreateDataRefFromURL(theURL, theFile->dataRef, theFile->dataRefType, &dataRef, &dataRefType);
	err = NewMovieFromDataRef(&refMovie, newMovieDontResolveDataRefs, nil, dataRef, dataRefType);
	if (err || refMovie == nil) goto bail;

	// add the movie to the track
	if (theMovie->dontScaleImagesAndEffectsToThisMovie)
	{
		SetMovieSelection(refMovie, 0, GetMovieDuration(refMovie));
		AddMovieSelection(theFile->theMovie, refMovie);
	}
	else
	{
		ScaleMovieSegment(theFile->theMovie, 0, GetMovieDuration(theFile->theMovie), GetMovieDuration(refMovie));
		SetMovieSelection(refMovie, 0, GetMovieDuration(refMovie));
		AddMovieSelection(theFile->theMovie, refMovie);
	}
	
	// increment our movie time
	*theTime = 0;

bail:
	if (dataRef)
	{
		DisposeHandle(dataRef);
	}
	
	if (desc)
	{
		DisposeHandle((Handle)desc);
	}
	
	if (ci)
	{
		CloseComponent(ci);
	}	
	
	return err;
}


// ---------------------------------------------------------------------------------
//		� CreateDataRefFromURL �
//
//  Create a URL data reference
//
// ---------------------------------------------------------------------------------

OSErr CreateDataRefFromURL(char* theURL, Handle baseDRef, OSType baseDRefType, Handle *dref, OSType *drefType)
{
	ComponentInstance	ci 				= 0;
	Handle				absoluteDRef 	= NULL;
	OSErr				err				= noErr;

	*dref = NULL;
	
	err	= PtrToHand(theURL, dref, (SInt32)(strlen(theURL) + 1));
	if (err) goto Bail;

	*drefType = URLDataHandlerSubType;

	if (OpenAComponent(GetDataHandler(baseDRef, baseDRefType, kDataHCanRead), &ci) != noErr)
		goto Bail;

	if (DataHSetDataRefWithAnchor(ci, baseDRef, URLDataHandlerSubType, *dref) != noErr)
		goto Bail;

	if (DataHGetDataRefAsType(ci, baseDRefType, &absoluteDRef) != noErr)
		goto Bail;

	DisposeHandle(*dref);
	*dref = absoluteDRef;
	*drefType = baseDRefType;

Bail:
	if (NULL != ci)
	{
		CloseComponent(ci);
	}
	
	return	err;
}

// ---------------------------------------------------------------------------------
//		� DisposeSlideShowFile �
//
// Delete the slide show file object and all its elements
// ---------------------------------------------------------------------------------
void DisposeSlideShowFile(SlideShowFile theFile)
{
	// delete the entire element list
	DisposeAllElements(theFile);

	if (theFile->dataRef)
		DisposeHandle(theFile->dataRef);
		
	// delete the file object
	DisposePtr((Ptr)theFile);
}


// ---------------------------------------------------------------------------------
//		� DisposeElementFields �
//
// Dispose the image and effect element fields
// ---------------------------------------------------------------------------------
void DisposeElementFields(SlideShowElement element)
{
	if (element)
	{
		switch (element->elementKind)
		{
			case kSlideShowImageKind:
			{
				DisposePtr((Ptr)element->image.src);
				element->image.src = nil;
			}
			break;
			
			case kSlideShowEffectKind:
			{
				DisposePtr((Ptr)element->effect.type);
				element->effect.type = nil;
				
				if (element->effect.effectDescription)
					QTDisposeAtomContainer(element->effect.effectDescription);
				element->effect.effectDescription = nil;
			}
			break;
		}
	}
}
	

// ---------------------------------------------------------------------------------
//		� DisposeAllElements �
//
// Dispose all elements from the slide show file
// ---------------------------------------------------------------------------------
void DisposeAllElements(SlideShowFile theFile)
{
	if (theFile->elements)
	{
		SlideShowElement element, temp;
		
		element = theFile->elements;
		
		// delete all the elements
		while (element)
		{
			temp = element;
			
			element = (SlideShowElement)(element->nextElement);
			
			DisposeElementFields(temp);
			DisposePtr((Ptr)temp);
		}
		
		theFile->elements = nil;
	}
}
	
// ---------------------------------------------------------------------------------
//		� GetStringAttribute �
//
// Get a string attribute
// ---------------------------------------------------------------------------------
ComponentResult GetStringAttribute(XMLAttributePtr attributes, short attributeType, StringPtr *theString)
{
	ComponentResult err = noErr;
	long 			attributeIndex, stringLength;
	
	// get the attribute index in the array
	attributeIndex = GetAttributeIndex(attributes, attributeType);
	
	// get the value
	if (attributeIndex != attributeNotFound && (attributes[attributeIndex].valueKind == attributeValueKindCharString))
	{
		// allocate the string
		stringLength = strlen((Ptr)(attributes[attributeIndex].valueStr));
		*theString = (StringPtr)NewPtr(stringLength + 1);
		err = MemError();
		if (err) goto bail;

		// copy the string value
		BlockMove(attributes[attributeIndex].valueStr, *theString + 1, stringLength);
		(*theString)[0] = stringLength;
	}
	else
	{
		err = attributeNotFound;
	}
bail:
	
	return err;
}

// ---------------------------------------------------------------------------------
//		� GetIntegerAttribute �
//
// Get the integer attribute
// ---------------------------------------------------------------------------------
ComponentResult GetIntegerAttribute(XMLAttributePtr attributes, short attributeType, long *theNumber)
{
	ComponentResult err = noErr;
	long 			attributeIndex;
	
	// get the attribute index in the array
	attributeIndex = GetAttributeIndex(attributes, attributeType);
	
	// get the value
	if (attributeIndex != attributeNotFound && (attributes[attributeIndex].valueKind & attributeValueKindInteger) && theNumber)
	{
		*theNumber = attributes[attributeIndex].value.number;
	}
	else
	{
		err = attributeNotFound;
	}
	
	return err;
}
	
// ---------------------------------------------------------------------------------
//		� GetAttributeIndex �
//
// Get the index of the attribute
// ---------------------------------------------------------------------------------
long GetAttributeIndex(XMLAttributePtr attributes, short attributeType)
{
	long index = 0;

	if (!attributes)
	{
		index = attributeNotFound;
		goto bail;
	}

	while ((attributes[index]).identifier != xmlIdentifierInvalid && (attributes[index]).identifier != attributeType)
	{
		index++;
	}

	if ((attributes[index]).identifier == xmlIdentifierInvalid)
	{
		index = attributeNotFound;
	}

bail:
	
	return index;
}

// ---------------------------------------------------------------------------------
//		� max �
// ---------------------------------------------------------------------------------
long max(long num1, long num2)
{
	if (num1 > num2)
	{
		return num1;
	}
	else
	{
		return num2;
	}
}

// ---------------------------------------------------------------------------------
//		� CreateEffectTrack �
//
// Given two sources and an effect type, create an effects track
// ---------------------------------------------------------------------------------
ComponentResult CreateEffectTrack(SlideShowFile theFile, SlideShowImage sourceA, SlideShowImage sourceB, SlideShowEffect effect, TimeValue *theTime)
{
	ComponentResult			err 					= noErr;
	Track					sourceATrack 			= nil, sourceBTrack = nil, theEffectTrack = nil;
	Media					theEffectMedia 			= nil;
	OSType					effectCode 				= 0;
	SMPTEWipeType			wipeType 				= 0;
	QTAtomContainer 		theEffectDescription 	= nil, inputMap = nil;
	Handle					dataRef 				= nil;
	ImageDescriptionHandle	desc 					= nil;
	QTAtom					inputAtom;
	long 					name, refIndex;
	OSType					inputType,myType;
	TimeValue				effectStart 			= *theTime;
	
	// add the effect time
	*theTime += effect->duration * 600;

	// add the image
	err = AddImageToMovie(theFile, sourceB, theTime);
	if (err) goto bail;

	// initialize
	dataRef = CreateHandleDataRef();
	if (dataRef == nil) goto bail;
	err = QTNewAtomContainer(&theEffectDescription);
	if (err) goto bail;
	err = QTNewAtomContainer(&inputMap);
	if (err) goto bail;
	desc = (ImageDescriptionHandle)NewHandle(0);
	if (noErr != (err = MemError())) goto bail;
	effectCode = GetEffectType(effect->type, &wipeType);

	// create two tracks that reference the appropriate portions of the source and dest tracks
	err = CreateImageTrackForEffect(theFile, effectStart, effect->duration * 600, sourceA->dataRefIndex, &sourceATrack, sourceA->sampleTime);
	if (err || (sourceATrack == nil)) goto bail;
	err = CreateImageTrackForEffect(theFile, effectStart, effect->duration * 600, sourceB->dataRefIndex, &sourceBTrack, sourceB->sampleTime);
	if (err || (sourceBTrack == nil)) goto bail;

	// create the effects track and media
	theEffectTrack = NewMovieTrack(theFile->theMovie, theFile->movieWidth << 16, theFile->movieHeight << 16, 0);
	if (theEffectTrack == nil) goto bail;
	theEffectMedia = NewTrackMedia(theEffectTrack, VideoMediaType, GetMovieTimeScale(theFile->theMovie), dataRef, HandleDataHandlerSubType);
	if (theEffectMedia == nil) goto bail;

	// create an effect description and set the height and width
	err = MakeImageDescriptionForEffect(effectCode, &desc);
	if (err) goto bail;
	(*desc)->width = theFile->movieWidth;
	(*desc)->height = theFile->movieHeight;

	// add the required atoms, sources and the effect type
	myType = EndianU32_NtoB(effectCode);
	QTInsertChild(theEffectDescription, kParentAtomIsContainer, kParameterWhatName, kParameterWhatID, 0, sizeof(myType), &myType, nil);
	
	if (wipeType)
	{
		myType = EndianU32_NtoB(wipeType);
		QTInsertChild(theEffectDescription, kParentAtomIsContainer, 'wpID', 1, 0, sizeof(myType), &myType, nil);
	}
	
	name = EndianU32_NtoB('src1');
	QTInsertChild(theEffectDescription, kParentAtomIsContainer, kEffectSourceName, 1, 0, sizeof(name), &name, nil);
	name = EndianU32_NtoB('src2');
	QTInsertChild(theEffectDescription, kParentAtomIsContainer, kEffectSourceName, 2, 0, sizeof(name), &name, nil);

	// add effects sample to movie
	err = BeginMediaEdits(theEffectMedia);
	if (err) goto bail;
	err = AddMediaSample(theEffectMedia, theEffectDescription, 0, GetHandleSize(theEffectDescription), effect->duration * 600, (SampleDescriptionHandle)desc, 1, 0, nil);
	if (err) goto bail;
	err = EndMediaEdits(theEffectMedia);
	if (err) goto bail;
	err = InsertMediaIntoTrack(theEffectTrack, *theTime - (effect->duration + sourceB->duration) * 600, 0, effect->duration * 600, fixed1);
	if (err) goto bail;

	// first input
	AddTrackReference(theEffectTrack, sourceATrack, kTrackModifierReference, &refIndex);
	QTInsertChild(inputMap, kParentAtomIsContainer, kTrackModifierInput, refIndex, 0, 0, nil, &inputAtom);
	inputType = EndianU32_NtoB(kTrackModifierTypeImage);
	QTInsertChild(inputMap, inputAtom, kTrackModifierType, 1, 0, sizeof(inputType), &inputType, nil);
	name = EndianU32_NtoB('src1');
	QTInsertChild(inputMap, inputAtom, kEffectDataSourceType, 1, 0, sizeof(name), &name, nil);

	// second input
	AddTrackReference(theEffectTrack, sourceBTrack, kTrackModifierReference, &refIndex);
	QTInsertChild(inputMap, kParentAtomIsContainer, kTrackModifierInput, refIndex, 0, 0, nil, &inputAtom);
	QTInsertChild(inputMap, inputAtom, kTrackModifierType, 1, 0, sizeof(inputType), &inputType, nil);
	name = EndianU32_NtoB('src2');
	QTInsertChild(inputMap, inputAtom, kEffectDataSourceType, 1, 0, sizeof(name), &name, nil);

	// set that map
	SetMediaInputMap(theEffectMedia, inputMap);
	
	// set and update the layer
	SetTrackLayer(theEffectTrack, -1);

bail:
	if (dataRef)
	{
		DisposeHandle(dataRef);
	}
	
	if (inputMap)
	{
		QTDisposeAtomContainer(inputMap);
	}
	
	if (theEffectDescription)
	{
		QTDisposeAtomContainer(theEffectDescription);
	}
	
	if (desc)
	{
		DisposeHandle((Handle)desc);
	}
	
	return err;
}

// ---------------------------------------------------------------------------------
//		� CreateImageTrackForEffect �
//
// Create a new track for the effect 
// ---------------------------------------------------------------------------------
ComponentResult	CreateImageTrackForEffect(SlideShowFile 	theFile,
											TimeValue 		effectOffset,
											TimeValue 		duration,
											long 			dataRefIndex,
											Track 			*sourceTrack,
											TimeValue 		sampleTime)
{
	ComponentResult			err 			= noErr;
	Media 					sourceMedia;
	TimeValue				theSampleTime;
	Handle					dataRef 		= nil;
	OSType					dataRefType;
	short					newDataRefIndex = 0;
	long					dataOffset, size;
	ImageDescriptionHandle	desc 			= nil;

	// initialize
	desc = (ImageDescriptionHandle)NewHandle(0);
	if (noErr != (err = MemError())) goto bail;
	
	// create a new track and media
	*sourceTrack = NewMovieTrack(theFile->theMovie, theFile->movieWidth << 16, theFile->movieHeight << 16, 0);
	if (*sourceTrack == nil) goto bail;
	sourceMedia = NewTrackMedia(*sourceTrack, VideoMediaType, GetMovieTimeScale(theFile->theMovie), nil, 0);
	if (sourceMedia == nil) goto bail;

	// get and add the dataref from the image track
	err = GetMediaDataRef(theFile->theImageMedia, dataRefIndex, &dataRef, &dataRefType, nil);
	if (err) goto bail;
	err = AddMediaDataRef(sourceMedia, &newDataRefIndex, dataRef, dataRefType);
	if (err) goto bail;

	// add a sample reference
	err = GetMediaSampleReference(theFile->theImageMedia, &dataOffset, &size, sampleTime, nil, nil, (SampleDescriptionHandle)desc, nil, 1, nil, 0);
	if (err) goto bail;
	(**desc).dataRefIndex = newDataRefIndex;
	err = AddMediaSampleReference(sourceMedia, dataOffset, size, duration, (SampleDescriptionHandle)desc, 1, 0, &theSampleTime);
	if (err) goto bail;

	// insert into track
	err	= InsertMediaIntoTrack(*sourceTrack, effectOffset, theSampleTime, duration, fixed1);
	if (err) goto bail;

bail:
	if (dataRef)
	{
		DisposeHandle(dataRef);
	}
	
	if (desc)
	{
		DisposeHandle((Handle)desc);
	}
	
	return err;
}

// ---------------------------------------------------------------------------------
//		� CreateHandleDataRef �
//
// Make a handle data reference
// ---------------------------------------------------------------------------------
Handle CreateHandleDataRef()
{
	ComponentResult	err 	= noErr;
	Handle 			dataRef = nil;
	long			dataAtom[2];
	
	dataRef	= NewHandleClear(sizeof(Handle) + sizeof(char));
	
	if (dataRef)
	{
		dataAtom[0]	= EndianU32_NtoB(sizeof(long) + sizeof(OSType));
		dataAtom[1]	= EndianU32_NtoB(FOUR_CHAR_CODE('data'));
		
		err	= PtrAndHand(dataAtom, dataRef, sizeof(long) + sizeof(OSType));
		
		if (err)
		{
			DisposeHandle(dataRef);
			dataRef = nil;
		}
	}
	
	return dataRef;
}

// ---------------------------------------------------------------------------------
//		� GetEffectType �
//
// Given an effects type string, return the corresponding effects constant 
// ---------------------------------------------------------------------------------
OSType GetEffectType(StringPtr theEffect, SMPTEWipeType *theWipeType)
{
	OSType effectType = kSlideTransitionType;
	long wipeType = 0;
	
	if (EqualString(theEffect, "\pdissolve", false, false))
		effectType = kCrossFadeTransitionType;
	else if (EqualString(theEffect, "\pwipe", false, false))
		effectType = kSlideTransitionType;
	else if (EqualString(theEffect, "\pheart", false, false))
	{
		effectType = kWipeTransitionType;
		wipeType = kHeartWipe;
	}
	else if (EqualString(theEffect, "\pstar", false, false))
	{
		effectType = kWipeTransitionType;
		wipeType = kFivePointStarWipe;
	}

	*theWipeType = wipeType;
	
	return effectType;
}

// ---------------------------------------------------------------------------------
//		� SlideShowImportRegister �
//
// Register our component manually, when running in linked mode
// ---------------------------------------------------------------------------------
#ifndef STAND_ALONE

extern void MySlideShowImportRegister(void);

void MySlideShowImportRegister(void)
{
	ComponentDescription	theComponentDescription;
	Handle					nameHdl;
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(SlideShowImportComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)SlideShowImportComponentDispatch);
	#endif

	PtrToHand("\pSlideShow", &nameHdl, 8);
  	theComponentDescription.componentType = MovieImportType;
  	theComponentDescription.componentSubType = FOUR_CHAR_CODE('QTSL');
  	theComponentDescription.componentManufacturer = kAppleManufacturer;
  	theComponentDescription.componentFlags = canMovieImportFiles | canMovieImportInPlace | hasMovieImportMIMEList | canMovieImportDataReferences;
  	theComponentDescription.componentFlagsMask = 0;

	RegisterComponent(&theComponentDescription, componentEntryPoint, 0, nameHdl, 0, 0);
}

#endif