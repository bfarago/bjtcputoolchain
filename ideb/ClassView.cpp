/** @file ClassView.cpp
*
* @brief ClassView interface
* The goal of this class, to implement an integrated development environment's Class view,
* which is dockable, and have the necessary look-and-feel...
*
* Probably the best way, to collect all of the symbols, and display on this UI element.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "stdafx.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "ideb.h"
#include "CSimulator.h"

class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CClassView;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CClassViewMenuButton, CMFCToolBarMenuButton, 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClassView::CClassView()
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CClassView::~CClassView()
{
}

BEGIN_MESSAGE_MAP(CClassView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction)
	ON_COMMAND(ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable)
	ON_COMMAND(ID_CLASS_DEFINITION, OnClassDefinition)
	ON_COMMAND(ID_CLASS_PROPERTIES, OnClassProperties)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView message handlers

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	// Load images:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CClassViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillClassView();

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CClassView::FillClassView()
{
	CWorkspace* ws = GetWorkspace();
	CSimulator* pSim = GetSimulator();
	CString sProjName = _T("New Project");
	CString s;
	if (ws) {
		sProjName = ws->GetProjectName();

	}
	//m_wndClassView.DeleteAllItems();
	m_wndClassView.SetRedraw(FALSE);
	m_wndClassView.DeleteAllItems();
	m_wndClassView.SetRedraw(TRUE);
	m_wndClassView.Invalidate(FALSE);

	//TODO: think througt: workspace can hold multiple CidebDoc ? Then manage multiple projects...
	s.Format(_T("%s symbols"), sProjName);
	HTREEITEM hRoot = m_wndClassView.InsertItem(s, 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
	if (!pSim) return;
	int nSymbols = (int)pSim->m_Symbols.GetCount();
	if (nSymbols < 1) return;
	int nSections = (int)pSim->m_Sections.GetCount();

	HTREEITEM hEQU = m_wndClassView.InsertItem(_T("EQU"), 1, 1, hRoot);
	HTREEITEM hLabels= m_wndClassView.InsertItem(_T("JUMP LABEL"), 1, 1, hRoot);

#define MAXSECTIONS (256)
	HTREEITEM hSection[MAXSECTIONS];
	if (nSections < 1) {
		hSection[0] = m_wndClassView.InsertItem(_T("in section .code"), 1, 1, hRoot);
		hSection[1] = m_wndClassView.InsertItem(_T("in section .data"), 1, 1, hRoot);
		nSections = 2;
	}
	else {
		for (int i = 0; i < nSections; i++) {
			CString s;
			s.Format(L"in section: '%s'", pSim->m_Sections[i]);
			hSection[i] = m_wndClassView.InsertItem(s, 1, 1, hRoot);
		}
	}
	
	
	for (int i = 0; i < nSymbols; i++) {
		const tDbgFileSymbol& sym = pSim->m_Symbols.GetAt(i);
		int secId = sym.sectionid;
		CString context = L"";
		if (sym.symcontexts) {
			#define CASEST(id, name) if (sym.symcontexts & (1 << id)) { context.Append(name); }
			CASEST(ST_Unknown, L"?");

			CASEST(ST_DB, L"data ");
			CASEST(ST_EQU, L":=");
			CASEST(ST_JMP, L"jump ");
			CASEST(ST_JMP, L"label ");
			CASEST(ST_LOAD, L"load ");
			CASEST(ST_STORE, L"store ");
		}
		if (secId >= nSections) {
			secId = 0;
		}
		HTREEITEM hItem = hSection[secId];
		HTREEITEM hSymbolItem = 0;
		CString s;
		unsigned char memtype = sym.memtype;
		if (sym.symcontexts&(1 << ST_EQU)) {
			memtype = MT_undef; // remove equ from other groups, add to equ group only...
		}
		//TODO: DBG file actually not unicode, so... analyze the problem :), if we need it or it is good as is...
		switch (memtype) {
		case MT_code:
			s.Format(L"%S = %03x %s", sym.name, sym.value, context);
			hSymbolItem =m_wndClassView.InsertItem(s, 3, 3, hItem);
			break;
		case MT_data:
			s.Format(L"%S = %03x %s", sym.name, sym.value, context);
			hSymbolItem = m_wndClassView.InsertItem(s, 5, 5, hItem);
			break;
		default:
			s.Format(L"%S equ %03x %s", sym.name, sym.value, context);
			hSymbolItem = m_wndClassView.InsertItem(s, 3, 3, hEQU);
			break;
		}
		if (hSymbolItem)
			m_wndClassView.SetItemData(hSymbolItem, i);

		if (sym.symcontexts&(1 << ST_JMP)) {
			s.Format(L"%S\n 0x%03x %s", sym.name, sym.value, context);
			hSymbolItem = m_wndClassView.InsertItem(s, 3, 3, hLabels);
			if (hSymbolItem)
				m_wndClassView.SetItemData(hSymbolItem, i);
		}
	}
	m_wndClassView.Expand(hRoot, TVE_EXPAND);

	BOOL m_EnableExpanded = FALSE;
	if (m_EnableExpanded) {
		m_wndClassView.Expand(hEQU, TVE_EXPAND);
		for (int i = 0; i < nSections; i++) {
			m_wndClassView.Expand(hSection[i], TVE_EXPAND);
		}
	}
	/*
	m_wndClassView.InsertItem(_T("periph_uart"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("periph_scr"), 3, 3, hClass);
	m_wndClassView.Expand(hRoot, TVE_EXPAND);
	m_wndClassView.Expand(hClass, TVE_EXPAND);

	hClass = m_wndClassView.InsertItem(_T("Code"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("reset"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("loop1"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("v_y"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("v_x"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("v_h"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("v_l"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("start_inc"), 3, 3, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);


	hClass = m_wndClassView.InsertItem(_T("Data"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("plus1"), 5, 5, hClass);
	m_wndClassView.InsertItem(_T("spos"), 5, 5, hClass);
	m_wndClassView.InsertItem(_T("soff"), 5, 5, hClass);
	m_wndClassView.InsertItem(_T("v_start_offs"), 5, 5, hClass);
	//m_wndClassView.InsertItem(_T("~CFakeAppView()"), 3, 3, hClass);
	//m_wndClassView.InsertItem(_T("GetDocument()"), 3, 3, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);
	*/
	/*hClass = m_wndClassView.InsertItem(_T("CFakeAppFrame"), 1, 1, hRoot);
	m_wndClassView.InsertItem(_T("CFakeAppFrame()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("~CFakeAppFrame()"), 3, 3, hClass);
	m_wndClassView.InsertItem(_T("m_wndMenuBar"), 6, 6, hClass);
	m_wndClassView.InsertItem(_T("m_wndToolBar"), 6, 6, hClass);
	m_wndClassView.InsertItem(_T("m_wndStatusBar"), 6, 6, hClass);

	hClass = m_wndClassView.InsertItem(_T("Globals"), 2, 2, hRoot);
	m_wndClassView.InsertItem(_T("theFakeApp"), 5, 5, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);
	*/
}

void CClassView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
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
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CClassView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_LBUTTONDBLCLK:
		{
			UINT nFlags = (UINT) pMsg->wParam ;
			CPoint point = pMsg->pt;
			OnLButtonDblClk(nFlags, point);
		}
		break;
	}
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClassView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

void CClassView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CClassView::OnClassAddMemberFunction()
{
	AfxMessageBox(_T("Add member function..."));
}

void CClassView::OnClassAddMemberVariable()
{
	// TODO: Add your command handler code here
}

void CClassView::OnClassDefinition()
{
	// TODO: Add your command handler code here
}

void CClassView::OnClassProperties()
{
	// TODO: Add your command handler code here
}

void CClassView::OnNewFolder()
{
	AfxMessageBox(_T("New Folder..."));
}

void CClassView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CClassView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

void CClassView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

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

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);
}


void CClassView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndClassView;
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
			int symbolIndex= (int)m_wndClassView.GetItemData(hTreeItem);
			CSimulator* pSim = GetSimulator();
			if (pSim) {
				const tDbgFileSymbol& sym = pSim->m_Symbols.GetAt(symbolIndex);
				CWorkspaceEvent event;
				event.EventType = CWorkspaceEvent::GoToSourceLine;
				event.Line = sym.lineno;
				event.FileId = sym.fileId;
				DoWorkspaceEvent(&event);
				return;
			}
		}
	}

	// pWndTree->SetFocus();

	__super::OnLButtonDblClk(nFlags, point);
}
