// CQuickTime.h : interface of the CQuickTime class
//
/////////////////////////////////////////////////////////////////////////////

#include <Movies.h>
#include <scrap.h>

#include <windows.h>	// Microsoft Windows
#include <winuser.h>

#ifndef __CQUICKTIME__
#define __CQUICKTIME__

#ifdef __cplusplus
extern "C" {
#endif
Boolean MCFilter(MovieController mc, short action, void*params, long refCon);
#ifdef __cplusplus
}
#endif

#define MINWINDOWWIDTH		320

class CQuickTime
{

public:
	CQuickTime();
	~CQuickTime();

	virtual BOOL	OpenMovie(unsigned char *fullPath);
	virtual void	CloseMovie(void);
	virtual void	NewMovieFile(void);
	virtual void	SaveAsMovie(void);
	virtual void	ProcessMovieEvent(HWND hWnd, unsigned int message, unsigned int wParam, long lParam); 
	virtual int		OnMovieWindowCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct); 
	virtual void	OnMovieWindowDestroy();
	virtual void	CreateNewMovieController(Movie theMovie);

  	virtual void	OnEditCut(void);
	virtual void	OnEditCopy(void);
	virtual void	OnEditPaste(void);
	virtual void	OnEditClear(void);
	virtual void	OnEditUndo(void);
	virtual void	OnEditSelectall(void);

	virtual void	SetWindowTitle(void);
	virtual void	GetFileNameFromFullPath(unsigned char *fileName); 
	virtual void	GetAppName(unsigned char *appName); 

	virtual void	CToPstr(char *theString);
	virtual void	PToCstr(char *theString);

	virtual int		GetWindowsBorderWidth (void);
	virtual int		GetWindowsTitleHeight (void);
	virtual int		GetWindowsCaptionHeight(void);
	virtual void	GetMaxBounds(Rect *maxRect);

	virtual Movie	GetMovie(void);

public:
	unsigned char	theAppName[128];

private: 
	BOOL			movieOpened;
	Movie			theMovie;
    MovieController theMC;
	Rect			theMovieRect;
	Rect			theMCRect;
	unsigned char	theFullPath[255];

	HWND			theHwnd;
	HWND			theViewHwnd;

// Operations

};

#endif
