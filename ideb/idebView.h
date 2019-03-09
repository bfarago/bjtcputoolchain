
// idebView.h : interface of the CidebView class
//

#pragma once
#include "WorkspaceView.h"

class CidebView : public CView, public CWorkspaceView
{
protected: // create from serialization only
	CidebView();
	DECLARE_DYNCREATE(CidebView)

// Attributes
public:
	CidebDoc* GetDocument() const;
	virtual void DoWorkspaceUpdate() {}
// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	CFont m_FontMonospace;
// Implementation
public:
	virtual ~CidebView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in idebView.cpp
inline CidebDoc* CidebView::GetDocument() const
   { return reinterpret_cast<CidebDoc*>(m_pDocument); }
#endif

