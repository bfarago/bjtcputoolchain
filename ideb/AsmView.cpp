#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ideb.h"
#endif
#include "AsmDoc.h"
#include "AsmView.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const TCHAR *gAsmMnemonics[17] =
{ _T("mvi a,"),_T("sta"),_T("lda"),_T("ad0"),_T("ad1"),_T("adc"),_T("nand"),_T("nor"),_T("rrm"),_T("jmp"),_T("jc "),_T("jnc"),_T("jz "),_T("jnz"),_T("jm "),_T("jp "),_T("INVALID") };

#define ID_ASMEDITORTIMER 1001
// CAsmView
IMPLEMENT_DYNCREATE(CAsmView, CView)

BEGIN_MESSAGE_MAP(CAsmView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CAsmView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CAsmView::CAsmView()
	:m_FirstLine(0), m_FirstColumn(0), m_CursorVisible(1),
	m_CurX(0), m_CurY(0), m_CurXWish(0)
{
	m_FontMonospace.CreateStockObject(SYSTEM_FIXED_FONT); //ANSI_FIXED_FONT);
	hIconBreak = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_BREAK),
		IMAGE_ICON, 16, 16, 0);
	m_BreakpointsAreaVisible = TRUE;
	m_LineNrAreaVisible = TRUE;
	m_WarningAreaVisible = TRUE;
	m_DisAsmAreaVisible = FALSE;
}


CAsmView::~CAsmView()
{
	m_FontMonospace.Detach();
	UnRegister();
}
BOOL CAsmView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}
void CAsmView::UpdateScollbars() {
	int maxlines = GetDocument()->m_lines.GetCount();
	SetScrollRange(SB_VERT, 0, maxlines);
}
#include "CSimulator.h"
#include "idebDoc.h"
void CAsmView::OnDraw(CDC* pDC)
{
	CScrollBar* pSV = GetScrollBarCtrl(SB_VERT);
	int startLine = GetScrollPos(SB_VERT);
	CWorkspace* pWs = GetWorkspace();
	if (!pWs) {
		//TODO: check if the opened asm file belongs to the workspace...
		//TODO: close views, when workspace closed...
		RegisterToActiveWorkspace();
		pWs = GetWorkspace();
	}
	CidebDoc* pProj = (CidebDoc*)pWs;
	CSimulator * pSimulator = NULL;
	if (pWs)
		pSimulator= pWs->GetSimulator();
	//ASSERT_VALID(pProj);
	/*if (pProj) {
		pSimulator = &(pProj->m_Simulator);	
	}*/
	CAsmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	CDC m_DCTmp;
	CBitmap m_BitmapTmp;
	CRect r;
	GetClientRect(&r);
	
	m_DCTmp.CreateCompatibleDC(pDC);
	CPalette* pPalette = pDC->GetCurrentPalette();
	m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
	m_DCTmp.SelectObject(&m_BitmapTmp);
	m_DCTmp.SelectObject(&m_FontMonospace);
	TEXTMETRIC tm;
	m_DCTmp.GetTextMetrics(&tm);
	COLORREF crBlack = 0; 
	CBrush bWhite(RGB(0xff, 0xff, 0xff));
	CBrush brCursorBk(RGB(0xf0, 0xf0, 0xf0));
	CBrush brProgramCounter(RGB(0xff, 0xdf, 0xdf));
	CBrush brBrakepointAreaBk(RGB(0xff, 0xfd, 0xfd));
	CBrush brLineNrAreaBk(RGB(0xf0, 0xff, 0xf0));
	CBrush brWarningAreaBk(RGB(0xef, 0xef, 0xef));
	CBrush brDisAsmAreaBk(RGB(0xd0, 0xd0, 0xd0));

	m_DCTmp.SetBkMode(TRANSPARENT);
	LONG n = pDoc->m_lines.GetCount();
	CRect tr;
	
	tr.left = 0; tr.right = r.Width();
	int firstX = 0;
	int firstXLineNr = 0;
	int firstXDisAsm = 0;
	int lastXLineNr = 0;
	int lastXDisAsm = 0;

	if (pSimulator) {
		m_DisAsmAreaVisible= pSimulator->IsDebugFileLoaded();
	}
	if (m_BreakpointsAreaVisible) {
		firstX += 16;
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = 0; tr.right = 16;
		m_DCTmp.FillRect(&tr, &brBrakepointAreaBk);
	}
	if (m_DisAsmAreaVisible) {
		firstXDisAsm = firstX;
		firstX += tm.tmAveCharWidth* (3 + 1 + 6 + 1 + 5);
		lastXDisAsm = firstX;
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = firstXDisAsm; tr.right = lastXDisAsm;
		m_DCTmp.FillRect(&tr, &brDisAsmAreaBk);
	}
	int linesInFile = pDoc->m_lines.GetCount();
	if (m_LineNrAreaVisible) {
		firstXLineNr = firstX;
		lastXLineNr = firstX += tm.tmAveCharWidth* (log10(linesInFile)+1);
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = firstXLineNr; tr.right = lastXLineNr;
		m_DCTmp.FillRect(&tr, &brLineNrAreaBk);
	}
	if (m_WarningAreaVisible) {
		firstX += 16;
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = lastXLineNr; tr.right = firstX;
		m_DCTmp.FillRect(&tr, &brWarningAreaBk);
	}
	m_FirstXPos = firstX;
	tr.top = 0; tr.bottom = r.bottom;
	tr.left = firstX; tr.right =r.right;
	m_DCTmp.FillRect(&tr, &bWhite);
	int DbgMemLocation = 0;
	int pc = 0;
	if (pSimulator) pc= pSimulator->GetPc();
	if (linesInFile < m_FirstLine + m_SizeLines)
		m_SizeLines = linesInFile - m_FirstLine;
	for (LONG i = 0; i < m_SizeLines; i++) {
		CAsmDoc::LineData* ld = &(pDoc->m_lines.GetAt(i+m_FirstLine));
		tr.top = i * tm.tmHeight; tr.bottom = tr.top + tm.tmHeight;

		if (m_DisAsmAreaVisible)
		{
			if (ld->memAddress < 0) {
				//if (pSimulator) ld->memAddress = pSimulator->SearchLine(i);
			}
			if (ld->memAddress >= 0) {
				if (pc == ld->memAddress) {
					tr.left = 0; tr.right = r.right;
					m_DCTmp.FillRect(&tr, &brProgramCounter);
				}
				CString s;
				CString disasm;
				if(pSimulator) pSimulator->GetDisAsm(ld->memAddress, disasm);
				else disasm = L"";
				//s.Format(L"%03x: %s", ld->memAddress & 0xfff, disasm);
				s = disasm;
				tr.left = firstXDisAsm; tr.right = lastXDisAsm;
				m_DCTmp.DrawText(s, tr, DT_LEFT);
			}
		}
		if (ld->memAddress >= 0)
		{
			if (pSimulator) {
				if (m_BreakpointsAreaVisible) {
					if (pSimulator->IsBreakPC(ld->memAddress)) {
						m_DCTmp.DrawIcon(0 - 16, tr.top - 8, hIconBreak);
					}
				}
			}
		}
		if (m_LineNrAreaVisible) {
			CString s;
			s.Format(L"%d", ld->number);
			tr.left = firstXLineNr; tr.right = lastXLineNr;
			m_DCTmp.DrawText(s, tr, DT_RIGHT);
		}
		if (m_CurY == i) {
			tr.left = firstX; tr.right = r.right;
			//m_DCTmp.FillSolidRect()
			m_DCTmp.FillRect(&tr, &brCursorBk);
		}
		int nTokens = ld->tokens.GetCount();
		if (nTokens) {
			for (int q = 0; q < nTokens; q++) {
				const CAsmDoc::TokenData& td = ld->tokens[q];
				int x = td.column * tm.tmAveCharWidth;
				tr.left = firstX + x; tr.right = tr.left + td.count*tm.tmAveCharWidth;
				m_DCTmp.SetTextColor(td.color);
				m_DCTmp.DrawText(ld->text.Mid(td.first, td.count), tr, DT_TOP | DT_EXPANDTABS);
			}
			m_DCTmp.SetTextColor(crBlack);
		}
		else
		{
			CString s = ld->text;
			tr.left = firstX; tr.right = r.right;
			m_DCTmp.DrawText(s, tr, DT_TOP | DT_EXPANDTABS);
		}
	}

	if (m_CursorVisible) {
		tr.top = m_CurY * tm.tmHeight; tr.bottom = tr.top + tm.tmHeight;
		m_DCTmp.MoveTo(firstX + m_CurX * tm.tmAveCharWidth, tr.top);
		m_DCTmp.LineTo(firstX + m_CurX * tm.tmAveCharWidth, tr.bottom);
	}
	pDC->BitBlt(r.left, r.top, r.Width(), r.Height(), &m_DCTmp, 0, 0, SRCCOPY);
	GdiFlush();
	m_DCTmp.DeleteDC();
	m_BitmapTmp.DeleteObject();
	//SetCursor(oldCursor);
	//DeleteObject(oldCursor);
}

// CidebView printing

void CAsmView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}
BOOL CAsmView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}
void CAsmView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CAsmView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CAsmView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CAsmView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


BOOL CAsmView::OnEraseBkgnd(CDC* pDC)
{

	return TRUE; //CView::OnEraseBkgnd(pDC);
}


void CAsmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	UpdateScollbars();
	SetTimer(ID_ASMEDITORTIMER, 600, NULL);
}


void CAsmView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CAsmDoc* pDoc= GetDocument();
	int maxlines = pDoc->m_lines.GetCount();
	switch (nChar) {
	case VK_HOME:
		if (m_CurY<1) m_FirstLine = 0;
		m_CurY = 0;
		break;
	case VK_END:
		if (m_CurY >= m_SizeLines) m_FirstLine = maxlines - m_SizeLines;
		m_CurY = m_SizeLines;
		break;
	case VK_UP: m_CurY--; break;
	case VK_DOWN: m_CurY++; break;
	case VK_LEFT: m_CurX--; break;
	case VK_RIGHT: m_CurX++; break;
	case VK_PRIOR:
		m_FirstLine -= m_SizeLines;
		if (m_FirstLine < 0)m_FirstLine = 0;
		break;
	case VK_NEXT:
		m_FirstLine += m_SizeLines;
		if (m_FirstLine + m_SizeLines > maxlines) m_FirstLine = maxlines - m_SizeLines;
		break;
	}
	if (m_CurY < 0) {
		m_FirstLine += m_CurY;
		m_CurY = 0;
		if (m_FirstLine < 0)m_FirstLine = 0;
	}
	if (m_CurX < 0) m_CurX = 0;
	if (m_CurY >= m_SizeLines) {
		if (m_FirstLine + m_CurY < maxlines) {
			m_FirstLine += 1;
		}
		m_CurY -= 1;
	}
	pDoc->CursorValidate(m_CurX, m_CurY, m_CurXWish, nChar);
	if (!pDoc->IsReadOnly())
	{
		switch (nChar) {
		case VK_SPACE:
			//todo: convert xcoord to text pos..
			break;
		}
	}
	Invalidate();
}


void CAsmView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CAsmDoc* pDoc = GetDocument();
	if (pDoc) {
		//pDoc->OnKeyUp(nChar, nRepCnt, nFlags);
	}
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


// CAsmView diagnostics

#ifdef _DEBUG
void CAsmView::AssertValid() const
{
	CView::AssertValid();
}

void CAsmView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAsmDoc* CAsmView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAsmDoc)));
	return (CAsmDoc*)m_pDocument;
}
CidebDoc* CAsmView::GetProjectDocument() // non-debug version is inline
{
	CWorkspace* pWs=GetWorkspace();
	//ASSERT(pWs->IsKindOf(RUNTIME_CLASS(CidebDoc)));
	return (CidebDoc*)pWs;
}
#endif //_DEBUG


BOOL CAsmView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// TODO: Add your specialized code here and/or call the base class

	return __super::OnScroll(nScrollCode, nPos, bDoScroll);
}

int CAsmView::GetLineAtMouse(CPoint point) {
	int offsy = 0;
	int n = (point.y - offsy) / 15;
	return n+m_FirstLine;
}
void CAsmView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_BreakpointsAreaVisible) {
		CSimulator* pSim = GetSimulator();
		if (pSim) {
			if (point.x <= 15) {
				int line = GetLineAtMouse(point);
				//m_LineNrAreaVisible
				CAsmDoc::LineData& ld = GetDocument()->m_lines.GetAt(line);
				if (ld.memAddress >= 0) {
					pSim->SetBrakePC(ld.memAddress);
					Invalidate();
				}
			}
		}
	}

	__super::OnLButtonDblClk(nFlags, point);
}

void CAsmView::OnLButtonDown(UINT nFlags, CPoint point)
{
	int offsy = 0;
	int offsx = m_FirstXPos;
	int n = (point.y - offsy) / 15;
	int m = (point.x - offsx) / 8;
	m_CurY = n;
	if (m>=0)
		m_CurXWish=m_CurX = m;
	CAsmDoc* pDoc = GetDocument();
	pDoc->CursorValidate(m_CurX, m_CurY, m_CurXWish, 0);
	Invalidate();
	__super::OnLButtonDown(nFlags, point);
}

void CAsmView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	m_SizeLines = cy / 15 ;
}




void CAsmView::OnTimer(UINT_PTR nIDEvent)
{
	if (ID_ASMEDITORTIMER == nIDEvent) {
		m_CursorVisible ^= 1;
		Invalidate();
	}

	__super::OnTimer(nIDEvent);
}


void CAsmView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL CAsmView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta > 0) {
		//m_CurY -= 10;
		m_FirstLine -= 10;
		if (m_FirstLine < 0) m_FirstLine = 0;
	}
	else {
		//m_CurY += 10;
		m_FirstLine += 10;
		int n = GetDocument()->m_lines.GetCount();
		int m = n - m_SizeLines;
		if (m_FirstLine >m ) m_FirstLine = m;
	}
	GetDocument()->CursorValidate(m_CurX, m_CurY, m_CurXWish, 0);
	SetScrollPos(SB_VERT, m_FirstLine);
	Invalidate();
	return __super::OnMouseWheel(nFlags, zDelta, pt);
}
