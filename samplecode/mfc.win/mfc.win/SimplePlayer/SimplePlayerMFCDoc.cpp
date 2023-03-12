// SimplePlayer MFCDoc.cpp : implementation of the CSimplePlayerMFCDoc class
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
// CSimplePlayerMFCDoc

IMPLEMENT_DYNCREATE(CSimplePlayerMFCDoc, CDocument)

BEGIN_MESSAGE_MAP(CSimplePlayerMFCDoc, CDocument)
	//{{AFX_MSG_MAP(CSimplePlayerMFCDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCDoc construction/destruction

CSimplePlayerMFCDoc::CSimplePlayerMFCDoc()
{
	// TODO: add one-time construction code here

}

CSimplePlayerMFCDoc::~CSimplePlayerMFCDoc()
{
}

BOOL CSimplePlayerMFCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCDoc serialization

void CSimplePlayerMFCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCDoc diagnostics

#ifdef _DEBUG
void CSimplePlayerMFCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSimplePlayerMFCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimplePlayerMFCDoc commands

BOOL CSimplePlayerMFCDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// Set the Movie filename for the view
	POSITION pos = GetFirstViewPosition();
	CSimplePlayerMFCView* pView = (CSimplePlayerMFCView*)GetNextView(pos);
	if (pView){
		pView->mfullPath = lpszPathName;
		if (pView->OpenMovie() != TRUE)
			return FALSE;
	}
	
	return TRUE;
}
