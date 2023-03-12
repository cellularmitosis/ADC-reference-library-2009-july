/*
	File:		CQuickTime.c

	Written by: Keith Gurganus

	Note: Set project settings to "Not using precompiled headers" for this file.
*/

#include "QTML.h"
#include "CQuickTime.h"

CQuickTime::CQuickTime()
{
	movieOpened = FALSE;
	theMovie = NULL;
    theMC = NULL;
	theHwnd = NULL;
	theViewHwnd = NULL;
	theAppName[0] = '\0';
	theFullPath[0] = '\0';
}

CQuickTime::~CQuickTime()
{
}

void CQuickTime::NewMovieFile(void){
	// Close any previously opened movie
	CloseMovie();

	// Set the port	
	SetGWorld((CGrafPtr)GetHWNDPort(theViewHwnd), nil);

	// Create a newMovie
	theMovie = NewMovie(newMovieActive);

	if ( theMovie != NULL ) {
		// set window title to name	
		strcpy((char*)theFullPath,"Untitled");
		SetWindowTitle();

		// Create the movie controller
		CreateNewMovieController(theMovie);
		
		movieOpened = TRUE;
	}	
}


BOOL CQuickTime::OpenMovie(unsigned char *fullPath)
{
	BOOL	isMovieGood = FALSE;

	if ( strlen ((char*)fullPath ) != 0)
	{
		OSErr				err;
		short				theFile = 0;
		long				controllerFlags = 0L;
		FSSpec				sfFile;
		short				movieResFile;

		// Close any previously opened movie
		CloseMovie();
	
		// make a copy of our full path name
		strcpy ( (char *)theFullPath, (const char *) fullPath );

		// convert theFullPath to pstring
		CToPstr((char*)theFullPath);

		// Make a FSSpec with a pascal string filename
		FSMakeFSSpec(0,0L,theFullPath, &sfFile);
		
		// Set the port	
		SetGWorld((CGrafPtr)GetHWNDPort(theViewHwnd), nil);

		// Open the movie file
		err = OpenMovieFile(&sfFile, &movieResFile, fsRdPerm);
		if (err == noErr)
		{
			// Get the Movie from the file
			err = NewMovieFromFile(&theMovie,movieResFile, 
									nil, 
									nil, 
									newMovieActive, /* flags */
									nil);
		
			// Close the movie file
			CloseMovieFile(movieResFile);

			if (err == noErr)
			{
				// Create the movie controller
			   	CreateNewMovieController(theMovie);
				isMovieGood = movieOpened = TRUE;	
				PToCstr((char*)theFullPath);
			} else
				theFullPath[0] = '\0'; 
				
		} else
			theFullPath[0] = '\0';
		
		SetWindowTitle();
	}
	return isMovieGood;
}

void CQuickTime::CloseMovie(void)
{
	if (movieOpened == TRUE ){
		movieOpened = FALSE;
 	
		if (theMC)
			DisposeMovieController(theMC);

		if (theMovie)
			DisposeMovie(theMovie);

		theMovie = NULL;
		theMC = NULL;
	}

	Rect initialRect;
	initialRect.bottom = MINWINDOWHEIGHT;
	initialRect.right = MINWINDOWWIDTH;
	
	SizeWindow((WindowPtr)GetHWNDPort(theViewHwnd), initialRect.right, initialRect.bottom, FALSE);

	/* set window title to empty name */
	theFullPath[0] ='\0';
	SetWindowTitle ();
}


void CQuickTime::SaveAsMovie(void){
	unsigned char	lpszPathName[256];
	OPENFILENAME	ofn;

    memset(&ofn, 0, sizeof(OPENFILENAME));
	lpszPathName[0] = '\0';

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ::GetActiveWindow ();
    ofn.lpstrFilter = "QuickTime Movies (*.mov) \0 *.mov\0";
    ofn.lpstrFile = (char *)lpszPathName;
    ofn.nMaxFile = sizeof(lpszPathName);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = (unsigned long)NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_OVERWRITEPROMPT;

	// Put up a save file dialog
    if (GetSaveFileName(&ofn)) {
		long			movieFlattenFlags = flattenAddMovieToDataFork;
		FSSpec			sfFile;
		OSType			creator = FOUR_CHAR_CODE('TVOD');
		long			createMovieFlags = createMovieFileDeleteCurFile;

		// Convert full path name to pstring
		CToPstr((char *)lpszPathName);	

		// Make a FSSpec with a pascal string filename
		FSMakeFSSpec(0,0L,lpszPathName, &sfFile);

		// FlattenMovie
		FlattenMovie(	theMovie,
						movieFlattenFlags,
						&sfFile,
						creator,
						-1,
						createMovieFlags,
						nil,
						NULL );
	
	}
}

void CQuickTime::OnEditCut() 
{
	Movie				scrapMovie;
	ComponentResult		theErr = noErr;
	
	if (theMC){
		scrapMovie = MCCut(theMC);
		if ( scrapMovie ) {
			theErr = PutMovieOnScrap(scrapMovie, 0L);
			DisposeMovie(scrapMovie);
		} 
	}
}

void CQuickTime::OnEditCopy() 
{
	Movie				scrapMovie;
	ComponentResult		theErr = noErr;
	
	if (theMC){
		scrapMovie = MCCopy(theMC);
		if ( scrapMovie ) {
			theErr = PutMovieOnScrap(scrapMovie, 0L);
			DisposeMovie(scrapMovie);
		}
	}
}

void CQuickTime::OnEditPaste() 
{
	if (theMC)
		MCPaste(theMC, nil);
}

void CQuickTime::OnEditClear() 
{
	if (theMC)
		MCClear(theMC);
}

void CQuickTime::OnEditUndo() 
{
	if (theMC)
		MCUndo(theMC);
}


void CQuickTime::OnEditSelectall() 
{
	TimeRecord 			tr;
	ComponentResult		theErr = noErr;
	
	if ( theMovie && theMC ) {
		tr.value.hi = 0;
		tr.value.lo = 0;
		tr.base = 0;
		tr.scale = GetMovieTimeScale(theMovie);
		MCDoAction(theMC, mcActionSetSelectionBegin, &tr);
		tr.value.lo = GetMovieDuration(theMovie);
		MCDoAction(theMC, mcActionSetSelectionDuration, &tr);
	} else {
		if ( theMovie == NULL )
			theErr = invalidMovie;
		else
			theErr = -1;
	}	
	
}

void CQuickTime::ProcessMovieEvent(HWND hWnd, unsigned int message, unsigned int wParam, long lParam) 
{	
	// Convert the Windows event to a QTML event
	MSG				theMsg;
	EventRecord		macEvent;
	LONG			thePoints = GetMessagePos();

    theMsg.hwnd = hWnd;
    theMsg.message = message;
    theMsg.wParam = wParam;
    theMsg.lParam = lParam;
    theMsg.time = GetMessageTime();
    theMsg.pt.x = LOWORD(thePoints);
    theMsg.pt.y = HIWORD(thePoints);

	// tranlate a windows event to a mac event
	WinEventToMacEvent(&theMsg, &macEvent);

	// Pump messages as mac event
    MCIsPlayerEvent(theMC,(const EventRecord *)&macEvent);
}

int CQuickTime::OnMovieWindowCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct) 
{

	if ( hWnd != NULL) { 
		theViewHwnd = hWnd;					// the view's hwnd
		theHwnd = ::GetParent(theViewHwnd);	// the parent hwnd	 hwndParent

		int borderWidth = GetWindowsBorderWidth();
		int titlebarHeight = GetWindowsTitleHeight();
		int captionHeight = GetWindowsCaptionHeight();	

		lpCreateStruct->cy = borderWidth + titlebarHeight + captionHeight;
		lpCreateStruct->cx = MINWINDOWWIDTH;

		SetWindowTitle();
		Rect initialRect;
		initialRect.top = initialRect.left = initialRect.bottom = 0;
		initialRect.right = MINWINDOWWIDTH;

		// Create GrafPort <-> HWND association
		CreatePortAssociation(theViewHwnd, NULL, 0);	
	}

	return 0;
}
 
void CQuickTime::CreateNewMovieController(Movie theMovie)
{
	Rect	bounds;
	Rect	maxBounds;
	long 	controllerFlags;

	// 0,0 Movie coordinates
	GetMovieBox(theMovie, &theMovieRect);
	MacOffsetRect(&theMovieRect, -theMovieRect.left, -theMovieRect.top);

	// Attach a movie controller
	theMC = NewMovieController(theMovie, &theMovieRect, mcTopLeftMovie );

	// Get the controller rect 
	MCGetControllerBoundsRect(theMC, &bounds);

	// Enable editing
	MCEnableEditing(theMC,TRUE);

	// Tell the controller to attach a movie's CLUT to the window as appropriate.
	MCDoAction(theMC, mcActionGetFlags, &controllerFlags);
	MCDoAction(theMC, mcActionSetFlags, (void *)(controllerFlags | mcFlagsUseWindowPalette));

	// Allow the controller to accept keyboard events
	MCDoAction(theMC, mcActionSetKeysEnabled, (void *)TRUE);

	// Set the controller action filter
	MCSetActionFilterWithRefCon(theMC, MCFilter, (long)theViewHwnd);

	// Set the grow box amound
	GetMaxBounds(&maxBounds);
	MCDoAction(theMC, mcActionSetGrowBoxBounds, &maxBounds);

	// Size our window
	SizeWindow((WindowPtr)GetHWNDPort(theViewHwnd), bounds.right, bounds.bottom, FALSE);
}

void CQuickTime::OnMovieWindowDestroy() 
{	
	CGrafPtr	windowPort = NULL;
	
	// close any movies	before destroying PortAssocation
	CloseMovie();

	// Destroy the view's GrafPort <-> HWND association
	if (theViewHwnd)
		windowPort = (CGrafPtr)GetHWNDPort(theViewHwnd);
	
	if (windowPort)
		DestroyPortAssociation(windowPort);

}

void CQuickTime::SetWindowTitle(void) 
{
	/* set window title to name */
	unsigned char	fileName[64];
	unsigned char	appName[32];
	unsigned char	titleName[256];

	fileName[0] = '\0';
	appName[0] = '\0';
	titleName[0] = '\0';

	GetAppName((unsigned char *)&appName);
	if (strlen((char*)appName))
		strcpy ((char *)titleName, (char *)appName);

	GetFileNameFromFullPath((unsigned char *)&fileName);
	if (strlen((char*)fileName)){
		if (strlen((char*)appName))
			strcat ((char *)titleName, " - ");

		strcat ((char *)titleName, (char *)fileName);
	}
	::SetWindowText(theHwnd, (const char *)titleName);
}

void CQuickTime::GetFileNameFromFullPath(unsigned char *fileName) 
{
	/* pluck the filename from the fullpath, */
	int		i = 0, j = -1, stringLen = 0;

	stringLen = strlen((char *)theFullPath);
	if (stringLen > 0 ) {
		while(i<stringLen){
			if (theFullPath[i] == 0x5c || theFullPath[i] == '/' )
				j = i;
			i++;
		}
		if ( j>-1)
			strcpy((char *)fileName, (char *)&theFullPath[j+1]);
		else
			strcpy((char *)fileName, (char *)theFullPath);

	}
}

void CQuickTime::GetAppName(unsigned char *appName)
{ 
#if (!WIN32)
	HINSTANCE theModule = (HINSTANCE)GetWindowLong(theHwnd, GWL_HINSTANCE);
	GetModuleFileName(theModule, (char *)appName, strlen((char *)appName) );
#else
	if (strlen((char*)theAppName) > 0)
		strcpy((char*)appName, (char*)theAppName);
#endif
}

void CQuickTime::CToPstr(char *theString)
{
	char	tempString[256];

	tempString[0] = strlen (theString);
	tempString[1] = '\0';

	strcat ( tempString, theString );
	strcpy	( theString, tempString );
}

void CQuickTime::PToCstr(char *theString)
{
	char	tempString[256];
	int		len = theString[0];

	memcpy ( tempString, &theString[1], theString[0]);
	tempString[len] = '\0';
	strcpy	( theString, tempString );
}

int	CQuickTime::GetWindowsBorderWidth (void)
{
	RECT	windowRect, clientRect;
	int		windowWidth, clientWidth;
	
	::GetWindowRect(theHwnd, &windowRect);
	::GetClientRect(theHwnd, &clientRect);

	windowWidth = windowRect.right - windowRect.left;
	clientWidth = clientRect.right - clientRect.left;

	return windowWidth - clientWidth;
}

int	CQuickTime::GetWindowsTitleHeight (void)
{
	RECT	windowRect, clientRect;
	int		windowHeight, clientHeight;
	int		windowWidth, clientWidth, bordersWidth;
	int		captionHeight, titlebarHeight;
	
	::GetWindowRect(theHwnd, &windowRect);
	::GetClientRect(theHwnd, &clientRect);

	windowWidth = windowRect.right - windowRect.left;
	clientWidth = clientRect.right - clientRect.left;
	bordersWidth = windowWidth - clientWidth;

	windowHeight = windowRect.bottom - windowRect.top;
	clientHeight = clientRect.bottom - clientRect.top;
	captionHeight = GetSystemMetrics(SM_CYCAPTION);	
	titlebarHeight = (windowHeight - clientHeight) - captionHeight;

	return titlebarHeight;
}

int CQuickTime::GetWindowsCaptionHeight(void)
{
 	DWORD	dwStyle = 0L;
	int		retValue = 0L;

	if(!IsWindow(theHwnd))
		return 0L;

	dwStyle = GetWindowLong(theHwnd, GWL_STYLE);

	if (dwStyle & WS_CAPTION)
		retValue = GetSystemMetrics(SM_CYCAPTION);

	return retValue;
}

void CQuickTime::GetMaxBounds(Rect *maxRect)
{
	RECT deskRect;

	GetWindowRect(GetDesktopWindow(), &deskRect);

	OffsetRect(&deskRect, -deskRect.left, -deskRect.top);

	maxRect->top = (short)deskRect.top;
	maxRect->bottom = (short)deskRect.bottom;
	maxRect->left = (short)deskRect.left;
	maxRect->right = (short)deskRect.right;
}

Movie CQuickTime::GetMovie()
{
	return theMovie;
}

Boolean MCFilter(MovieController mc, short action, void*params, long refCon)
{
	if(action == mcActionControllerSizeChanged) {
		Rect		bounds;
		WindowPtr	w;
		MCGetControllerBoundsRect(mc, &bounds);

		w = GetHWNDPort((HWND)refCon);
		SizeWindow((WindowPtr)w, bounds.right, bounds.bottom, TRUE);
	}
	return FALSE;
}




