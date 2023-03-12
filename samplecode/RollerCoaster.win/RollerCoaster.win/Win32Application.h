/*
	File:		Win32Application.h
	
	Contains:	Interface file for Win32Application.c
	
	Written by:	Scott Kuechle, based on original Gerbils code by Brian Greenstone

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/1/98		srk		first file


*/

#pragma once

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#if defined(_MSC_VER)
#include "WinPrefix.h"
#endif

/* Windows headers */
#define	STRICT
#include <windows.h>		// required for all Windows applications
#include "resource.h"		// Windows resource IDs

#include <stdio.h>

#include "Document.h"
#include "QD3DSupport.h"

#include "QD3DIO.h"
#include "QD3DErrors.h"
#include "QD3DStorage.h"
