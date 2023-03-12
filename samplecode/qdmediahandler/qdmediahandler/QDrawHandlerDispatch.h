//////////
//
//	File:		QDrawHandlerDispatch.h
//
//	Contains:	Header file for component dispatch selectors.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	05/19/99	rtm		revised to include all unsupported selectors in MediaHandlers.k.h		
//	   <1>	 	01/14/99	rtm		first file
//	   
//
//	This file describes the selectors used by this component. The 16-bit selector space
//	is divided into consecutively-numbered ranges of uniform size. Each range has a list
//	of zero or more consecutive selectors.
//
//	The file ComponentDispatchHelper.c uses this file to generate function prototypes and
//	a dispatcher for this component.
//
//////////

	ComponentComment ("Count of selectors in range 0")
	ComponentSelectorOffset (6)
	
	ComponentComment ("Last selector range of this component")
	ComponentRangeCount (6)
	
	ComponentComment ("Size of each selector range in bits")
	ComponentRangeShift (8)
	ComponentRangeMask (FF)
	
	ComponentComment ("Core Selector Range")
	ComponentRangeBegin (0)
		StdComponentCall 	(Target)
		StdComponentCall 	(Register)
		StdComponentCall 	(Version)
		StdComponentCall 	(CanDo)
		StdComponentCall 	(Close)
		StdComponentCall 	(Open)
	ComponentRangeEnd (0)
	
	ComponentRangeUnused (1)		// selectors 0x00xx
	ComponentRangeUnused (2)		// selectors 0x01xx
	ComponentRangeUnused (3)		// selectors 0x02xx
	ComponentRangeUnused (4)		// selectors 0x03xx
	ComponentRangeUnused (5)		// selectors 0x04xx
	
	ComponentComment ("Derived Media Handler Range")
	ComponentRangeBegin (6)			// selectors 0x05xx (see MediaHandlers.h)
		ComponentError		(0)
		ComponentCall		(Initialize)
		ComponentDelegate	(SetHandlerCapabilities)
		ComponentCall		(Idle)
		ComponentDelegate	(GetMediaInfo)
		ComponentDelegate	(PutMediaInfo)
		ComponentCall		(SetActive)
		ComponentCall		(SetRate)
		ComponentDelegate	(GGetStatus)
		ComponentCall		(TrackEdited)
		ComponentDelegate	(SetMediaTimeScale)
		ComponentDelegate	(SetMovieTimeScale)
		ComponentCall		(SetGWorld)
		ComponentCall		(SetDimensions)
		ComponentDelegate	(SetClip)
		ComponentCall		(SetMatrix)
		ComponentDelegate	(GetTrackOpaque)
		ComponentDelegate	(SetGraphicsMode)
		ComponentDelegate	(GetGraphicsMode)
		ComponentDelegate	(GSetVolume)
		ComponentDelegate	(SetSoundBalance)
		ComponentDelegate	(GetSoundBalance)
		ComponentDelegate	(GetNextBoundsChange)
		ComponentDelegate	(GetSrcRgn)
		ComponentDelegate	(Preroll)
		ComponentCall		(SampleDescriptionChanged)
		ComponentDelegate	(HasCharacteristic)
		ComponentDelegate	(GetOffscreenBufferSize)
		ComponentDelegate	(SetHints)
		ComponentDelegate	(GetName)
		ComponentDelegate	(ForceUpdate)
		ComponentDelegate	(GetDrawingRgn)
		ComponentDelegate	(GSetActiveSegment)
		ComponentDelegate	(InvalidateRegion)
		ComponentDelegate	(GetNextStepTime)
		ComponentDelegate	(SetNonPrimarySourceData)
		ComponentDelegate	(ChangedNonPrimarySource)
		ComponentDelegate	(TrackReferencesChanged)
		ComponentDelegate	(GetSampleDataPointer)
		ComponentDelegate	(ReleaseSampleDataPointer)
		ComponentDelegate	(TrackPropertyAtomChanged)
		ComponentDelegate	(SetTrackInputMapReference)
		ComponentDelegate	(0x052A)
		ComponentDelegate	(SetVideoParam)
		ComponentDelegate	(GetVideoParam)
		ComponentDelegate	(Compare)
		ComponentDelegate	(GetClock)
		ComponentDelegate	(SetSoundOutputComponent)
		ComponentDelegate	(GetSoundOutputComponent)
		ComponentDelegate	(SetSoundLocalizationData)
		ComponentDelegate	(0x0532)
		ComponentDelegate	(0x0533)
		ComponentDelegate	(0x0534)
		ComponentDelegate	(0x0535)
		ComponentDelegate	(0x0536)
		ComponentDelegate	(0x0537)
		ComponentDelegate	(0x0538)
		ComponentDelegate	(0x0539)
		ComponentDelegate	(0x053A)
		ComponentDelegate	(0x053B)
		ComponentDelegate	(GetInvalidRegion)
		ComponentDelegate	(0x053D)
#if HANDLER_SWAPS_SAMPLE_DESC
		ComponentCall		(SampleDescriptionB2N)
		ComponentCall		(SampleDescriptionN2B)
#else
		ComponentDelegate	(SampleDescriptionB2N)
		ComponentDelegate	(SampleDescriptionN2B)
#endif
	ComponentRangeEnd (6)
