// SimplePlayer MFCView.h : interface of the CSimplePlayerMFCView class
//
/////////////////////////////////////////////////////////////////////////////
#include "CQuickTime.h"

class CSimplePlayerMFCView : public CView
{
protected: // create from serialization only
	CSimplePlayerMFCView();
	DECLARE_DYNCREATE(CSimplePlayerMFCView)

// Attributes
public:
	CSimplePlayerMFCDoc* GetDocument(); 
	CString	mfullPath;
	CQuickTime *pQuickTime;		// QuickTime object
	BOOL OpenMovie(void);		// Open a movie
	void CloseMovie(void);		// Close a movie


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimplePlayerMFCView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSimplePlayerMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSimplePlayerMFCView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SimplePlayer MFCView.cpp
inline CSimplePlayerMFCDoc* CSimplePlayerMFCView::GetDocument()
   { return (CSimplePlayerMFCDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
