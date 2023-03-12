//////////
//
//	File:		QDMediaCommon.h
//
//	Contains:	Header file for things common to QuickDraw media handler and media creator.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/14/99	rtm		
//	   
//////////

#ifndef __QDMEDIACOMMON__
#define __QDMEDIACOMMON__


//////////
//
// compiler flags
//
//////////

#define HANDLER_SWAPS_SAMPLE_DESC		0			// does handler implement _SampleDescriptionB2N/N2B?


//////////
//
// constants
//
//////////

#define kQDMH_MediaType					'Qdrw'
#define kQDMH_Version					0x00010001
#define kQDMH_ComponentManufacturer		'WANG'		// component manufacturer

// resource IDs
#define kQDMH_ComponentResID			1000		// ID of media handler 'thng' and code resources
#define kQDMH_NameStringResID			1000		// ID of codec name resource
#define kQDMH_InfoStringResID			1001		// ID of codec info string resource
#define kQDMH_IconResID					1000		// ID of codec icon resource


// resource names
#define kQDMH_Name						"QuickDraw Media Handler"


#ifndef REZ
//////////
//
// data types
//
//////////

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct QDrawDescription {
	long							size;					// total size of QDrawDescription
	long							type;					// QDrawMediaType
	long							resvd1;
	short							resvd2;
	short							dataRefIndex;
	long							version;				// the version of this data
} QDrawDescription, *QDrawDescriptionPtr, **QDrawDescriptionHandle;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#endif	// #ifndef REZ

#endif	// #ifndef __QDMEDIACOMMON__
