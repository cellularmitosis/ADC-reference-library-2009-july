 /*
	File:		QTime.c

	Written by: Keith Gurganus

  	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
*/

#include "QTML.h"
#include "Movies.h"
#include "stdwin.h"
#include "QTime.h"
#include "TextUtils.h"

#define USEEXPLORERSTYLE (LOBYTE(LOWORD(GetVersion()))>=4)

static UINT APIENTRY GenericHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateNewMovieController(HWND hwnd, Movie theMovie, MovieController *theMC);
Boolean MCFilter(MovieController mc, short action, void*params, long refCon);
void GetMaxBounds(Rect *maxRect);

BOOL GetFile(char *fileName)
{
    OPENFILENAME    ofn;

    memset(&ofn, 0, sizeof(OPENFILENAME));
	*fileName = '\0';
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = (LPSTR)fileName;
    ofn.nMaxFile  = 255;
	ofn.lpstrFilter  = "QuickTime Movies (*.mov;*.avi) \0 *.mov;*.avi\0All Files (*.*) \0 *.*\0";	// Robert requested *.* to be added 07/10/96 BSF
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
	if(USEEXPLORERSTYLE)
		ofn.Flags |= OFN_ENABLEHOOK | OFN_EXPLORER;
	else
		ofn.Flags |= OFN_ENABLEHOOK;
	ofn.lpfnHook = GenericHook;

    if (GetOpenFileName(&ofn))
    {
        return TRUE;
	} else
        return FALSE;
}


static UINT APIENTRY GenericHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:		// Center window
		{
			Point	ptTopLeft;
			RECT	rcWindow;
			BOOL	retValue;
			HWND	theWnd = hWnd;
			RECT	rcDesktopWindow;
			long	width;
			long	height;

			// if we are using 95 or NT 4.0 use the new explorer style
			if(USEEXPLORERSTYLE)
				theWnd = GetParent(hWnd);

			GetWindowRect(theWnd, &rcWindow);

			width = rcWindow.right - rcWindow.left;
			height = rcWindow.bottom - rcWindow.top;
			GetWindowRect(GetDesktopWindow(), &rcDesktopWindow);
			ptTopLeft.h = (short)((rcDesktopWindow.right + rcDesktopWindow.left)/2 - width/2);
			ptTopLeft.v = (short)((rcDesktopWindow.top + rcDesktopWindow.bottom)/3 - height/3);
		
			retValue = SetWindowPos(theWnd, 0, ptTopLeft.h, ptTopLeft.v, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	
			return true;
		}
	}
	return 0;
}

BOOL OpenMovie(HWND hwnd, MovieStuff *movieStuff)
{
	BOOL	isMovieGood = FALSE;

	if ( strlen ((char*)movieStuff->filename ) != 0)
	{
		OSErr				err;
		short				theFile = 0;
		long				controllerFlags = 0L;
		FSSpec				sfFile;
		short				movieResFile;
		char				theFullPath[255];

		// make a copy of our full path name
		strcpy ( (char *)theFullPath, (const char *) movieStuff->filename );

		// convert theFullPath to pstring
		c2pstr((char*)theFullPath);

		// Make a FSSpec with a pascal string filename
		FSMakeFSSpec(0,0L,theFullPath, &sfFile);
		
		// Set the port	
		SetGWorld((CGrafPtr)GetNativeWindowPort((void *)hwnd), nil);

		// Open the movie file
		err = OpenMovieFile(&sfFile, &movieResFile, fsRdPerm);
		if (err == noErr)
		{
			// Get the Movie from the file
			err = NewMovieFromFile(&movieStuff->theMovie,movieResFile, 
									nil, 
									nil, 
									newMovieActive, /* flags */
									nil);
		
			// Close the movie file
			CloseMovieFile(movieResFile);

			if (err == noErr)
			{
				// Create the movie controller
			   	CreateNewMovieController(hwnd, movieStuff->theMovie, &movieStuff->theMC);
				movieStuff->movieOpened = TRUE;
				isMovieGood = TRUE;	
				p2cstr((char*)theFullPath);

				SetWindowTitle(movieStuff->theHwnd, theFullPath);
			} else
				theFullPath[0] = '\0'; 
				
		} else
			theFullPath[0] = '\0';
	}

	return isMovieGood;
}

OSErr SaveMovie(MovieStuff *movieStuff)
{
	OSErr			theErr = noErr;
	if (strlen(movieStuff->filename) != 0) {
		long			movieFlattenFlags = flattenAddMovieToDataFork;
		FSSpec			sfFile;
		OSType			creator = FOUR_CHAR_CODE('TVOD');
		long			createMovieFlags = createMovieFileDeleteCurFile;
		char			theFullPath[255];

		strcpy(theFullPath, movieStuff->filename);

		// Convert full path name to pstring
		c2pstr((char *)theFullPath);	

		// Make a FSSpec with a pascal string filename
		FSMakeFSSpec(0,0L,theFullPath, &sfFile);

		// Try to delete the original
		DeleteMovieFile(&sfFile);		

		// FlattenMovie
		FlattenMovie(	movieStuff->theMovie,
						movieFlattenFlags,
						&sfFile,
						creator,
						-1,
						createMovieFlags,
						nil,
						NULL );
	
		theErr = GetMoviesError();
	}
	return theErr;
}

OSErr SaveAsMovie(MovieStuff *movieStuff)
{
	unsigned char	lpszPathName[256];
	OPENFILENAME	ofn;
	OSErr			theErr = noErr;

    memset(&ofn, 0, sizeof(OPENFILENAME));
	lpszPathName[0] = '\0';

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = GetActiveWindow();
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
		c2pstr((char *)lpszPathName);	

		// Make a FSSpec with a pascal string filename
		FSMakeFSSpec(0,0L,lpszPathName, &sfFile);

		// Try to delete the original
		DeleteMovieFile(&sfFile);		

		// FlattenMovie
		FlattenMovie(	movieStuff->theMovie,
						movieFlattenFlags,
						&sfFile,
						creator,
						-1,
						createMovieFlags,
						nil,
						NULL );

		theErr = GetMoviesError();
		p2cstr((char *)lpszPathName);
		SetWindowTitle(movieStuff->theHwnd, lpszPathName);
	}
	return theErr;
}

void CloseMovie(MovieStuff *movieStuff)
{
	if (movieStuff->movieOpened == TRUE ){
		movieStuff->movieOpened = FALSE;
 	
		if (movieStuff->theMC)
			DisposeMovieController(movieStuff->theMC);

		if (movieStuff->theMovie)
			DisposeMovie(movieStuff->theMovie);

		movieStuff->theMovie = NULL;
		movieStuff->theMC = NULL;
	}
}


void CreateNewMovieController(HWND hwnd, Movie theMovie, MovieController *theMC)
{
	Rect	bounds;
	Rect	maxBounds;
	long 	controllerFlags;
	Rect	theMovieRect;

	// 0,0 Movie coordinates
	GetMovieBox(theMovie, &theMovieRect);
	MacOffsetRect(&theMovieRect, -theMovieRect.left, -theMovieRect.top);

	// Attach a movie controller
	*theMC = NewMovieController(theMovie, &theMovieRect, mcTopLeftMovie );

	// Get the controller rect 
	MCGetControllerBoundsRect(*theMC, &bounds);

	// Enable editing
	MCEnableEditing(*theMC,TRUE);

	// Tell the controller to attach a movie's CLUT to the window as appropriate.
	MCDoAction(*theMC, mcActionGetFlags, &controllerFlags);
	MCDoAction(*theMC, mcActionSetFlags, (void *)(controllerFlags | mcFlagsUseWindowPalette));

	// Allow the controller to accept keyboard events
	MCDoAction(*theMC, mcActionSetKeysEnabled, (void *)TRUE);

	// Set the controller action filter
	MCSetActionFilterWithRefCon(*theMC, MCFilter, (long)hwnd);

	// Set the grow box amount
	GetMaxBounds(&maxBounds);
	MCDoAction(*theMC, mcActionSetGrowBoxBounds, &maxBounds);

	// Size our window
	SizeWindow((WindowPtr)GetNativeWindowPort(hwnd), bounds.right, bounds.bottom, FALSE);
}


Boolean MCFilter(MovieController mc, short action, void*params, long refCon)
{
	if(action == mcActionControllerSizeChanged) {
		Rect		bounds;
		WindowPtr	w;
		MCGetControllerBoundsRect(mc, &bounds);

		w = GetNativeWindowPort((HWND)refCon);
		SizeWindow((WindowPtr)w, bounds.right, bounds.bottom, TRUE);
	}
	return FALSE;
}

void GetMaxBounds(Rect *maxRect)
{
	RECT deskRect;

	GetWindowRect(GetDesktopWindow(), &deskRect);

	OffsetRect(&deskRect, -deskRect.left, -deskRect.top);

	maxRect->top = (short)deskRect.top;
	maxRect->bottom = (short)deskRect.bottom;
	maxRect->left = (short)deskRect.left;
	maxRect->right = (short)deskRect.right;
}

ComponentResult EditCut(MovieController mc) 
{
	Movie				scrapMovie;
	ComponentResult		theErr = invalidMovie;
	
	if (mc){
		scrapMovie = MCCut(mc);
		if ( scrapMovie ) {
			theErr = PutMovieOnScrap(scrapMovie, 0L);
			DisposeMovie(scrapMovie);
		} 
	}
	return theErr;
}

ComponentResult EditCopy(MovieController mc) 
{
	Movie				scrapMovie;
	ComponentResult		theErr = invalidMovie;
	
	if (mc){
		scrapMovie = MCCopy(mc);
		if ( scrapMovie ) {
			theErr = PutMovieOnScrap(scrapMovie, 0L);
			DisposeMovie(scrapMovie);
		}
	}
	return theErr;
}

ComponentResult EditPaste(MovieController mc) 
{
	ComponentResult		theErr = invalidMovie;

	if (mc)
		theErr = MCPaste(mc, nil);

	return theErr;
}

ComponentResult EditClear(MovieController mc) 
{
	ComponentResult		theErr = invalidMovie;
	
	if (mc)
		theErr = MCClear(mc);

	return theErr;
}

ComponentResult EditUndo(MovieController mc) 
{
	ComponentResult		theErr = invalidMovie;

	if (mc)
		theErr = MCUndo(mc);
	
	return theErr;
}

ComponentResult EditSelectAll(Movie movie, MovieController mc) 
{
	TimeRecord 			tr;
	ComponentResult		theErr = noErr;
	
	if ( movie && mc ) {
		tr.value.hi = 0;
		tr.value.lo = 0;
		tr.base = 0;
		tr.scale = GetMovieTimeScale(movie);
		MCDoAction(mc, mcActionSetSelectionBegin, &tr);
		tr.value.lo = GetMovieDuration(movie);
		MCDoAction(mc, mcActionSetSelectionDuration, &tr);
	} else {
		if ( movie == NULL )
			theErr = invalidMovie;
		else
			theErr = -1;
	}
	return theErr;
}

void SetWindowTitle(HWND hWnd, unsigned char *theFullPath) 
{
	/* set window title to name */
	unsigned char	titleName[256];
	titleName[0] = '\0';

	GetFileNameFromFullPath(theFullPath, (unsigned char *)&titleName);
	SetWindowText(hWnd, (const char *)titleName);
}

void GetFileNameFromFullPath(unsigned char *theFullPath, unsigned char *fileName) 
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
