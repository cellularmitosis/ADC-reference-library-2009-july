/*
	File:		VersionInfo.r

	Contains:	Resources used to add version info to the project.

	Written by: Apple DTS

	Copyright:	Copyright © 1996-2001 by Apple Computer, Inc., All Rights Reserved.

				You may incorporate this Apple sample source code into your program(s) without
				restriction. This Apple sample source code has been provided "AS IS" and the
				responsibility for its operation is yours. You are not permitted to redistribute
				this Apple sample source code as "Apple sample source code" after having made
				changes. If you're going to re-distribute the source, we require that you make
				it clear in the source that the code was descended from Apple sample source
				code, but that you've made changes.

	Change History (most recent first):
				11/28/2000	GAW	Intergrated into MoreIsBetter
				7/21/1999	KG	Updated for Metrowerks Codewarror Pro 2.1
				

*/

#include "SysTypes.r"
#include "Types.r"

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#define kMajorVersNumber	0x01
#define kMinorVersNumber	0x02
#define kReleaseStage		developStage
#define kNonFinalRelease	0x01
#define kVersString			"1.2d1"

#define kRegionCode			verUS

resource 'vers' (1) {
	kMajorVersNumber,
	kMinorVersNumber,
	kReleaseStage,
	kNonFinalRelease,
	kRegionCode,
	kVersString,
	"© 1996-2001 Apple Computer, Inc.\n" kVersString ", DTS"
};

resource 'vers' (2) {
	kMajorVersNumber,
	kMinorVersNumber,
	kReleaseStage,
	kNonFinalRelease,
	kRegionCode,
	kVersString,
	"MoreAppleEvents " kVersString
};

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

resource 'STR#' (1, "Product") {		//	as defined in VersionEdit from Teknosys
	{
		"MoreAppleEvents",		//	Product Name
		"",	//	File Name
		"1996-2001",		//	Release Year
		"http://developer.apple.com/samplecode/Sample_Code/Interapplication_Comm/MoreAppleEvents.htm",		//	Web Page
		"ftp://ftp.apple.com/developer/Sample_Code/Interapplication_Comm/MoreAppleEvents.sit"		//	FTP Directory
	}
};

resource 'STR#' (2, "Vendor") {		//	as defined in VersionEdit from Teknosys
	{
		"Developer Technical Support",				//	Author
		"Apple Computer, Inc.",						//	Company
		"1 Infinite Loop\nCopertino, CA  95014",	//	Address
		"",		//	Phone
		"",		//	Fax
		"",		//	AOL address
		"",		//	CompuServe address
		"devbugs@apple.com",			//	Internet eMail
		"http://developer.apple.com/",	//	Web Site
		"",		//	FTP Directory
		""
	}
};

#define prodDesc "ProductInfo.txt"
read 'STR ' (1, prodDesc) prodDesc;				//	as defined in VersionEdit from Teknosys

