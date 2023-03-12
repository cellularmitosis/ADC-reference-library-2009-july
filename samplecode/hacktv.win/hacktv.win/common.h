/*
	File:		Common.h

	Contains:	HackTV common routines
			
	Copyright:	© 1992-1998 by Apple Computer, Inc.
*/

#ifndef _APP_COMMON_
#define _APP_COMMON_

#include <QTML.h>
#include <Menus.h>
#include <Printing.h>
#include <QuickTimeComponents.h>

void InitializeSequenceGrabber(void);
void DoRecord(void);
void DoAboutDialog(void);
void DoPageSetup(void);
void DoPrint(void);
void DoCopyToClipboard(void);
void DoVideoSettings(void);
void DoSoundSettings(void);
void DoResize(short divisor);

// CreateMonitorWindow is implemented platform-specific
void CreateMonitorWindow(void);
#endif
