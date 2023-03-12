// SimpleEdit MFCView.h : interface of the CSimpleEditMFCView class
//
/////////////////////////////////////////////////////////////////////////////
#include "CQuickTime.h"

class CSimpleEditMFCView : public CView
{
protected: // create from serialization only
	CSimpleEditMFCView();
	DECLARE_DYNCREATE(CSimpleEditMFCView)

// Attributes
public:
	CSimpleEditMFCDoc* GetDocument();
	LRESULT ProcessEvent (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 
	CString	mfullPath;
	CQuickTime *pQuickTime;		// QuickTime object
	BOOL OpenMovie(void);		// Open a movie
	void CloseMovie(void);		// Close a movie

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleEditMFCView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSimpleEditMFCView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSimpleEditMFCView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileNew();
	afx_msg void OnFileClose();
	afx_msg void OnFileSaveAs();
	afx_msg void OnEditUndo();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditClear();
	afx_msg void OnEditSelectall();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SimpleEdit MFCView.cpp
inline CSimpleEditMFCDoc* CSimpleEditMFCView::GetDocument()
   { return (CSimpleEditMFCDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
