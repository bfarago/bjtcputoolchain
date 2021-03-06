/** @file idebView.cpp
*
* @brief ideb (project) view implementation
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ideb.h"
#endif

#include "idebDoc.h"
#include "idebView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//Refresh timer for screen and cpu simulation
#define SIM_ID_REFRESHTIMER 1000

// CidebView

IMPLEMENT_DYNCREATE(CidebView, CView)

BEGIN_MESSAGE_MAP(CidebView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CidebView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CidebView construction/destruction

CidebView::CidebView()
{
	m_FontMonospace.CreateStockObject(SYSTEM_FIXED_FONT); //ANSI_FIXED_FONT);
}

CidebView::~CidebView()
{
	m_FontMonospace.Detach();
}

BOOL CidebView::PreCreateWindow(CREATESTRUCT& cs)
{
	// OPTIONAL: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CView::PreCreateWindow(cs);
}

// CidebView drawing
void CidebView::OnDraw(CDC* pDC)
{
	CidebDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	CDC m_DCTmp;
	CBitmap m_BitmapTmp;
	CRect r;
	m_DCTmp.CreateCompatibleDC(pDC);

	if (!pDC->IsPrinting()) {
		GetClientRect(&r);
	}
	else {
		int sx = 1000;
		int sy = 600;
		r.left = 0; r.right = sx;
		r.top = 0; r.bottom = sy;
		m_DCTmp.m_bPrinting = TRUE;
	}
	CPalette* pPalette = pDC->GetCurrentPalette();
	m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
	m_DCTmp.SelectObject(&m_BitmapTmp);
	m_DCTmp.SelectObject(&m_FontMonospace);
	CBrush bWhite(RGB(0xff, 0xff, 0xff));
	m_DCTmp.FillRect(&r, &bWhite);
	pDoc->m_Simulator.OnDraw(&m_DCTmp, pDoc->m_SimulationRunning?1:0);
	if (!pDC->IsPrinting()) {
		pDC->BitBlt(r.left, r.top, r.Width(), r.Height(), &m_DCTmp, 0, 0, SRCCOPY);
		GdiFlush();
	}
	else {

		int px = pDC->GetDeviceCaps(HORZRES);
		int py = r.Height() * px / r.Width();
		pDC->StretchBlt(0, 0, px , py, &m_DCTmp, 0, 0, r.Width(), r.Height(), SRCCOPY);
	}
	m_DCTmp.DeleteDC();
	m_BitmapTmp.DeleteObject();
}


// CidebView printing

void CidebView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CidebView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CidebView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	KillTimer(SIM_ID_REFRESHTIMER);
}

void CidebView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	SetTimer(SIM_ID_REFRESHTIMER, SIM_REFRESH_TIMER, NULL);
}

void CidebView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CidebView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CidebView diagnostics

#ifdef _DEBUG
void CidebView::AssertValid() const
{
	CView::AssertValid();
}

void CidebView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CidebDoc* CidebView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CidebDoc)));
	return (CidebDoc*)m_pDocument;
}
#endif //_DEBUG


// CidebView message handlers

void CidebView::OnTimer(UINT_PTR nIDEvent)
{
	if (SIM_ID_REFRESHTIMER == nIDEvent) {
		CidebDoc* pDoc = GetDocument();
		if (pDoc) {
			if (pDoc->m_SimulationRunning) {
				pDoc->OnTimer(nIDEvent);
				Invalidate();
			}
		}
		return;
	}
	CView::OnTimer(nIDEvent);
}

BOOL CidebView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; //CView::OnEraseBkgnd(pDC); //for flickerfree draw
}


void CidebView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	SetTimer(SIM_ID_REFRESHTIMER, SIM_REFRESH_TIMER, NULL);
}

void CidebView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CidebDoc* pDoc = GetDocument();
	if (pDoc) {
		pDoc->OnKeyDown(nChar, nRepCnt, nFlags);
	}
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CidebView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CidebDoc* pDoc = GetDocument();
	if (pDoc) {
		pDoc->OnKeyUp(nChar, nRepCnt, nFlags);
	}
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


BOOL CidebView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CSimulator &rSimulator = GetDocument()->m_Simulator;
	if (rSimulator.m_DisplayTimeLine) {
		if (zDelta > 0) {
			rSimulator.m_TraceShiftR++;
			if (rSimulator.m_TraceShiftR > 9) rSimulator.m_TraceShiftR = 9;
			Invalidate();
		}
		else {
			rSimulator.m_TraceShiftR--;
			if (rSimulator.m_TraceShiftR < 0)rSimulator.m_TraceShiftR = 0;
			Invalidate();
		}
	}

	return __super::OnMouseWheel(nFlags, zDelta, pt);
}
