/** @file idebView.h
*
* @brief ideb (project) view interface, namely the CidebView class
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#pragma once
#include "WorkspaceView.h"

/** CidebView
View part of the project, implements screen and cpu emluation.
*/
class CidebView : public CView, public CWorkspaceView
{
protected: // create from serialization only
	CidebView();
	DECLARE_DYNCREATE(CidebView)

// Attributes
public:

// Operations
public:
	CidebDoc * GetDocument() const;
	virtual void DoWorkspaceUpdate() {}
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
	void OnWorkspaceUpdate(){}
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
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#ifndef _DEBUG  // debug version in idebView.cpp
inline CidebDoc* CidebView::GetDocument() const
   { return reinterpret_cast<CidebDoc*>(m_pDocument); }
#endif

