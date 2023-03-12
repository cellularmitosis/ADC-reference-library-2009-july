//////////
//
//	File:		ComResource.h
//
//	Contains:	Resource definitions for the QuickTime sample code framework. 
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/08/99	rtm		first file
//
//////////

//////////
//
// menu IDs
//
// Windows accesses menu items with a single "menu item identifier", which is of type UINT.
// Macintosh accesses menu items with both a menu ID and a menu item index. To be able to 
// access menu items in a cross-platform manner, we will map the pair of numbers associated
// with a Macintosh menu item into a single "menu item identifier", using the MENU_IDENTIFIER
// macro defined in the file ComFramework.h. This means that the menu item identifiers that
// we use in our Windows code depend on the values we use for the menu ID and menu item indices
// in the Macintosh resource file.
//
//////////

// resource IDs for Macintosh menus
#define kMenuBarResID                   128
#define kAppleMenuResID                 128
#define kFileMenuResID					129
#define kEditMenuResID					130
#define kEffectMenuResID				131
#define kSettingsMenuResID				132

// IDs for Apple menu items
#define kAboutMenuItem					1

// IDs for File and Edit menus and menu items
#define IDS_FILEMENU                    33024	// ((kFileMenuResID<<8)+(0))
#define IDM_FILENEW						33025
#define IDM_FILEOPEN                    33026
#define IDM_FILECLOSE                   33027
#define IDM_FILESAVE                 	33028
#define IDM_FILESAVEAS                 	33029
#define IDM_EXIT                        33031

#define IDS_EDITMENU                    33280	// ((kEditMenuResID<<8)+(0))
#define IDM_EDITUNDO                    33281
#define IDM_EDITCUT                     33283
#define IDM_EDITCOPY                    33284
#define IDM_EDITPASTE                   33285
#define IDM_EDITCLEAR                   33286
#define IDM_EDITSELECTALL               33288
#define IDM_EDITSELECTNONE				33289
#define IDM_PREFERENCES		            33291

#define IDS_EFFECT_MENU              	2
#define IDM_SELECT_EFFECT				33537	// ((kEffectMenuResID<<8)+(1))
#define IDM_RUN_EFFECT					33539	// ((kEffectMenuResID<<8)+(3))
#define IDM_STEP_AHEAD					33540	// ((kEffectMenuResID<<8)+(4))
#define IDM_STEP_BACK					33541	// ((kEffectMenuResID<<8)+(5))
#define IDM_MAKE_EFFECT_MOVIE			33543	// ((kEffectMenuResID<<8)+(7))
#define IDM_GET_FIRST_PICTURE			33545	// ((kEffectMenuResID<<8)+(9))
#define IDM_GET_SECOND_PICTURE			33546	// ((kEffectMenuResID<<8)+(10))

#define IDS_SETTINGS_MENU              	3
#define IDM_NO_LOOPING					33793	// ((kSettingsMenuResID<<8)+(1))
#define IDM_NORMAL_LOOPING				33794	// ((kSettingsMenuResID<<8)+(2))
#define IDM_PALINDROME_LOOPING			33795	// ((kSettingsMenuResID<<8)+(3))
#define IDM_STANDARD_DIALOG				33797	// ((kSettingsMenuResID<<8)+(5))
#define IDM_CUSTOM_DIALOG				33798	// ((kSettingsMenuResID<<8)+(6))
#define IDM_FAST_DISPLAY				33800	// ((kSettingsMenuResID<<8)+(8))
#define IDM_SUBPANELS					33802	// ((kSettingsMenuResID<<8)+(10))

// IDs for Window menu and menu items (Windows-only)
#define IDS_WINDOWMENU                  1300
#define IDM_WINDOWTILE                  1301
#define IDM_WINDOWCASCADE               1302
#define IDM_WINDOWCLOSEALL              1303
#define IDM_WINDOWICONS                 1304
#define IDM_WINDOWCHILD                 1310

// Windows-only resource IDs
#define IDS_HELPMENU                    1400
#define IDM_ABOUT                       1401
#define IDC_STATIC                      -1

#define IDS_APPNAME                     1
#define IDS_DESCRIPTION                 2
#define WINDOWMENU                      4
#define IDS_FILTERSTRING                3
#define IDS_SAVEONCLOSE              	2000
#define IDS_SAVEONQUIT              	2001
#define IDI_APPICON                     101
#define IDI_CHILDICON                   102
#define IDD_ABOUT                       103

