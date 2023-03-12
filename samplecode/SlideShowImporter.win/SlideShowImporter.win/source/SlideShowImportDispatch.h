/*
	File:		SlideShowImportDispatch
	
	Description: 	Slide Show Importer dispatch file.

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


	ComponentSelectorOffset (6)

	ComponentRangeCount (1)
	ComponentRangeShift (7)
	ComponentRangeMask	(7F)

	ComponentRangeBegin (0)
		ComponentError 		(Target)
		ComponentError 		(Register)
		StdComponentCall	(Version)
		StdComponentCall 	(CanDo)
		StdComponentCall 	(Close)
		StdComponentCall 	(Open)
	ComponentRangeEnd (0)
		
	ComponentRangeBegin (1)
		ComponentError 		(0)
		ComponentError 		(Handle)
		ComponentCall 		(File)
		ComponentError 		(SetSampleDuration)
		ComponentError 		(SetSampleDescription)
		ComponentError 		(SetMediaFile)
		ComponentError 		(SetDimensions)
		ComponentError 		(SetChunkSize)
		ComponentError 		(SetProgressProc)
		ComponentError 		(SetAuxiliaryData)
		ComponentError 		(SetFromScrap)
		ComponentError 		(DoUserDialog)
		ComponentError 		(SetDuration)
		ComponentError 		(GetAuxiliaryDataType)
		ComponentError 		(Validate)
		ComponentError 		(GetFileType)
		ComponentCall 		(DataRef)
		ComponentError 		(GetSampleDescription)
		ComponentCall 		(GetMIMETypeList)
		ComponentError 		(SetOffsetAndLimit)
		ComponentError 		(GetSettingsAsAtomContainer)
		ComponentError 		(SetSettingsFromAtomContainer)
		ComponentError 		(SetOffsetAndLimit64)
		ComponentError		(Idle)
		ComponentError		(ValidateDataRef)
		ComponentError		(GetLoadState)
	ComponentRangeEnd (1)