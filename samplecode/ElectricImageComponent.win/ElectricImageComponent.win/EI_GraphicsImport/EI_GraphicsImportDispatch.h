/*
	File:		EI_GraphicsImportDispatch.h
	
	Description: QuickTime graphics importer component

	Author:		QuickTime Engineering, JSAM

	Copyright: 	� Copyright 1999-2003 Apple Computer, Inc. All rights reserved.
	
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
	   <2>		1/31/01		ERA			updated for X
	   <1>	 	11/28/99	QTE			first file
*/

/*	A Quick Course in ComponentDispatchHelper.c

		ComponentRangeCount
		The number (0 based) of discrete, continuous ranges of selectors that are used in the macro.  The
		'range shift' & 'range mask' determine the size of the range.

		ComponentRangeShift
		ComponentRangeMask
		The mask == 2^shift -1.  This is used to define size of the range of selectors.

		For ImageCodecs as an example, the ranges are in size of 0x100:
		
		   Range 0 = -0xFF to -0x01 	Standard Component Calls
		   Range 1 = 0x000 to 0x0FF		Generic codecs 
		   Range 2 = 0x100 to 0x1FF		Specific to QT Photo JPEG codecs
		   Range 3 = 0x200 to 0x2FF		Base Decompressor Client codecs
		   Range 4 = 0x300 to 0x3FF		Effect codecs
			
		ComponentRangeBegin & ComponentRangeEnd
		Use these macros to bracket the start/stop of each range of selectors.  First range use (0), for the Nth
		range use (N-1).

		ComponentRangeUnused
		Used to skip a range of selectors.

		ComponentSelectorOffset
		Since the 'default' selectors are negative numbers, an offset must be specified.  For example, if the first
		default selector supported was kComponentUnregisterSelect (-7), then the offset would have to be 7.

		StdComponentCall
		Used to dispatch to one of the standard compoenent routines, such as Open(), Close(), Register(), etc. that
		are implemented in the component.

		ComponentError
		Returns an error without trying to do anything.  Use this for the selectors that will NEVER be implmented
		or delegated.  Additionally, it can be used to fill in a 'blank' in a range of selectors to get the range
		continuous. For example, if there were selectors that corresponded to 10,11,12, 14, but NO 13, merely put
		ComponentError (0) between the selectors for 12 & 14.
		
		ComponentNoError
		Returns 'noErr' without trying to do anything.

		ComponentDelegate
		Delagates the call to the specified delagate component.

		ComponentCall
		This dispatches to the appropriate routine in that is implemented in the component.

		ComponentComment
		Allows comments to be placed in the big macro.
*/

	ComponentSelectorOffset (6)

	ComponentRangeCount (1)
	ComponentRangeShift (8)
	ComponentRangeMask	(FF)

	ComponentRangeBegin (0)
		StdComponentCall (Target)
		ComponentError   (Register)
		StdComponentCall (Version)
		StdComponentCall (CanDo)
		StdComponentCall (Close)
		StdComponentCall (Open)
	ComponentRangeEnd (0)
	
	ComponentRangeBegin (1)
		ComponentError (0)
		ComponentDelegate (SetDataReference)
		ComponentDelegate (GetDataReference)
		ComponentDelegate (SetDataFile)
		ComponentDelegate (GetDataFile)
		ComponentDelegate (SetDataHandle)
		ComponentDelegate (GetDataHandle)
		ComponentCall     (GetImageDescription)
		ComponentCall     (GetDataOffsetAndSize)
		ComponentDelegate (ReadData)
		ComponentDelegate (SetClip)
		ComponentDelegate (GetClip)
		ComponentDelegate (SetSourceRect)
		ComponentDelegate (GetSourceRect)
		ComponentDelegate (GetNaturalBounds)
		ComponentDelegate (Draw)
		ComponentDelegate (SetGWorld)
		ComponentDelegate (GetGWorld)
		ComponentDelegate (SetMatrix)
		ComponentDelegate (GetMatrix)
		ComponentDelegate (SetBoundsRect)
		ComponentDelegate (GetBoundsRect)
		ComponentDelegate (SaveAsPicture)
		ComponentDelegate (SetGraphicsMode)
		ComponentDelegate (GetGraphicsMode)
		ComponentDelegate (SetQuality)
		ComponentDelegate (GetQuality)
		ComponentDelegate (SaveAsQuickTimeImageFile)
		ComponentDelegate (SetDataReferenceOffsetAndLimit)
		ComponentDelegate (GetDataReferenceOffsetAndLimit)
		ComponentDelegate (GetAliasedDataReference)
		ComponentCall     (Validate)
		ComponentCall     (GetMetaData)
		ComponentCall     (GetMIMETypeList)
		ComponentDelegate (DoesDrawAllPixels)
		ComponentDelegate (GetAsPicture)
		ComponentDelegate (ExportImageFile)
		ComponentDelegate (GetExportImageTypeList)
		ComponentDelegate (DoExportImageFileDialog)
		ComponentDelegate (GetExportSettingsAsAtomContainer)
		ComponentDelegate (SetExportSettingsFromAtomContainer)
		ComponentDelegate (SetProgressProc)
		ComponentDelegate (GetProgressProc)
		ComponentDelegate (GetImageCount)
		ComponentDelegate (SetImageIndex)
		ComponentDelegate (GetImageIndex)
		ComponentDelegate (GetDataOffsetAndSize64)
		ComponentDelegate (ReadData64)
		ComponentDelegate (SetDataReferenceOffsetAndLimit64)
		ComponentDelegate (GetDataReferenceOffsetAndLimit64)
		ComponentDelegate (GetDefaultMatrix)
		ComponentDelegate (GetDefaultClip)
		ComponentDelegate (GetDefaultGraphicsMode)
		ComponentDelegate (GetDefaultSourceRect)
		ComponentDelegate (GetColorSyncProfile)
		ComponentDelegate (SetDestRect)
		ComponentDelegate (GetDestRect)
		ComponentDelegate (SetFlags)
		ComponentDelegate (GetFlags)
		ComponentError 	  (0x003B)
		ComponentError    (0x003C)
		ComponentDelegate (GetBaseDataOffsetAndSize64Select)
		ComponentDelegate (SetImageIndexToThumbnailSelect)
	ComponentRangeEnd (1)
