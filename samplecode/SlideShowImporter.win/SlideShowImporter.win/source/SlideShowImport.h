/*
	File:		SlideShowImport.h
	
	Description: 	Slide Show Importer include file.

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

// ---------------------------------------------------------------------------------
//		� Includes �
// ---------------------------------------------------------------------------------

#if __APPLE_CC__
    #include <QuickTime/QuickTime.h>
#else
    #include <QuickTimeComponents.h>
#endif

// ---------------------------------------------------------------------------------
//		� Structure Definitions �
// ---------------------------------------------------------------------------------
typedef struct
	{
	ComponentInstance	self;
	}
	SlideShowImportGlobalsRecord, *SlideShowImportGlobals;


enum {
	kSlideShowImageKind = 'imag',
	kSlideShowEffectKind = 'effe',
	kSlideShowAudioKind = 'audi'
};

	
typedef struct
	{
	TimeValue	duration;
	StringPtr	src;
	
	// used during import
	Fixed		discoveredWidth;
	Fixed		discoveredHeight;
	
	short		dataRefIndex;
	TimeValue	sampleTime;
	}
	SlideShowImageRecord, *SlideShowImage;

typedef struct
	{
	TimeValue		duration;
	StringPtr		type;
	
	// 
	QTAtomContainer	effectDescription;
	}
	SlideShowEffectRecord, *SlideShowEffect;
	
typedef struct 
	{
	StringPtr	src;
	Boolean		dontScaleImagesAndEffectsToThisMovie;
	}
	SlideShowMovieRecord, *SlideShowMovie;
	
typedef struct SlideShowElementRecord
	{
		struct SlideShowElementRecord *	nextElement;

		OSType					elementKind;	
		
		SlideShowImageRecord	image;
		SlideShowEffectRecord	effect;
		SlideShowMovieRecord	movie;
	}
	SlideShowElementRecord, *SlideShowElement;

typedef struct
	{
	long			movieHeight;
	long			movieWidth;
	SlideShowImage	images;
	Movie			theMovie;
	Handle			dataRef;
	OSType			dataRefType;
	
	long			numImages;
	long			numEffects;
	long			numAudioTracks;
	
	SlideShowElement	elements;		// linked list of SlideShowElement records

	// used during construction of movie
	Track			theImageTrack;
	Media			theImageMedia;
	
	
	SlideShowImage	thePreviousSource;		// source A for the transition
	SlideShowEffect	thePendingEffect;		// 
	
	}
	SlideShowFileRecord, *SlideShowFile;

// ---------------------------------------------------------------------------------
//		� Defines �
// ---------------------------------------------------------------------------------

enum
	{
	element_slideshow = 1,
	element_image,
	element_effect,
	element_audio,
	SLIDESHOW_NUM_ELEMENTS,
	element_NOTFOUND = SLIDESHOW_NUM_ELEMENTS
	};

enum
	{
	attr_height = 1,
	attr_width,
	attr_src,
	attr_dur,
	attr_type,
	attr_scale,
	SLIDESHOW_NUM_ATTRIBUTES,
	attr_NOTFOUND = SLIDESHOW_NUM_ATTRIBUTES
	};

enum
	{
	attributeNotFound = -1
	};

// ---------------------------------------------------------------------------------
//		� Function Prototypes �
// ---------------------------------------------------------------------------------

ComponentResult 	ReadSlideShowFile(SlideShowFile theFile);
ComponentInstance 	CreateXMLParserForSlideShow();
ComponentResult		GetRootElementAttributes(XMLAttribute *element, SlideShowFile theFile);
ComponentResult		ParseAndSaveImage(XMLElement *element, SlideShowFile theFile);
ComponentResult		ParseEffectElement(XMLElement *element, SlideShowFile theFile);
ComponentResult		ParseAudioElement(XMLElement *element, SlideShowFile theFile);
ComponentResult		BuildMovie(SlideShowFile theSlideShowFile);
void 				DisposeSlideShowFile(SlideShowFile theFile);
void				DisposeAllElements(SlideShowFile theFile);
void				DisposeElementFields(SlideShowElement element);
void				AddSlideShowElement(SlideShowFile theFile, SlideShowElement theElement);
ComponentResult 	GetStringAttribute(XMLAttributePtr attributes, short attributeType, StringPtr *theString);
ComponentResult 	GetIntegerAttribute(XMLAttributePtr attributes, short attributeType, long *theNumber);
long 				GetAttributeIndex(XMLAttributePtr attributes, short attributeType);
OSErr				AddImageToMovie(SlideShowFile theFile, SlideShowImage theImage, TimeValue *theTime);
OSErr				AddAudioToMovie(SlideShowFile theFile, SlideShowMovie theImage, TimeValue *theTime);
long				max(long num1, long num2);
Boolean 			isAbsoluteURL(char *theURL);
OSErr 				CreateDataRefFromURL(char* theURL, Handle baseDRef, OSType baseDRefType, Handle *dref, OSType *drefType);
ComponentResult 	CreateEffectTrack(SlideShowFile theFile, SlideShowImage sourceA, SlideShowImage sourceB, SlideShowEffect effect, TimeValue *theTime);
ComponentResult		CreateImageTrackForEffect(SlideShowFile theFile, TimeValue effectOffset, TimeValue duration, long dataRefIndex, Track *sourceTrack, TimeValue sampleTime);
Handle 				CreateHandleDataRef();
OSType 				GetEffectType(StringPtr theEffect, SMPTEWipeType *theWipeType);
