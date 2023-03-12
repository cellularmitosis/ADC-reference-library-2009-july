// SimpleEdit MFCView.cpp : implementation of the CSimpleEditMFCView class
//

#include "stdafx.h"
#include "SimpleEditMFC.h"

#include "SimpleEditMFCDoc.h"
#include "SimpleEditMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleEditMFCView

IMPLEMENT_DYNCREATE(CSimpleEditMFCView, CView)

BEGIN_MESSAGE_MAP(CSimpleEditMFCView, CView)
	//{{AFX_MSG_MAP(CSimpleEditMFCView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleEditMFCView construction/destruction

CSimpleEditMFCView::CSimpleEditMFCView()
{
	// Create a QuickTime object
	pQuickTime = new CQuickTime;

}

CSimpleEditMFCView::~CSimpleEditMFCView()
{
	// Destroy the QuickTime object
	if ( pQuickTime ) 
 		delete pQuickTime;
}

BOOL CSimpleEditMFCView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleEditMFCView drawing

void CSimpleEditMFCView::OnDraw(CDC* pDC)
{
	Movie	theMovie;
	CSimpleEditMFCDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (theMovie = pQuickTime->GetMovie())
		UpdateMovie(theMovie);
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleEditMFCView diagnostics

#ifdef _DEBUG
void CSimpleEditMFCView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleEditMFCView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleEditMFCDoc* CSimpleEditMFCView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleEditMFCDoc)));
	return (CSimpleEditMFCDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimpleEditMFCView message handlers

int CSimpleEditMFCView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Setup movie window	 
 	strcpy((char *)pQuickTime->theAppName, AfxGetAppName());  // store app name for window title

	pQuickTime->OnMovieWindowCreate( m_hWnd, lpCreateStruct);
	
	return 0;
}

void CSimpleEditMFCView::OnDestroy() 
{
	CView::OnDestroy();
	
	// Destroy movie window
	pQuickTime->OnMovieWindowDestroy();
}


BOOL CSimpleEditMFCView::OpenMovie(void)
{
	return pQuickTime->OpenMovie((unsigned char *)(LPCSTR)mfullPath);
}

void CSimpleEditMFCView::CloseMovie(void)
{
	pQuickTime->CloseMovie();
}


void CSimpleEditMFCView::OnFileNew() 
{
	pQuickTime->NewMovieFile();
	
}

void CSimpleEditMFCView::OnFileClose() 
{
	CSimpleEditMFCDoc* pDoc = GetDocument();

	pQuickTime->CloseMovie();

	pDoc->SetPathName( "Untitled", false );
}

void CSimpleEditMFCView::OnFileSaveAs() 
{
	pQuickTime->SaveAsMovie();
}

void CSimpleEditMFCView::OnEditUndo() 
{
	pQuickTime->OnEditUndo();
	
}

void CSimpleEditMFCView::OnEditCut() 
{
	pQuickTime->OnEditCut();
	
}

void CSimpleEditMFCView::OnEditCopy() 
{
	pQuickTime->OnEditCopy();	
}

void CSimpleEditMFCView::OnEditPaste() 
{
	pQuickTime->OnEditPaste();
}

void CSimpleEditMFCView::OnEditClear() 
{
	pQuickTime->OnEditClear();	
}

void CSimpleEditMFCView::OnEditSelectall() 
{
	pQuickTime->OnEditSelectall();	
}

LRESULT CSimpleEditMFCView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

void CSimpleEditMFCView::OnSize(UINT nType, int cx, int cy) 
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

