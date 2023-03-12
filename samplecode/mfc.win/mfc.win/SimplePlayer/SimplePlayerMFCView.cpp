// SimplePlayer MFCView.cpp : implementation of the CSimplePlayerMFCView class
//

#include "stdafx.h"
#include "SimplePlayerMFC.h"

#include "SimplePlayerMFCDoc.h"
#include "SimplePlayerMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCView

IMPLEMENT_DYNCREATE(CSimplePlayerMFCView, CView)

BEGIN_MESSAGE_MAP(CSimplePlayerMFCView, CView)
	//{{AFX_MSG_MAP(CSimplePlayerMFCView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCView construction/destruction

CSimplePlayerMFCView::CSimplePlayerMFCView()
{
	// Create a QuickTime object
	pQuickTime = new CQuickTime;

}

CSimplePlayerMFCView::~CSimplePlayerMFCView()
{
	// Destroy the QuickTime object
	if ( pQuickTime ) 
 		delete pQuickTime;
}

BOOL CSimplePlayerMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCView drawing

void CSimplePlayerMFCView::OnDraw(CDC* pDC)
{
	Movie	theMovie;
	CSimplePlayerMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (theMovie = pQuickTime->GetMovie())
		UpdateMovie(theMovie);
}

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCView diagnostics

#ifdef _DEBUG
void CSimplePlayerMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CSimplePlayerMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimplePlayerMFCDoc* CSimplePlayerMFCView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimplePlayerMFCDoc)));
	return (CSimplePlayerMFCDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCView message handlers

int CSimplePlayerMFCView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Setup movie window	 
 	strcpy((char *)pQuickTime->theAppName, AfxGetAppName());  // store app name for window title

	pQuickTime->OnMovieWindowCreate( m_hWnd, lpCreateStruct);
	
	return 0;
}

void CSimplePlayerMFCView::OnDestroy() 
{
	CView::OnDestroy();
	
	// Destroy movie window
	pQuickTime->OnMovieWindowDestroy();	
}


BOOL CSimplePlayerMFCView::OpenMovie(void)
{
	return pQuickTime->OpenMovie((unsigned char *)(LPCSTR)mfullPath);
}

void CSimplePlayerMFCView::CloseMovie(void)
{
	CSimplePlayerMFCDoc* pDoc = GetDocument();

	pQuickTime->CloseMovie();

	pDoc->SetPathName( "Untitled", false );
}

LRESULT CSimplePlayerMFCView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

void CSimplePlayerMFCView::OnSize(UINT nType, int cx, int cy) 
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
			pFrame->SetWindowPos(this,0,0,rectSized.Width() + 4,rectSized.Height()+ GetSystemMetrics(SM_CYMENU) + 4, SWP_NOZORDER | SWP_NOMOVE );

		}
	}
	
}
