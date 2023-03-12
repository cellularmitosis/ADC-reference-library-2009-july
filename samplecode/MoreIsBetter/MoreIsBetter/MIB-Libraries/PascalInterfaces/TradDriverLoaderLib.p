(*
	File:		TradDriverLoaderLib.p

	Contains:	Pascal interface for the pseudo-DriverLoaderLib for 'DRVR's.

	Written by:	Quinn

	Copyright:	© 1996-1999 by Apple Computer, Inc., all rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

$Log: TradDriverLoaderLib.p,v $
Revision 1.2  2001/11/07 15:59:00         
Tidy up headers, add CVS logs, update copyright.


         <1>     21/9/01    Quinn   Moved this file into the new Pascal ghetto.
         <1>     25/2/99    Quinn   First checked in.
*)

unit TradDriverLoaderLib;

interface

	uses
		
		// MoreIsBetter Setup

		MoreSetup,

		// Mac OS Interfaces

		Types,
		Devices;

	(* The following types are not in the "Devices.p".  I need them 
		because TradGetDriverInformation uses pointers instead
		of var parameters to make it easy to pass nil to
		the parameters you're not interested in.
	*)

	type
		DriverFlagsPtr = ^DriverFlags;
		UnitNumberPtr = ^UnitNumber;
		DriverRefNumPtr = ^DriverRefNum;
		DRVRHeaderPtrPtr = ^DRVRHeaderPtr;
	
	(* See "TradDriverLoaderLib.h" for comments.  I couldn't be bothered
		replicating them in both files.
	*)

	function TradHigherDriverVersion(var driverVersion1 : NumVersion;
										var driverVersion2 : NumVersion) : Boolean;
	function TradHighestUnitNumber : UnitNumber;
	function TradDriverGestaltOn(refNum : DriverRefNum) : OSErr;
	function TradDriverGestaltOff(refNum : DriverRefNum) : OSErr;
	function TradDriverGestaltIsOn(flags : DriverFlags) : Boolean;
	function TradLookupDrivers(beginningUnit : UnitNumber;
								endingUnit : UnitNumber;
								emptyUnits : Boolean;
								var returnedRefNums : ItemCount; 
								refNums : DriverRefNumPtr) : OSErr;
	function TradInstallDriverFromPtr(driver : DRVRHeaderPtr;
										beginningUnit : UnitNumber;
										endingUnit : UnitNumber;
										var refNum : DriverRefNum) : OSErr;
	function TradInstallDriverFromHandle(driver : DRVRHeaderHandle;
											beginningUnit : UnitNumber;
											endingUnit : UnitNumber;
											var refNum : DriverRefNum) : OSErr;
	function TradInstallDriverFromResource(rsrcID : SInt16; rsrcName : StringPtr;
											beginningUnit : UnitNumber;
											endingUnit : UnitNumber;
											var refNum : DriverRefNum) : OSErr;
	function TradGetDriverInformation(refNum : DriverRefNum;
										thisUnit : UnitNumberPtr;
										flags : DriverFlagsPtr;
										name : StringPtr;
										driverHeader : DRVRHeaderPtrPtr
										) : OSErr;
	function TradOpenInstalledDriver(refNum : DriverRefNum; ioPermission : SInt8) : OSErr;
	function TradRemoveDriver(refNum : DriverRefNum; immediate : Boolean) : OSErr;
	function TradRenameDriver(refNum : DriverRefNum; newDriverName : Str255) : OSErr;

end. (* TradDriverLoaderLib *)

