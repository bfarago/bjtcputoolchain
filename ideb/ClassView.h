/** @file ClassView.h
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
#pragma once

#include "ViewTree.h"
#include "WorkspaceView.h"

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CClassView : public CDockablePane, public CWorkspaceView
{
public:
	CClassView();
	virtual ~CClassView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	void DoWorkspaceUpdate() {  }
	void OnWorkspaceUpdate() { FillClassView(); }
	
protected:
	CClassToolBar m_wndToolBar;
	CViewTree m_wndClassView;
	CImageList m_ClassViewImages;
	UINT m_nCurrSort;
	void FillClassView();

// Overrides
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClassAddMemberFunction();
	afx_msg void OnClassAddMemberVariable();
	afx_msg void OnClassDefinition();
	afx_msg void OnClassProperties();
	afx_msg void OnNewFolder();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnChangeActiveTab(WPARAM, LPARAM);
	afx_msg void OnSort(UINT id);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	
};

