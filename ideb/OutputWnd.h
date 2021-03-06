
#pragma once
#include "WorkspaceView.h"
#include "CSimulator.h"
/////////////////////////////////////////////////////////////////////////////
// COutputList window
class COutputList : public CListBox
{
// Construction
public:
	COutputList();

// Implementation
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

class COutputWnd : public CDockablePane, public CWorkspaceView
{
// Construction
public:
	COutputWnd();
	COutputList* GetBuildOutput() { return &m_wndOutputBuild; }
	void AddError(const tErrorRecord& err);
	void FillBuildWindow(const CString& s);
	void UpdateFonts();

// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;
	COutputList m_wndOutputBuild;
	COutputList m_wndOutputFind;

protected:
	void FillFindWindow();
	void AdjustHorzScroll(CListBox& wndListBox);
	CFont m_FontMonospace;
// Implementation
public:
	virtual ~COutputWnd();
	virtual void DoWorkspaceUpdate() {}
	void OnWorkspaceUpdate() {}
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

