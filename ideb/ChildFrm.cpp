
// ChildFrm.cpp : implementation of the CChildFrameProj class
//

#include "stdafx.h"
#include "ideb.h"

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrameProj

IMPLEMENT_DYNCREATE(CChildFrameProj, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrameProj, CMDIChildWndEx)
END_MESSAGE_MAP()

// CChildFrameProj construction/destruction

CChildFrameProj::CChildFrameProj()
{
	// TODO: add member initialization code here
}

CChildFrameProj::~CChildFrameProj()
{
}

BOOL CChildFrameProj::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	return m_wndSplitter.Create(this,
		2, 2,			// TODO: adjust the number of rows, columns
		CSize(10, 10),	// TODO: adjust the minimum pane size
		pContext);
}

BOOL CChildFrameProj::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrameProj diagnostics

#ifdef _DEBUG
void CChildFrameProj::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrameProj::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrameProj message handlers
