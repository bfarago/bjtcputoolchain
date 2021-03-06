
#include "stdafx.h"


#include "Resource.h"
#include "MainFrm.h"
#include "ideb.h"
#include "Workspace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
	m_nComboHeight = 0;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd () == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}


int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	m_wndObjectCombo.AddString(_T("Application"));
	m_wndObjectCombo.AddString(_T("Properties Window"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::DoWorkspaceUpdate()
{
	CWorkspace* ws = GetWorkspace();
	if (!ws) return;
	m_PropToolchainDir->SetValue((_variant_t)ws->ToolchainDir());
	m_PropTargetBinFileName->SetValue((_variant_t)ws->GetTargetBinFileName());
	m_PropProjectName->SetValue((_variant_t)ws->GetProjectName());
	m_PropAsmbFlags->SetValue((_variant_t)ws->AsmbFlags());
	m_PropAsmbVerbose->SetValue((_variant_t)(bool)ws->AsmbVerbose());
}
void CPropertiesWnd::OnProperties2()
{
	CWorkspace* ws = GetWorkspace();
	if (!ws) return;
	CMFCPropertyGridProperty* pProp;
	pProp = m_PropTargetBinFileName;
	if (pProp->IsModified()) {
		ws->SetTargetBinFileName(pProp->GetValue());
		pProp->SetOriginalValue((_variant_t)ws->GetTargetBinFileName());
		pProp->ResetOriginalValue();
	}
	pProp = m_PropProjectName;
	if (pProp->IsModified()) {
		ws->SetProjectName(pProp->GetValue());
		pProp->SetOriginalValue((_variant_t)ws->GetProjectName());
		pProp->ResetOriginalValue();
	}
	pProp = m_PropAsmbFlags;
	if (pProp->IsModified()) {
		ws->AsmbFlags()= pProp->GetValue();
		pProp->SetOriginalValue((_variant_t)ws->AsmbFlags());
		pProp->ResetOriginalValue();
	}
	pProp = m_PropAsmbVerbose;
	if (pProp->IsModified()) {
		BOOL b= pProp->GetValue().boolVal;
		ws->AsmbVerbose() = b;
		pProp->SetOriginalValue((_variant_t)(bool)b);
		pProp->ResetOriginalValue();
	}
	pProp = m_PropAsmbDirIn;
	if (pProp->IsModified()) {
		ws->AsmbDirIn() = pProp->GetValue();
		pProp->SetOriginalValue((_variant_t)ws->AsmbDirIn());
		pProp->ResetOriginalValue();
	}
	pProp = m_PropAsmbDirOut;
	if (pProp->IsModified()) {
		ws->AsmbDirOut() = pProp->GetValue();
		pProp->SetOriginalValue((_variant_t)ws->AsmbDirOut());
		pProp->ResetOriginalValue();
	}
	pProp = m_PropToolchainDir;
	if (pProp->IsModified()) {
		ws->ToolchainDir() = pProp->GetValue();
		pProp->SetOriginalValue((_variant_t)ws->ToolchainDir());
		pProp->ResetOriginalValue();
	}
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(
		m_PropTargetBinFileName->IsModified()
		| m_PropProjectName->IsModified()
		| m_PropAsmbFlags->IsModified()
		| m_PropAsmbVerbose->IsModified()
		| m_PropAsmbDirIn->IsModified()
		| m_PropAsmbDirOut->IsModified()
		| m_PropToolchainDir->IsModified()
	);
	
}
void CPropertiesWnd::InitPropList()
{
	SetPropListFont();
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
	CMFCPropertyGridProperty* pProp;
/*
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Appearance"));

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("3D Look"), (_variant_t) false, _T("Specifies the window's font will be non-bold and controls will have a 3D border")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("Border"), _T("Dialog Frame"), _T("One of: None, Thin, Resizable, or Dialog Frame"));
	pProp->AddOption(_T("None"));
	pProp->AddOption(_T("Thin"));
	pProp->AddOption(_T("Resizable"));
	pProp->AddOption(_T("Dialog Frame"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Caption"), (_variant_t) _T("About"), _T("Specifies the text that will be displayed in the window's title bar")));
	pGroup1->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup1);
	*/
	/*CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("Window Size"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("Height"), (_variant_t) 250l, _T("Specifies the window's height"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Width"), (_variant_t) 150l, _T("Specifies the window's width"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);
	*/
	/*
	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Font"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	_tcscpy_s(lf.lfFaceName, _T("Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("Font"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("Specifies the default font for the window")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Use System Font"), (_variant_t) true, _T("Specifies that the window uses MS Shell Dlg font")));
	pGroup2->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("Misc"));
	pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("Application"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));
	pGroup3->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup3);
	*/
	CMFCPropertyGridProperty* pGroupProjSet = new CMFCPropertyGridProperty(_T("Project settings"));
	m_PropProjectName = new CMFCPropertyGridProperty(_T("Project name"), (_variant_t)_T("TestSw"), _T("This is the output project name"));
	pGroupProjSet->AddSubItem(m_PropProjectName);

	m_PropTargetBinFileName = new CMFCPropertyGridProperty(_T("Target File"), (_variant_t)_T("a.bin"), _T("Simulator target bin file."));
	pGroupProjSet->AddSubItem(m_PropTargetBinFileName);

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("Toolchain settings"));
	m_PropToolchainDir = new CMFCPropertyGridFileProperty(_T("Toolchain bin folder"), _T("..\\bin"));
	pGroup41->AddSubItem(m_PropToolchainDir);
	pGroupProjSet->AddSubItem(pGroup41);
	
	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("Asmb settings"));
	pGroupProjSet->AddSubItem(pGroup411);
	m_PropAsmbDirIn = new CMFCPropertyGridProperty(_T("Input directory"), (_variant_t)_T("..\\sample\\"), _T("Input directory of the sources."));
	pGroup411->AddSubItem(m_PropAsmbDirIn);
	m_PropAsmbDirOut = new CMFCPropertyGridProperty(_T("Output directory"), (_variant_t)_T("..\\sample\\"), _T("Output directory, where log and target bin generated."));
	pGroup411->AddSubItem(m_PropAsmbDirOut);
	m_PropAsmbFlags = new CMFCPropertyGridProperty(_T("Flags"), (_variant_t)_T("-f -l"), _T("Command line flags of asmb"));
	pGroup411->AddSubItem(m_PropAsmbFlags);
	m_PropAsmbVerbose = new CMFCPropertyGridProperty(_T("Verbose"), (_variant_t)false, _T("Debug level verbosity of the asmb.exe. -v"));
	pGroup411->AddSubItem(m_PropAsmbVerbose);


	//pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 2"), (_variant_t) _T("Value 2"), _T("This is a description")));
	//pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 3"), (_variant_t) _T("Value 3"), _T("This is a description")));
	CMFCPropertyGridProperty* pGroup412 = new CMFCPropertyGridProperty(_T("Simb settings"));
	pGroupProjSet->AddSubItem(pGroup412);
	pProp = new CMFCPropertyGridProperty(_T("Steps"), (_variant_t)250l, _T("Simulation steps"));
	pProp->EnableSpinControl(TRUE, 5, 3000);
	pGroup412->AddSubItem(pProp);


	pGroupProjSet->Expand(TRUE);
	m_wndPropList.AddProperty(pGroupProjSet);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	m_wndObjectCombo.SetFont(&m_fntPropList);
}
