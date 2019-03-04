
// idebView.cpp : implementation of the CidebView class
//

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
END_MESSAGE_MAP()

// CidebView construction/destruction

CidebView::CidebView()
{
	// TODO: add construction code here
	m_FontMonospace.CreateStockObject(ANSI_FIXED_FONT);
}

CidebView::~CidebView()
{
	m_FontMonospace.Detach();
}

BOOL CidebView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
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
	this->GetClientRect(&r);
	m_DCTmp.CreateCompatibleDC(pDC);
	CPalette* pPalette = pDC->GetCurrentPalette();
	m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
	m_DCTmp.SelectObject(&m_BitmapTmp);
	m_DCTmp.SelectObject(&m_FontMonospace);
	CBrush bWhite(RGB(0xff, 0xff, 0xff));
	// CBrush bRed(RGB(0xff, 0x00, 0x00));
	m_DCTmp.FillRect(&r, &bWhite);

	pDoc->m_Simulator.OnDraw( &m_DCTmp);

	pDC->BitBlt(r.left, r.top, r.Width(), r.Height(), &m_DCTmp, 0, 0, SRCCOPY);
	GdiFlush();
	m_DCTmp.DeleteDC();
	m_BitmapTmp.DeleteObject();
	//SetCursor(oldCursor);
	//DeleteObject(oldCursor);
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
	// TODO: add extra initialization before printing
}

void CidebView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
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
	CidebDoc* pDoc = GetDocument();
	if (pDoc) {
		pDoc->OnTimer(nIDEvent);
	}

	CView::OnTimer(nIDEvent);
}


BOOL CidebView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE; //CView::OnEraseBkgnd(pDC);
}
