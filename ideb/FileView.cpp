/** @file FileView.cpp
*
* @brief FileView implementation
* The goal of this class, to implement a project/workspace specific dockable window for
* the file list/tree view.
*
* It collects the informations from the active workspace and project, and displays as
* a user-friendly list or tree, depending on user interactions.
* It also can handle some context menu, and double click actions.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "ideb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
{
}

CFileView::~CFileView()
{
}

void CFileView::DoWorkspaceUpdate()
{
	FillFileView();
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

BOOL CFileView::PreTranslateMessage(MSG * pMsg)
{
	switch (pMsg->message) {
	case WM_LBUTTONDBLCLK:
	{
		UINT nFlags = pMsg->wParam;
		CPoint point = pMsg->pt;
		OnLButtonDblClk(nFlags, point);
	}
	break;
	}
	return CDockablePane::PreTranslateMessage(pMsg);
}

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	// Load view images:
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillFileView();
	AdjustLayout();

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::FillFileView()
{
	CWorkspace* ws = GetWorkspace();
	//CidebDoc* pProj = NULL;
	CString sProjName = _T("New Project");
	CString s;
	if (ws) {
		sProjName= ws->GetProjectName();
	}
	m_wndFileView.DeleteAllItems();
	//TODO: think througt: workspace can hold multiple CidebDoc ? Then manage multiple projects...
	s.Format(_T("%s files"), sProjName);
	HTREEITEM hRoot = m_wndFileView.InsertItem(s, 0, 0);
	m_wndFileView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
	s.Format(_T("%s  Source Files"), sProjName);
	HTREEITEM hSrc = m_wndFileView.InsertItem(s, 0, 0, hRoot);
	if (ws){
		CStringArray* pAsmFiles = ws->GetAsmFileList();
		int n = (int) pAsmFiles->GetCount();
		for (int i = 0; i < n; i++) {
			HTREEITEM h=
			m_wndFileView.InsertItem(pAsmFiles->GetAt(i), 1, 1, hSrc);
			m_wndFileView.SetItemData(h, i);
		}
	}
	
	s.Format(_T("%s  Target Files"), sProjName);
	HTREEITEM hTrg = m_wndFileView.InsertItem(s, 0, 0, hRoot);
	if (ws){
		m_wndFileView.InsertItem(ws->GetTargetBinFileName(), 2, 2, hTrg);
	}
	
	s.Format(_T("%s  Report Files"), sProjName);
	HTREEITEM hRep = m_wndFileView.InsertItem(s, 0, 0, hRoot);
	s.Format(_T("%s.lst"), sProjName);
	m_wndFileView.InsertItem(s, 2, 2, hRep);
	s.Format(_T("%s.log"), sProjName);
	m_wndFileView.InsertItem(s, 2, 2, hRep);
	// m_wndFileView.InsertItem(_T("test_screen.svg"), 2, 2, hRep);

	m_wndFileView.Expand(hRoot, TVE_EXPAND);
	m_wndFileView.Expand(hSrc, TVE_EXPAND);
	m_wndFileView.Expand(hTrg, TVE_EXPAND);
	m_wndFileView.Expand(hRep, TVE_EXPAND);
}

void CFileView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndFileView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnProperties()
{
	AfxMessageBox(_T("Properties...."));

}
#include "ideb.h"
void CFileView::OpenFile(const CString&fname, int fileId){
	CString fn;
	CidebApp* app = (CidebApp*)AfxGetApp();

	if (fname.Find(L".bin", 0)>0) {
		return;
	}

	if (fname.Find(L".asm", 0)>0) {
		CWorkspace* ws = GetWorkspace();
		fn.Format(L"%s%s", ws->AsmbDirIn(), fname);
		CDocument* pD= app->OpenDocumentFile(fn);
	}
}

void CFileView::OnFileOpen()
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndFileView;
	HTREEITEM it = pWndTree->GetSelectedItem();
	if (it) {
		CString fname = pWndTree->GetItemText(it);
		int id= pWndTree->GetItemData(it);
		OpenFile(fname, id);
	}
	
}

void CFileView::OnFileOpenWith()
{
	// TODO: Add your command handler code here
}

void CFileView::OnDummyCompile()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCut()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditCopy()
{
	// TODO: Add your command handler code here
}

void CFileView::OnEditClear()
{
	// TODO: Add your command handler code here
}

void CFileView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* Locked */);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndFileView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}




void CFileView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndFileView;
	ASSERT_VALID(pWndTree);
	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
			CString fname = pWndTree->GetItemText(hTreeItem);
			int fileId = pWndTree->GetItemData(hTreeItem);
			OpenFile(fname, fileId);
			return;
		}
	}

	//pWndTree->SetFocus();

	__super::OnLButtonDblClk(nFlags, point);
}
