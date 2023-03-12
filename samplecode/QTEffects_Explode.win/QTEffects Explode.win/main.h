/*
	File:		main.h
	
	Contains:	Code to generate a QuickTime movie with a QuickTime video effect in it.
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime SDK QTShowEffect sample code by Tim Monroe)

	Copyright:	� 1999 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/25/99		srk		first file


*/
#if TARGET_OS_MAC
	static void InitMacToolbox (void);
	static void Macintosh_DisplayMsg(char *msg);
#endif

#if TARGET_OS_WIN32
	static void Win32_DisplayMsg(char *msg);
#endif

static void QuickTimeInit (void);
void CheckError(OSErr error, char *msg);