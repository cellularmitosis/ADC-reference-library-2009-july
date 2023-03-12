//////////
//
//	File:		dllmain.c
//
//	Contains:	Code for creating a sound output component.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	03/05/99	rtm		first file
//
//////////

#include <QTML.h>
#include <windows.h>


static HINSTANCE	ghInst = 0;

BOOL WINAPI dllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	ghInst = hInst;

	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
    }
    
    return(true);
}
