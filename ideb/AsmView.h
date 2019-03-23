/** @file AsmView.h
*
* @brief Asm file View interface
* The goal of this class, to display syntax highlighted asm source view.
* Also additional columns are switchable to be displayed, like:
* Brakepoint area, Disassembly area, Line numbering area, and Warning area.
* The view generates the GDI graphics for windows screen and printer cases.
* The printer variant is not speed critical, but windowed graphics should be
* very quick, because of the screen simulator refresh the window a lot,
* at least on every ~60..100ms. There are differences on color schemes as well.
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#pragma once
#include "WorkspaceView.h"

/**CAsmView: View part of the asm file handling.
It displays the content of the asm file with syntax highlight colored way.
Also it handles some of the debug behaviours like displaying the actual PC location,
disasm coulumn, breakpoints...
It draws to printer and screen..
*/
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
	BOOL OnWorkspaceEvent(CWorkspaceEvent* pEvent);
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
	CPen m_PenCursor;
	CFont m_FontMonospace;
	CFont m_FontPrint;
	CFont m_FontHeader;
	CString m_TitleStr;
	CString m_DateStr;
	CRect m_RcPage;
	int m_nRowHeight;
	int m_nRowsPerPage;
	int m_nMaxRowCount;
	int m_PrintCurPage;
	int m_PrintMaxColumn;
	int m_PrintCharWidth;
	unsigned int m_PrintRatioX;
	unsigned int m_PrintRatioY;
	void PrintFooter(CDC * pDC, CPrintInfo * pInfo);
	void ComputeMetrics(CDC * pDC);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	void OnPrepareDC(CDC * pDC, CPrintInfo * pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	afx_msg void OnFilePrintPreview();
	void PrintHeader(CDC * pDC, CPrintInfo * pInfo);
	void DrawLineAt(CDC * pDC, unsigned int y);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
	HICON hIconBreak;
	HICON hWarning;
	int m_CurX;
	int m_CurY;
	int m_CurXWish;
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	void GoToSourceLine(int line, BOOL bCursorUpdate=TRUE);
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