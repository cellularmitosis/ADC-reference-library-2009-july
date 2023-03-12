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
	ComponentRangeMask	(FF)
	
	ComponentComment ("Core Selector Range")
	ComponentRangeBegin (0)
		StdComponentCall (Target)
		StdComponentCall (Register)
		StdComponentCall (Version)
		StdComponentCall (CanDo)
		StdComponentCall (Close)
		StdComponentCall (Open)
	ComponentRangeEnd (0)
	
	ComponentRangeUnused(1)	
	ComponentRangeUnused(2)	
	ComponentRangeUnused(3)
	ComponentRangeUnused(4)	
	ComponentRangeUnused(5)	
	
	ComponentComment ("Derived Media Handler Range")
	ComponentRangeBegin (6)
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
		ComponentDelegate	(SetVideoParam)
		ComponentDelegate	(GetVideoParam)
		ComponentDelegate	(Compare)
		ComponentDelegate	(GetClock)
		ComponentDelegate	(SetSoundOutputComponent)
		ComponentDelegate	(GetSoundOutputComponent)
		ComponentDelegate	(SetSoundLocalizationData)
		ComponentDelegate	(GetInvalidRegion)
#if HANDLER_SWAPS_SAMPLE_DESC
		ComponentCall		(SampleDescriptionB2N)
		ComponentCall		(SampleDescriptionN2B)
#else
		ComponentDelegate	(SampleDescriptionB2N)
		ComponentDelegate	(SampleDescriptionN2B)
#endif
		ComponentDelegate	(QueueNonPrimarySourceData)
		ComponentDelegate	(FlushNonPrimarySourceData)
		ComponentDelegate	(GetURLLink)
		ComponentDelegate	(MakeMediaTimeTable)
		ComponentDelegate	(HitTestForTargetRefCon)
		ComponentDelegate	(HitTestTargetRefCon)
		ComponentDelegate	(GetActionsForQTEvent)
		ComponentDelegate	(DisposeTargetRefCon)
		ComponentDelegate	(TargetRefConsEqual)
		ComponentDelegate	(SetActionsCallback)
		ComponentDelegate	(PrePrerollBegin)
		ComponentDelegate	(PrePrerollCancel)
		ComponentDelegate	(EnterEmptyEdit)
		ComponentDelegate	(CurrentMediaQueuedData)
		ComponentDelegate	(GetEffectiveVolume)
		ComponentDelegate	(ResolveTargetRefCon)
		ComponentDelegate	(GetSoundLevelMeteringEnabled)
		ComponentDelegate	(SetSoundLevelMeteringEnabled)
		ComponentDelegate	(GetSoundLevelMeterInfo)
		ComponentDelegate	(GetEffectiveSoundBalance)
		ComponentDelegate	(SetScreenLock)
		ComponentDelegate	(SetDoMCActionCallback)
		ComponentDelegate	(GetErrorString)
		ComponentDelegate	(GetSoundEqualizerBands)
		ComponentDelegate	(SetSoundEqualizerBands)
		ComponentDelegate	(GetSoundEqualizerBandLevels)
		ComponentDelegate	(DoIdleActions)
		ComponentDelegate	(SetSoundBassAndTreble)
		ComponentDelegate	(GetSoundBassAndTreble)
		ComponentDelegate	(TimeBaseChanged)
	ComponentRangeEnd (6)
