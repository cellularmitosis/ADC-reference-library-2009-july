//////////
//
//	File:		QTMissingComp.c
//
//	Contains:	Sample code for detecting missing QuickTime components.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	03/24/99	rtm		first file
//	 
//////////

#include "QTMissingComp.h"


//////////
//
// QTMissing_IsComponentOfTypeAvailable
// Is the component with the specified type and subtype available?
//
//////////

Boolean QTMissing_IsComponentOfTypeAvailable (OSType theType, OSType theSubType)
{
	ComponentDescription		myCompDesc;
	Component					myComponent = NULL;

	// look for the specified component
	myCompDesc.componentType = theType;
	myCompDesc.componentSubType = theSubType;
	myCompDesc.componentManufacturer = 0;
	myCompDesc.componentFlags = 0;
	myCompDesc.componentFlagsMask = cmpIsMissing;
	
	myComponent = FindNextComponent(NULL, &myCompDesc);

	return(myComponent != NULL);
}


