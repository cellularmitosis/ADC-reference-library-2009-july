/*
	File:		FX.h

	Contains:	

	Written by: EL	

	Copyright:	Copyright © 1992-1999 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Change History (most recent first):
        08/2000		JM				Carbonized, non-Carbon code is commented out
                                                        for demonstration purposes.
        7/13/1999	KG				Updated for Metrowerks Codewarror Pro 2.1
				

*/
#include "CarbonPrefix.h"

#define		numTItems		8
#define		numAItems		8
#define		numCItems		4
#define		numMItems		3
#define		numDItems		2
#define		numBItems		3
#define		numPItems		2
#define		numLItems		2

enum {
	transferID = 1,
	arithmeticID,
	ditherID,
	colorizationID,
	searchProcID,
	paintBucketID,
	lassoID,
	pixelAverageID,
	customID
};

typedef	struct {
	Str255	label;
	Rect	rect;
} itemType;

typedef struct {
	int		tItem;
	int		aItem;
	int		cItem;
	int		mItem;
	int		dItem;
	int		bItem;
	int		pItem;
	int		lItem;
} itemSettings;

extern	WindowPtr		gWindow;
extern	GWorldPtr		gGWorld;
extern	int				gCurrentExample;
extern	itemSettings	settings;

extern	itemType	tItem[numTItems];
extern	itemType	aItem[numAItems];
extern	itemType	cItem[numCItems];
extern	itemType	dItem[numDItems];
extern	itemType	mItem[numMItems];
extern	itemType	bItem[numBItems];
extern	itemType	pItem[numPItems];
extern	itemType	lItem[numLItems];

void initMac();
void resetItems();
void createWindow();
void createOffscreen(int pictItem);
void defineItems();
void updateWindow();
void drawBackground();
void drawAllItems();
void drawSourceImage();
long drawFXImage();

void drawTransferItems();
void drawArithmeticItems();
void drawColorizeItems();
void drawDitherItems();
void drawMappingItems();
void drawDestinationItems();
void drawPaintBucketItems();
void drawLassoToolItems();

void drawExampleName();
void drawTime(long ticks);

void drawName(int left, int top, Str255 name );
void drawItem(int left, int top, Str255 *label, Boolean listEnabled, Boolean itemEnabled );

void drawDeepBox(Rect *rect);
void eraseRect(Rect *rect);

void eventLoop();

void setMenuItem( MenuHandle menu, int itemNum, Boolean enabled );
void adjustMenus();
void handleMenu(long mSelect);

void doAboutBox();
void drawMyString( int col, int *row, int increment, Str255 string);
void setUpMenus();

void transferExample(Rect *rect, int item);
void arithmeticExample(Rect *rect, int item);
void colorizationExample(Rect	* rect, int item);
void ditherExample(Rect	* rect,int item);
void mappingExample(Rect * rect, int	item);
void paintBucketExample(Rect *rect, int exampleItem, Point point);
void lassoToolExample(Rect	* rect, int	exampleItem,Rect *frame);
void pixelAverageExample(Rect	*rect, int	 item);
void customExample(Rect	* rect,int	item);

GrafPtr	CreateGrafPort(Rect *bounds);
void	DisposeGrafPort(GrafPtr doomedPort);
int setTransferMode(int	 item );
int setArithmeticMode(int item);
int setDitherMode(int item);
pascal Boolean matchProc(RGBColor *color, long		*position );
pascal Boolean searchProc(RGBColor	* color, long		*position);