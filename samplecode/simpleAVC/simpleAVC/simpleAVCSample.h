/*
	File:		SimpleAVCSample.h
	
	Description:	Sample showing use of the IOFireWireAVCFamily

	Author:		<JAS>

	Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
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
	When		Who	Version	What
	====		===	=======	====
	04/04/02	JAS	1.0	Created

*/


// human readable response strings
char* responseStr[] =
{
	"Reserved0",
	"Reserved1",
	"Reserved2",
	"Reserved3",
	"Reserved4",
	"Reserved5",
	"Reserved6",
	"Reserved7",
	"NOT IMPLEMENTED",
	"ACCEPTED",
	"REJECTED",
	"IN TRANSITION",
	"IMPLEMENTED/STABLE",
	"CHANGED",
	"ReservedE",
	"INTERIM"
};

// human readable unit and subunit types
char* unitTypeStr[] =
{
	"Video monitor",
	"Reserved1", "Reserved2", "Reserved3",
	"Video cassette recorder (VCR)",
	"TV tuner",
	"Reserved6",
	"Video camera",
	"Reserved8", "Reserved9", "ReservedA", "ReservedB", "ReservedC", "ReservedD", "ReservedE", "ReservedF",
	"Reserved10", "Reserved11", "Reserved12", "Reserved13", "Reserved14", "Reserved15", "Reserved16",
	"Reserved17", "Reserved18", "Reserved19", "Reserved1A", "Reserved1B",
	"Vendor unique",
	"Reserved1D", "Reserved1E",
	"Unit"
};