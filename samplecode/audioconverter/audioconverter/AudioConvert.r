#include "Dialogs.r"
#include "Types.r"



#define rCustomGetFileDialog	1000				// resource ID for Std File DLOG and DITL
#define rAIFFMenuLabelRes		128					// resource ID for AIFF menu label



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// custom put file dialog

resource 'DLOG' (rCustomGetFileDialog, purgeable) {
	{0, 0, 228, 344}, dBoxProc, invisible, noGoAway, 0,	
	rCustomGetFileDialog, "", noAutoCenter
};

resource 'DITL' (rCustomGetFileDialog, purgeable) {
	{
/* [1] */	{161, 252, 181, 332}, Button {enabled, "Save"},
/* [2] */ 	{130, 252, 150, 332}, Button {enabled, "Cancel"},
/* [3] */ 	{0, 0, 0, 0}, HelpItem {disabled, HMScanhdlg {-6043}},
/* [4] */ 	{8, 235, 24, 337}, UserItem {enabled},
/* [5] */ 	{32, 252, 52, 332}, Button {enabled, "Eject"},
/* [6] */ 	{60, 252, 80, 332}, Button {enabled, "Desktop"},
/* [7] */ 	{29, 12, 127, 230}, UserItem {enabled},
/* [8] */ 	{6, 12, 25, 230}, UserItem {enabled},
/* [9] */ 	{119, 250, 120, 334}, Picture {disabled, 11},
/* [10] */ 	{157, 15, 173, 227}, EditText {enabled, ""},
/* [11] */ 	{136, 15, 152, 227}, StaticText {disabled, "Save as:"},
/* [12] */ 	{88, 252, 108, 332}, UserItem {disabled},
/* [13] */ 	{186, 100, 204, 230}, RadioButton {enabled, "AIFF"},
/* [14] */ 	{206, 100, 224, 230}, RadioButton {enabled, "QuickTime Movie"},
/* [15] */ 	{195, 13, 210, 93}, StaticText {disabled, "File Format:"}
	}
};


resource 'STR#' (rAIFFMenuLabelRes, purgeable) {
	{
		"AIFF File",
		".aiff",
		".mov",
		"AudioConvert"
	}
};


data 'carb' (0) {
	$"00000000"
};

