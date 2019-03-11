#pragma once
#include "WorkspaceView.h"
class CAsmView : public CView, public CWorkspaceView
{
public:
	CAsmView();
	DECLARE_DYNCREATE(CAsmView)
	virtual ~CAsmView();
	// Operations
public:
	CidebDoc * GetProjectDocument();
	CAsmDoc * GetDocument() const;
	virtual void DoWorkspaceUpdate() {}
	virtual void OnWorkspaceUpdate() {
		Invalidate();
	}
	// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void UpdateScollbars();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	CFont m_FontMonospace;
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
	HICON hIconBreak;
	int m_CurX;
	int m_CurY;
	int m_FirstLine; //on screen
	int m_FirstColumn;
	int m_SizeLines;
	int m_FirstXPos; //in pixels
	BOOL m_BreakpointsAreaVisible;
	BOOL m_LineNrAreaVisible;
	BOOL m_WarningAreaVisible;
	BOOL m_DisAsmAreaVisible;
	BOOL m_CursorVisible;
public:
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	int GetLineAtMouse(CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


#ifndef _DEBUG  // debug version in idebView.cpp
inline CidebDoc* CAsmView::GetProjectDocument() 
{
	CWorkspace* pWs = GetWorkspace();
	return reinterpret_cast<CidebDoc*>(pWs);
}
inline CAsmDoc* CAsmView::GetDocument() const
{
	return reinterpret_cast<CAsmDoc*>(m_pDocument);
}
#endif