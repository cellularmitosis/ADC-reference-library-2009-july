//////////
//
//	File:		QTFileTransfer.h
//
//	Contains:	Sample code for transferring a file asynchronously from a web server.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/11/98	rtm		first file
//	 
//////////

#include <FixMath.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
#include <Script.h>

#include <string.h>

#define TESTING_FTP_TRANSFER	1			// compiler flag for our test shell


//////////
//
// constants
//
//////////

#define kDataBufferSize			1024*10		// the size, in bytes, of our data buffer

// type and creator for the transferred file
#define kTransFileType			FOUR_CHAR_CODE('TEXT')
#define kTransFileCreator		FOUR_CHAR_CODE('CWIE')


//////////
//
// function prototypes
//
//////////

OSErr							QTFileTrans_CopyRemoteFileToLocalFile (char *theURL, FSSpecPtr theFSSpecPtr);
PASCAL_RTN void					QTFileTrans_ReadDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr);
PASCAL_RTN void					QTFileTrans_WriteDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr);
void							QTFileTrans_CloseDownHandlers (void);
