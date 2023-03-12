// MFCMDIPlayerView.cpp : implementation of the CMFCMDIPlayerView class
//

#include "stdafx.h"
#include "MFCMDIPlayer.h"

#include "MFCMDIPlayerDoc.h"
#include "MFCMDIPlayerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView

IMPLEMENT_DYNCREATE(CMFCMDIPlayerView, CView)

BEGIN_MESSAGE_MAP(CMFCMDIPlayerView, CView)
	//{{AFX_MSG_MAP(CMFCMDIPlayerView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView construction/destruction

CMFCMDIPlayerView::CMFCMDIPlayerView()
{
	// Create a QuickTime object
	pQuickTime = new CQuickTime;
}

CMFCMDIPlayerView::~CMFCMDIPlayerView()
{
	// Destroy the QuickTime object
	if ( pQuickTime ) 
 		delete pQuickTime;
}

BOOL CMFCMDIPlayerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView drawing

void CMFCMDIPlayerView::OnDraw(CDC* pDC)
{
	Movie	theMovie;
	CMFCMDIPlayerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (theMovie = pQuickTime->GetMovie())
		UpdateMovie(theMovie);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView printing

BOOL CMFCMDIPlayerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMFCMDIPlayerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMFCMDIPlayerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView diagnostics

#ifdef _DEBUG
void CMFCMDIPlayerView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCMDIPlayerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCMDIPlayerDoc* CMFCMDIPlayerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCMDIPlayerDoc)));
	return (CMFCMDIPlayerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerView message handlers

BOOL CMFCMDIPlayerView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

int CMFCMDIPlayerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Setup movie window	 
 	strcpy((char *)pQuickTime->theAppName, AfxGetAppName());  // store app name for window title

	pQuickTime->OnMovieWindowCreate( m_hWnd, lpCreateStruct);
	
	return 0;
}

void CMFCMDIPlayerView::OnDestroy() 
{

	CView::OnDestroy();
		// Destroy movie window
	pQuickTime->OnMovieWindowDestroy();	

}


BOOL CMFCMDIPlayerView::OpenMovie(void)
{
	return pQuickTime->OpenMovie((unsigned char *)(LPCSTR)mfullPath);
}

void CMFCMDIPlayerView::CloseMovie(void)
{
	pQuickTime->CloseMovie();
}

LRESULT CMFCMDIPlayerView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	if(message == WM_ERASEBKGND){

		LRESULT theResult = CView::WindowProc(message, wParam, lParam);
		pQuickTime->ProcessMovieEvent (m_hWnd, message, wParam, lParam);	
		return theResult;
	} else {
		pQuickTime->ProcessMovieEvent (m_hWnd, message, wParam, lParam);
		return CView::WindowProc(message, wParam, lParam);
	}
}

void CMFCMDIPlayerView::OnSize(UINT nType, int cx, int cy) 
{	
	CView::OnSize(nType, cx, cy);

	if (cy && cx)
	{
		// calculate the size of the frame
		CFrameWnd* pFrame = GetParentFrame();
		if (pFrame != NULL)
		{
			CRect rectSized(0, 0, cx, cy);
			pFrame->CalcWindowRect(rectSized);
			pFrame->SetWindowPos(this,0,0,rectSized.Width() + 4,rectSized.Height()+ 4, SWP_NOZORDER | SWP_NOMOVE );

		}
	}
}
