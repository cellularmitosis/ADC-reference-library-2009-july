/*
	File:		Utils.c
	
	Contains:	Utility functions
	
	Written by:	Scott Kuechle, based on original Gerbils code by Brian Greenstone

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/1/98		srk		first file


*/
/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#include "Utils.h"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	void Utils_Macintosh_DisplayMsg(char *msg);
#else if TARGET_OS_WIN32
	void Utils_Win32_DisplayMsg(char *msg);

#endif

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	#define MsgDialogRsrcID		129
	#define MsgItemID			3	
#endif

/************************************************************
*                                                           *
*    FUNCTION:  Utils_MyRandomLong                          *
*                                                           *
*    PURPOSE:   Our own random number generator which       *
*               generates a random long value               *
*                                                           *
*************************************************************/

unsigned long Utils_MyRandomLong(void)
{
unsigned long seed0 = 0, seed1 = 0, seed2 = 0;

  return seed2 ^= (((seed1 ^= (seed2>>5)*1568397607UL)>>7)+
                   (seed0 = (seed0+1)*3141592621UL))*2435386481UL;
}


/************************************************************
*                                                           *
*    FUNCTION:  Utils_RotatePoint                           *
*                                                           *
*    PURPOSE:   Rotates the given point around the y-axis   *
*               0,0 by angle radians                        *
*                                                           *
*************************************************************/

void Utils_RotatePoint(TQ3Point3D *point, float yangle)
{
float	x,y,z;

	x = point->x;
	y = point->y;
	z = point->z;
	
				/* ROTATE ON Y AXIS */
				
	point->z = z * cos(yangle) - x * sin(yangle);
	point->x = z * sin(yangle) + x * cos(yangle);
}

/************************************************************
*                                                           *
*    FUNCTION:  Utils_AngleBetweenVectors                   *
*                                                           *
*    PURPOSE:   Calculate the angle in radians between 2    *
*               3D vectors                                  *
*                                                           *
*************************************************************/

float Utils_AngleBetweenVectors(TQ3Vector3D v1, TQ3Vector3D v2)
{
float	dot,angle;

	Q3Vector3D_Normalize(&v1,&v1);	/* make sure they're normalized */
	Q3Vector3D_Normalize(&v2,&v2);

	dot = Q3Vector3D_Dot(&v1,&v2);	/* the dot product is the cosine of the angle between the 2 vectors */
	angle = acos(dot);				/* get arc-cosine to get the angle out of it */
	
	return(angle);
}

/************************************************************
*                                                           *
*    FUNCTION:  Utils_DisplayErrorMsg                       *
*                                                           *
*    PURPOSE:   Display routine for error messages          *
*                                                           *
*************************************************************/
void Utils_DisplayErrorMsg(char *msg)
{
	#if TARGET_OS_MAC
		Utils_Macintosh_DisplayMsg(msg);
	#else if TARGET_OS_WIN32
		Utils_Win32_DisplayMsg(msg);
	#endif
}

/************************************************************
*                                                           *
*    FUNCTION:  Utils_DisplayFatalErrorMsg                  *
*                                                           *
*    PURPOSE:   Display routine for error messages          *
*                                                           *
*************************************************************/
void Utils_DisplayFatalErrorMsg(char *msg)
{
	#if TARGET_OS_MAC
		Utils_Macintosh_DisplayMsg(msg);
		ExitToShell();
	#else if TARGET_OS_WIN32
		Utils_Win32_DisplayMsg(msg);
		ExitProcess(0); 
	#endif
}


/************************************************************
*                                                           *
*    FUNCTION:  Utils_Macintosh_DisplayMsg                  *
*                                                           *
*    PURPOSE:   Displays Macintosh error messages           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
void Utils_Macintosh_DisplayMsg(char *msg)
{
	DialogPtr theDlog;
	Handle item = NULL;
	Rect box;

		theDlog = GetNewDialog(MsgDialogRsrcID, NULL, (WindowPtr)-1);
		if (theDlog != NULL)
		{
			short itemType;
			
				GetDialogItem(theDlog, MsgItemID, &itemType, &item, &box);
				if (item != NULL)
				{
					short itemHit;
					
						SetDialogItemText(item, c2pstr(msg));
						ModalDialog(NULL, &itemHit);
						DisposeDialog(theDlog);
						p2cstr((StringPtr)msg);	/* restore C-string */
				}
		}
		else	/* program resources can't be found, so use macsbug */
		{
			DebugStr(c2pstr(msg));
			p2cstr((StringPtr)msg);	/* restore C-string */
		}
}
#endif

/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_DisplayMsg                      *
*                                                           *
*    PURPOSE:   Displays error messages for Win95/NT sample *
*               code                                        *
*                                                           *
*************************************************************/

#if TARGET_OS_WIN32
void Utils_Win32_DisplayMsg(char *msg)
{
	/* Display the string. */

	MessageBox( NULL, msg, "Error", MB_OK|MB_ICONINFORMATION );
}


/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_DisplayMsg                      *
*                                                           *
*    PURPOSE:   Displays error messages for Win95/NT sample *
*               code                                        *
*                                                           *
*************************************************************/


DWORD Utils_Win32_BuildCurDirPath(LPTSTR path, LPTSTR filename)
{
	char szAppPathHold[MAX_PATH];
	DWORD nSize = MAX_PATH, len;
	char *ndirLocal;
	int count;

		path[0] = 0;

 		len = GetModuleFileName( NULL, szAppPathHold, nSize);

		if (len != 0)
		{
 			ndirLocal = strrchr( szAppPathHold, '\\' );
 			count = ndirLocal - szAppPathHold + 1;
 			strncat (path, szAppPathHold, count);

				/* tack filename onto path to current directory */
			strcat (path, filename );

			return ERROR_SUCCESS;
		}
		else
		{
			return (GetLastError());
		}
}



/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_GetPicFromFile                  *
*                                                           *
*    PURPOSE:   Returns a Picture Handle for a PICT file    *
*                                                           *
*                                                           *
*************************************************************/


ComponentResult Utils_Win32_GetPicFromFile(LPTSTR		filePath,
											PicHandle	*picH,
											Rect		*picRect)
{
	FSSpec					fsspec;
	OSErr					err;
	GraphicsImportComponent	gi;
	ComponentResult			result;

		*picH = NULL;
		err = NativePathNameToFSSpec((char *)filePath, &fsspec, 0);

		if (err == noErr)
		{
			err = GetGraphicsImporterForFile(&fsspec, &gi);
			if (err == noErr)
			{
				result = GraphicsImportGetAsPicture(gi, picH);
				if (result == noErr)
				{
					return (GraphicsImportGetNaturalBounds(gi, picRect));
				}
			}
		}

		return err;
}

/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_GetPicFromFile                  *
*                                                           *
*    PURPOSE:   Returns a Picture Handle for a PICT file    *
*                                                           *
*                                                           *
*************************************************************/

Boolean Utils_Win32_DoesFileExist(LPTSTR filePath)
{
	HANDLE		fileHndl = NULL;
	Boolean		exists;

			/* ATTEMPT TO OPEN FILE */
		fileHndl = CreateFile(filePath,
							GENERIC_READ,
							FILE_SHARE_READ,	/* share mode */
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
		if ((fileHndl == NULL) || (fileHndl == INVALID_HANDLE_VALUE))
		{
			exists = FALSE;
		}
		else
		{
			exists = TRUE;
			CloseHandle(fileHndl);
		}

		return exists;
}

#endif

/************************************************************
*                                                           *
*    FUNCTION:  Utils_Mac_GetPictForTexture                 *
*                                                           *
*    PURPOSE:   Returns a Picture Handle for a PICT rsrc    *
*                                                           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
void Utils_Mac_GetPictForTexture(short 		resourceID,
								PicHandle	*picH,
								Rect		*picRect)
{
	*picH = GetPicture(resourceID);
	if (*picH != NULL)
	{
		PicHandle pH;
		
			pH = *picH;
			MacSetRect(picRect,
						 (**pH).picFrame.left,
						 (**pH).picFrame.top,
						 (**pH).picFrame.right,
						 (**pH).picFrame.bottom);
	}
}
#endif
