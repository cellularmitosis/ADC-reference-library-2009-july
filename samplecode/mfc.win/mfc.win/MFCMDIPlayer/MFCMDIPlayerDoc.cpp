// MFCMDIPlayerDoc.cpp : implementation of the CMFCMDIPlayerDoc class
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
// CMFCMDIPlayerDoc

IMPLEMENT_DYNCREATE(CMFCMDIPlayerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCMDIPlayerDoc, CDocument)
	//{{AFX_MSG_MAP(CMFCMDIPlayerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerDoc construction/destruction

CMFCMDIPlayerDoc::CMFCMDIPlayerDoc()
{
	// TODO: add one-time construction code here

}

CMFCMDIPlayerDoc::~CMFCMDIPlayerDoc()
{
}

BOOL CMFCMDIPlayerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerDoc serialization

void CMFCMDIPlayerDoc::Serialize(CArchive& ar)
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
// CMFCMDIPlayerDoc diagnostics

#ifdef _DEBUG
void CMFCMDIPlayerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCMDIPlayerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMFCMDIPlayerDoc commands

BOOL CMFCMDIPlayerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	
	// Set the Movie filename for the view
	POSITION pos = GetFirstViewPosition();
	CMFCMDIPlayerView* pView = (CMFCMDIPlayerView*)GetNextView(pos);
	if (pView){
		pView->mfullPath = lpszPathName;
		if (pView->OpenMovie() != TRUE)
			return FALSE;
	}
	return TRUE;
	
	return TRUE;
}
