 /*
	File:		QTime.c

	Written by: Keith Gurganus

  	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
*/

#ifndef __QTIME__
#define __QTIME__

typedef struct{
	char			filename[255];
	Movie			theMovie;
	MovieController theMC;
	Boolean			movieOpened;
	HWND			theHwnd;
}MovieStuff;

BOOL OpenMovie(HWND hwnd, MovieStuff *movieStuff);
void CloseMovie(MovieStuff *movieStuff);
OSErr SaveMovie(MovieStuff *movieStuff);
OSErr SaveAsMovie(MovieStuff *movieStuff);
BOOL GetFile(char *fileName);

ComponentResult EditCut(MovieController mc);
ComponentResult EditCopy(MovieController mc);
ComponentResult EditPaste(MovieController mc);
ComponentResult EditClear(MovieController mc);
ComponentResult EditUndo(MovieController mc);
ComponentResult EditSelectAll(Movie movie, MovieController mc) ;

void SetWindowTitle(HWND hWnd, unsigned char *theFullPath);
void GetFileNameFromFullPath(unsigned char *theFullPath, unsigned char *fileName);
#endif