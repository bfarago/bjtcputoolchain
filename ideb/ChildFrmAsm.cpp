// ChildFrmAsm.cpp : implementation file
//

#include "stdafx.h"
#include "ideb.h"
#include "ChildFrmAsm.h"


// CChildFrameAsm

IMPLEMENT_DYNCREATE(CChildFrameAsm, CMDIChildWndEx)

CChildFrameAsm::CChildFrameAsm()
{

}

CChildFrameAsm::~CChildFrameAsm()
{
}

BOOL CChildFrameAsm::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	return m_wndSplitter.Create(this,
		2, 2,       // TODO: adjust the number of rows, columns
		CSize(10, 10),  // TODO: adjust the minimum pane size
		pContext);
}

BEGIN_MESSAGE_MAP(CChildFrameAsm, CMDIChildWndEx)
END_MESSAGE_MAP()


// CChildFrameAsm message handlers
