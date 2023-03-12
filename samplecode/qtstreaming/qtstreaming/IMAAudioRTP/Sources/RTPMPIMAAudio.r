/*
	File:		RTPMPIMAAudio.r

	Contains:	Resources for IMA Audio RTPMediaPacketizer

	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.

	
	
	An RTPMediaPaketizer must define at least one packetizer info resource ('pcki')
	and a public component resource map ('thnr') that points to the packetizer info
	resources.
	
	QuickTime Streaming uses a packetizer info resource to determine what media data
	format a packetizer encodes, and to compare packetizers that encode the same
	type of data.
*/



#undef DLOG_RezTemplateVersion
#define DLOG_RezTemplateVersion 1



#include "RTPMPIMAAudioResources.h"
#include "ComponentThing.r"
#include "Controls.r"
#include "ControlDefinitions.r"
#include "Dialogs.r"
#include "MacTypes.r"
#include "Menus.r"
#include "QTStreamingComponents.r"



#define __SETTINGS_STRING			"Interleaving: "
#define __NO_INTERLEAVING_STRING	"None"
#define __DIALOG_TITLE_STRING		IMA_AUDIO_HI_ENCODING_STRING " Options"
#define __OK_BUTTON_STRING			"OK"
#define __CANCEL_BUTTON_STRING		"Cancel"



resource 'thnr' ( kComponentBaseID )
{
	{
		'pcki', 1, 0,
		'pcki', kComponentBaseID, cmpResourceNoFlags,
	}
};



resource 'pcki' ( kComponentBaseID )
{
	{
		'soun',							// media type
		kIMAAudioDataFormat,			// data format type
		kComponentManufactureType,
		kMediaPacketizerCanPackEmptyEdit,
		canPackIdentityMatrixType,
		{
			kRTPPayloadSpeedTag, 128,
			kRTPPayloadLossRecoveryTag, 128
		},
		kRTPPayloadTypeDynamicFlag,
		0,
		IMA_AUDIO_PROTOCOL_ENCODING_STRING
	};
};



resource 'STR#' ( kRTPMPIMAAudioStringListResource )
{
	{
		IMA_AUDIO_PROTOCOL_ENCODING_STRING,		// kRTPMPIMAAudioProtocolEncodingString
		IMA_AUDIO_HI_ENCODING_STRING,			// kRTPMPIMAAudioHIEncodingString
		__SETTINGS_STRING,						// kRTPMPIMAAudioSettingsString
		__NO_INTERLEAVING_STRING				// kRTPMPIMAAudioNoInterleavingString
	}
};



resource 'MENU' ( -kComponentBaseID, nonpurgeable )
{
	-kComponentBaseID,				// Menu ID
	textMenuProc,					// ID of menu def proc
	0xFFFFFFFD,						// Enable flags 
	enabled,						// Menu enable
	__SETTINGS_STRING,				// Menu Title
	{
		__NO_INTERLEAVING_STRING,	// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"-",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"2",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"3",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"4",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"5",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"6",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"7",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
		"8",						// Item title
		noIcon,						// Icon number
		"",							// Key equivalent or
		noMark,						// Marking char or id
		plain,						// Style
		
	}								// Items
};



resource 'CNTL' ( kComponentBaseID, purgeable )
{
		{ 0, 0, 20, 183 },						// Bounds
		popupTitleLeftJust,						// Value	(title appearance)
		visible,	 							// visible
		100,									// Max		(title width)
		-kComponentBaseID,						// Min		('MENU' ID)
		popupMenuCDEFproc + popupFixedWidth,	// ProcID
		0,										// RefCon
		__SETTINGS_STRING						// Title
};



resource 'DITL' ( kComponentBaseID ) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{51, 163, 71, 221},
		Button {
			enabled,
			__OK_BUTTON_STRING
		},
		/* [2] */
		{51, 92, 71, 150},
		Button {
			enabled,
			__CANCEL_BUTTON_STRING
		},
		/* [3] */
		{14, 38, 34, 221},
		Control {
			disabled,
			kComponentBaseID
		}
	}
};

resource 'DLOG' ( kComponentBaseID, __DIALOG_TITLE_STRING ) {
	{70, 113, 157, 348},
	movableDBoxProc,
	invisible,
	goAway,
	0x0,
	kComponentBaseID,
	__DIALOG_TITLE_STRING,
	alertPositionParentWindow
};



// Appearance Manager support
resource 'dlgx' ( kComponentBaseID )
{
	versionZero
	{
		kDialogFlagsUseThemeBackground + kDialogFlagsUseControlHierarchy +
			kDialogFlagsUseThemeControls
	}
};
