// MFCMDIPlayerView.h : interface of the CMFCMDIPlayerView class
//
/////////////////////////////////////////////////////////////////////////////
#include "CQuickTime.h"

class CMFCMDIPlayerView : public CView
{
protected: // create from serialization only
	CMFCMDIPlayerView();
	DECLARE_DYNCREATE(CMFCMDIPlayerView)

// Attributes
public:
	CMFCMDIPlayerDoc* GetDocument();
	CString	mfullPath;
	CQuickTime *pQuickTime;		// QuickTime object
	BOOL OpenMovie(void);		// Open a movie
	void CloseMovie(void);		// Close a movie

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCMDIPlayerView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMFCMDIPlayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMFCMDIPlayerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MFCMDIPlayerView.cpp
inline CMFCMDIPlayerDoc* CMFCMDIPlayerView::GetDocument()
   { return (CMFCMDIPlayerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
