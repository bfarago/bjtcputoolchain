/** @file AsmView.cpp
*
* @brief Asm file View implementation
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
#include "CSimulator.h"
#include "idebDoc.h"

//TODO: check if this global should be different from the CSimulator one, or may be combined...
static const TCHAR *gAsmMnemonics[17] =
{ _T("mvi a,"),_T("sta"),_T("lda"),_T("ad0"),_T("ad1"),_T("adc"),_T("nand"),_T("nor"),_T("rrm"),_T("jmp"),_T("jc "),_T("jnc"),_T("jz "),_T("jnz"),_T("jm "),_T("jp "),_T("INVALID") };

//ASM view timer, for blinky cursor...
#define ID_ASMEDITORTIMER 1001

//Printer related defines
#define HEADER_HEIGHT (2)
#define FOOTER_HEIGHT (2)
#define LEFT_MARGIN (4)
#define RIGHT_MARGIN (4)

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
	m_CurX(0), m_CurY(0), m_CurXWish(0),
	m_PenCursor(PS_SOLID, 2, (COLORREF)0)
{
	m_FontMonospace.CreateStockObject(SYSTEM_FIXED_FONT); //ANSI_FIXED_FONT);
	hIconBreak = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_BREAK),
		IMAGE_ICON, 16, 16, 0);
	hWarning = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_WARNING),
		IMAGE_ICON, 16, 16, 0);
	m_BreakpointsAreaVisible = TRUE;
	m_LineNrAreaVisible = TRUE;
	m_WarningAreaVisible = TRUE;
	m_DisAsmAreaVisible = TRUE;
	m_PrintMaxColumn = 80;
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
	int maxlines = (int)GetDocument()->m_lines.GetCount();
	SetScrollRange(SB_VERT, 0, maxlines);
}

BOOL CAsmView::OnWorkspaceEvent(CWorkspaceEvent * pEvent)
{
	if (pEvent) {
		if (pEvent->FileId == GetDocument()->m_FileId) {
			switch (pEvent->EventType) {
			case CWorkspaceEvent::EnsureSourceLineVisible:
				GoToSourceLine(pEvent->Line, FALSE);
				break;
			case CWorkspaceEvent::GoToSourceLine:
				GoToSourceLine(pEvent->Line);
				break;
			}
		}
	}
	return 0;
}

void CAsmView::OnDraw(CDC* pDC)
{
	BOOL isPrinting = pDC->IsPrinting();
	CScrollBar* pSV = GetScrollBarCtrl(SB_VERT);
	int startLine = GetScrollPos(SB_VERT);
	CWorkspace* pWs = GetWorkspace();
	if (!pWs) {
		//TODO: check if the opened asm file belongs to the workspace...
		//TODO: close views, when workspace closed...
		//TODO: Workspace and project will be refactored soon. Figure out, how? :)
		RegisterToActiveWorkspace();
		pWs = GetWorkspace();
	}
	CidebDoc* pProj = (CidebDoc*)pWs;
	CSimulator * pSimulator = NULL;
	if (pWs)
		pSimulator= pWs->GetSimulator();
	CAsmDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CDC m_DCTmp;
	CDC* pDCTmp;
	CBitmap m_BitmapTmp;
	CRect r;
	GetClientRect(&r);
	TEXTMETRIC tm;

	//this layout specific variables are updated by display mode, and member variables.
	BOOL LineNrAreaVisible;
	BOOL BreakpointsAreaVisible;
	BOOL DisAsmAreaVisible;
	BOOL WarningAreaVisible;
	int firstX = 0;
	int firstXWarning = 0;
	int firstXLineNr = 0;
	int firstXDisAsm = 0;
	int lastXLineNr = 0;
	int lastXDisAsm = 0;

	if (!isPrinting) {
		pDCTmp = &m_DCTmp;
		m_DCTmp.CreateCompatibleDC(pDC);
		CPalette* pPalette = pDC->GetCurrentPalette();
		m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
		m_DCTmp.SelectObject(&m_BitmapTmp);
		pDCTmp->SelectObject(&m_FontMonospace);
		pDCTmp->GetTextMetrics(&tm);
		BreakpointsAreaVisible = m_BreakpointsAreaVisible;
		LineNrAreaVisible = m_LineNrAreaVisible;
		DisAsmAreaVisible = m_DisAsmAreaVisible;
		WarningAreaVisible = m_WarningAreaVisible;
	}
	else {
		pDCTmp = pDC;
		pDCTmp->GetTextMetrics(&tm);
		r = m_RcPage;
		r.right -= (LEFT_MARGIN + RIGHT_MARGIN) * m_PrintCharWidth * 2;
		r.bottom = m_nRowsPerPage*(tm.tmHeight); //todo: +tm.tmExternalLeading ?
		BreakpointsAreaVisible = FALSE; //always false
		LineNrAreaVisible = TRUE; //always true
		DisAsmAreaVisible = TRUE;
		WarningAreaVisible = m_WarningAreaVisible;
		firstX = m_RcPage.left;
	}

	/*Todo: figure out some optimal and user friendly color schame/palette handling.
	So obviously this variables should be stay somewhere else then.
	*/

	COLORREF crBlack = 0; 
	COLORREF crLineNrText = RGB(0x60, 0x90, 0x60);
	COLORREF crCursorBk = RGB(0xf0, 0xf0, 0xf0);
	COLORREF crTextAreaBk = RGB(0xff, 0xff, 0xff);
	COLORREF crLineNrAreaBk = RGB(0xf0, 0xff, 0xf0);
	COLORREF crDisAsmAreaBk = RGB(0xd0, 0xd0, 0xd0);
	COLORREF crProgramCounter = RGB(0xff, 0xdf, 0xdf);
	COLORREF crBrakepointAreaBk = RGB(0xff, 0xfd, 0xfd);
	COLORREF crWarningAreaBk = RGB(0xef, 0xef, 0xef);

	pDCTmp->SetBkMode(TRANSPARENT);
	LONG n = (LONG) pDoc->m_lines.GetCount();
	CRect tr;
	
	tr.left = 0; tr.right = r.Width();


	if (pSimulator) {
		DisAsmAreaVisible &= pSimulator->IsDebugFileLoaded();
		BreakpointsAreaVisible &= pSimulator->IsDebugFileLoaded();
	}
	else {
		m_DisAsmAreaVisible = FALSE;
		BreakpointsAreaVisible = FALSE;
	}
	if (BreakpointsAreaVisible) {
		firstX += 16;// icon width
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = 0; tr.right = 16;
		pDCTmp->FillSolidRect(&tr, crBrakepointAreaBk);
	}
	if (DisAsmAreaVisible) {
		firstXDisAsm = firstX;
		firstX += tm.tmAveCharWidth* (3 + 1 + 6 + 1 + 5);
		lastXDisAsm = firstX;
		if (!pDC->IsPrinting()) {
			tr.top = 0; tr.bottom = r.bottom;
			tr.left = firstXDisAsm; tr.right = lastXDisAsm;
			pDCTmp->FillSolidRect(&tr, crDisAsmAreaBk);
		}
	}
	int linesInFile = (int)pDoc->m_lines.GetCount();
	if (LineNrAreaVisible) {
		firstXLineNr = firstX;
		lastXLineNr = firstX += (int)( tm.tmAveCharWidth* (log10(linesInFile)+1));
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = firstXLineNr; tr.right = lastXLineNr;
		pDCTmp->FillSolidRect(&tr, crLineNrAreaBk);
	}
	if (WarningAreaVisible) {
		firstXWarning = firstX;
		firstX += 16;
		tr.top = 0; tr.bottom = r.bottom;
		tr.left = lastXLineNr; tr.right = firstX;
		pDCTmp->FillSolidRect(&tr, crWarningAreaBk);
	}
	m_FirstXPos = firstX;
	tr.top = 0; tr.bottom = r.bottom;
	tr.left = firstX; tr.right =r.right;
	pDCTmp->FillSolidRect(&tr, crTextAreaBk);

	int DbgMemLocation = 0;
	int pc = 0;
	if (pSimulator) pc= pSimulator->GetPc();

	int nFirstLine;
	int nLines;
	if (m_FirstLine < 0) m_FirstLine = 0;
	if (m_FirstLine >= linesInFile) m_FirstLine = linesInFile-1;
	if (!pDC->IsPrinting()) {
		nFirstLine = m_FirstLine;
		if (linesInFile < m_FirstLine + m_SizeLines)
			m_SizeLines = linesInFile - m_FirstLine;
		nLines = m_SizeLines;
	}
	else {
		nFirstLine = ( m_PrintCurPage-1) * m_nRowsPerPage;
		if (nFirstLine >= linesInFile) nFirstLine = linesInFile - 1;
		nLines = m_nRowsPerPage;
		if (linesInFile <= nFirstLine + nLines)
			nLines = linesInFile - nFirstLine;
		if (nLines < 0) nLines = 0;
	}
	
	//Iterates through the displayed lines.
	//TODO: I would like to implement line-wrap mode too, especially for the printer mode.
	//So, design should be change accordingly. Right now, each source line could be exactly one row only.
	for (LONG i = 0; i < nLines; i++) {
		CAsmDoc::LineData* ld = &(pDoc->m_lines.GetAt(i+ nFirstLine));
		tr.top = i * tm.tmHeight; tr.bottom = tr.top + tm.tmHeight;
		if (ld->memAddress >= 0) {
			if (pSimulator) {
				if (DisAsmAreaVisible) {
					//disasm column will be generated by simulator class
					if (!pDC->IsPrinting()) {
						if (pc == ld->memAddress) {
							tr.left = 0; tr.right = r.right;
							pDCTmp->FillSolidRect(&tr, crProgramCounter);
						}
					}
					tr.left = firstXDisAsm; tr.right = lastXDisAsm;
					pSimulator->OnDrawDisasm(pDCTmp, tr, ld->memAddress);
				}
				if (BreakpointsAreaVisible) {
					//brakepoint dot will be printed, when necessary
					if (!pDC->IsPrinting()) {
						if (pSimulator->IsBreakPC(ld->memAddress)) {
							pDCTmp->DrawIcon(0 - 16, tr.top - 8, hIconBreak);
						}
					}
				}
			}
		}
		if (WarningAreaVisible) {
			if (!pDC->IsPrinting()) {
				int ne = (int)pSimulator->m_Errors.GetCount();
				for (int ie = 0; ie < ne; ie++) {
					const tErrorRecord& rErr = pSimulator->m_Errors.GetAt(ie);
					if (rErr.lineNr == ld->number) {
						pDCTmp->DrawIcon(firstXWarning - 16, tr.top - 8, hWarning);
					}
				}
			}
		}
		if (LineNrAreaVisible) {
			//display the actual lines number
			CString s;
			s.Format(L"%d", ld->number);
			tr.left = firstXLineNr; tr.right = lastXLineNr;
			pDCTmp->SetTextColor(crLineNrText);
			pDCTmp->DrawText(s, tr, DT_RIGHT);
		}

		if (!pDC->IsPrinting()) {
			if (m_CurY == i) {
				//Display a different background on the cursors row.
				tr.left = firstX; tr.right = r.right;
				pDCTmp->FillSolidRect(&tr, crCursorBk);
			}
		}

		//Draw source code line
		//One line of the source code stored in a list of tokens.
		//The tokeniser actually implemented by the AsmDoc. These represents not only the text snipets,
		//but the color codes, and visual coordinate x. This is needed because of the tabulators, and so on...
		int nTokens = (int) (ld->tokens.GetCount());
		if (nTokens) {
			for (int q = 0; q < nTokens; q++) {
				const CAsmDoc::TokenData& td = ld->tokens[q];
				int charWidth= tm.tmAveCharWidth;
				int x = td.column * charWidth;
				tr.left = firstX + x; tr.right = tr.left + (td.count+1) * charWidth;
				pDCTmp->SetTextColor(td.color);
				int count = td.count;
				if (isPrinting) {
					//line wrap is needed for print layout right now. This will truncate the line now.
					//TODO: think through, how is the best to handle multi-rows wrapped lines.
					if (td.first+count > m_PrintMaxColumn) {
						count = m_PrintMaxColumn - td.first;
					}
				}
				pDCTmp->DrawText(ld->text.Mid(td.first, count), tr, DT_TOP | DT_EXPANDTABS);
			}
			pDCTmp->SetTextColor(crBlack);
		}
		else
		{
			//In case of the tokenizer was not successful, at least the text member should be
			//displayed as is in one. This should not be runed normally.
			CString s = ld->text;
			tr.left = firstX; tr.right = r.right;
			pDCTmp->DrawText(s, tr, DT_TOP | DT_EXPANDTABS);
		}
	}
	
	//further screen specific drawing, like blinking cursor, and double buffering of the whole screen.
	if (!isPrinting) {
		if (m_CursorVisible) {
			CPen* pOldPen = pDCTmp->SelectObject(&m_PenCursor);
			//pDCTmp->SelectStockObject(BLACK_PEN);
			tr.top = m_CurY * tm.tmHeight; tr.bottom = tr.top + tm.tmHeight;
			pDCTmp->MoveTo(firstX + m_CurX * tm.tmAveCharWidth, tr.top);
			pDCTmp->LineTo(firstX + m_CurX * tm.tmAveCharWidth, tr.bottom);
			pDCTmp->SelectObject(pOldPen);
		}
		pDC->BitBlt(r.left, r.top, r.Width(), r.Height(), &m_DCTmp, 0, 0, SRCCOPY);
		GdiFlush();
		m_DCTmp.DeleteDC();
		m_BitmapTmp.DeleteObject();
	}
}



// CAsmView printing
// TODO: this section is on progress, will be cleaned later.

void CAsmView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

//Draws a horizontal line, used in printer specific layouts only.
void CAsmView::DrawLineAt(CDC *pDC, unsigned int y)
{
	pDC->MoveTo(0, y);
	pDC->LineTo(m_RcPage.right - m_RcPage.left, y);//Use the page_rc to figure out the width of the line
}

//The printed page have its own header. The heading contains the filename maybe...
void CAsmView::PrintHeader(CDC *pDC, CPrintInfo *pInfo)
{
	pDC->SelectObject(&m_FontHeader);
	pDC->SetTextColor(RGB(0, 0, 0));//Black text on
	pDC->SetBkColor(RGB(255, 255, 255));//White paper

	CRect rc = m_RcPage;
	rc.bottom = rc.top + HEADER_HEIGHT*m_nRowHeight;
	rc.right -= (LEFT_MARGIN+RIGHT_MARGIN) * m_PrintCharWidth*2;
	DrawLineAt(pDC, rc.bottom);     //draws a horiz. line
	rc.OffsetRect(0, m_nRowHeight / 2);
	//print a title on top center margin
	pDC->DrawText(m_TitleStr, &rc, DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_CENTER | DT_NOCLIP);//DT_RIGHT
}


//print footer separated by a horiz. line. The date, and page number may printed to the footer.
//****************************PRINT_FOOTER****************************************
void CAsmView::PrintFooter(CDC *pDC, CPrintInfo *pInfo)
{
	CRect     rc = m_RcPage;
	rc.top = rc.bottom - FOOTER_HEIGHT * m_nRowHeight;
	rc.bottom = rc.top + m_nRowHeight;
	rc.right -= (LEFT_MARGIN + RIGHT_MARGIN) * m_PrintCharWidth;
	DrawLineAt(pDC, rc.top);     //draw h. line

	//draw page number to the left
	CString   sTemp;
	rc.OffsetRect(0, m_nRowHeight / 2);
	sTemp.Format(L"%d", pInfo->m_nCurPage);
	pDC->DrawText(sTemp, -1, rc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

	//Now draw the DateStr at bottom-right of page
	pDC->DrawText(m_DateStr, -1, rc, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
}


//This function sets alls of the row and metric member vars
//TODO: Cleanup the design and code soon...
void CAsmView::ComputeMetrics(CDC *pDC)
{
	//This includes width for all columns
	CRect row_rc;
	//lc->GetItemRect(0, &row_rc, LVIR_BOUNDS);

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int nHeight = tm.tmHeight + tm.tmExternalLeading+1;
	int nWidth = tm.tmAveCharWidth;
	m_PrintCharWidth = nWidth;
	row_rc.top = row_rc.left = 0;
	row_rc.bottom = nHeight;
	row_rc.right = (10+80)* nWidth;


	//Get the list control window DC
	CDC  *pCtlDC = pDC; //lc->GetDC(); if (NULL == pCtlDC) return;

	//so we can get the avg character width
	//TEXTMETRIC tm;
	//pCtlDC->GetTextMetrics(&tm);

	//Lets get the ratios for scaling to printer DC
	//Fit all columns to 1 page, regardless of column number.
	m_PrintRatioX = pDC->GetDeviceCaps(HORZRES) / (row_rc.Width() + (LEFT_MARGIN + RIGHT_MARGIN)*tm.tmAveCharWidth);

	//width of pDC/whats got to fit into it in lcDC units
	m_PrintRatioY = pDC->GetDeviceCaps(LOGPIXELSY) / pCtlDC->GetDeviceCaps(LOGPIXELSY);

	//lc->ReleaseDC(pCtlDC);

	//Set up a page rc in list control units that accounts for left and right margins
	m_RcPage.SetRect(0, 0, pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
	m_RcPage.bottom = m_RcPage.bottom / m_PrintRatioY;//Convert units to List Control
	m_RcPage.right = m_RcPage.right / m_PrintRatioX;
	m_RcPage.left = LEFT_MARGIN * tm.tmAveCharWidth;//adjust sides for magins
	m_RcPage.right -= RIGHT_MARGIN * tm.tmAveCharWidth;

	m_nRowHeight = row_rc.Height();//Get the height of a row.
	int pRowHeight = (int)(m_nRowHeight*m_PrintRatioY);//Get RowHeight in printer units.
	m_nRowsPerPage = pDC->GetDeviceCaps(VERTRES) / pRowHeight;//How many rows will fit on page?
	m_nRowsPerPage -= (HEADER_HEIGHT + FOOTER_HEIGHT);//After header and footer rows
	m_nRowsPerPage -= 1; //After header Control row
}

//This function is called by the system to calculate the max number of pages mainly...
BOOL CAsmView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (pInfo == NULL)          return FALSE;

	//Lets make a guess as to how many pages there are based on the default printer.
	CPrintDialog pdlg(FALSE);
	if (!pdlg.GetDefaults()) return FALSE;//If no defaults then no printer!!
	CDC t_pDC;
	t_pDC.Attach(pdlg.GetPrinterDC());
	ComputeMetrics(&t_pDC);

	CAsmDoc* pDoc = GetDocument();
	if (!pDoc) return FALSE;
	m_nMaxRowCount = (int) pDoc->m_lines.GetCount();

	if (!m_nMaxRowCount) return FALSE;//Get the number of rows
	int nMaxPage = m_nMaxRowCount / m_nRowsPerPage + 1;
	pInfo->SetMaxPage(nMaxPage);
	pInfo->m_nCurPage = 1;  // start printing at page# 1

	//If you want to be able to do this remove it.
	//pInfo->m_pPD->m_pd.Flags |= PD_HIDEPRINTTOFILE;

	return DoPreparePrinting(pInfo);
}

//To set the actual page related things.
void CAsmView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CView::OnPrepareDC(pDC, pInfo);
	if (pDC->IsPrinting())
	{
		int m_nPageHeight = pDC->GetDeviceCaps(VERTRES);
		int y = (pInfo->m_nCurPage - 1)*m_nPageHeight;
		m_PrintCurPage = pInfo->m_nCurPage;
		//pDC->SetViewportOrg(0, y); // -y: remove the minus sign if you are printing in MM_TEXT
		//TODO: Margins... Check the MFC print speciality, maybe there is some tricks/best practics to handle this...
	}
}

//This function will be called by the system before the whole document printing started.
void CAsmView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	/*
	//think through the design, what we need...
	int horzsize = pDC->GetDeviceCaps(HORZSIZE); // gives the width of the physical display in millimeters
	int vertsize = pDC->GetDeviceCaps(VERTSIZE); // gives the height of the physical display in millimeters
	int horzres = pDC->GetDeviceCaps(HORZRES); // gives the height of the physical display in pixels
	int vertres = pDC->GetDeviceCaps(VERTRES); // gives the width of the physical display in pixels
	int hdps = horzres / horzsize; // calculate the horizontal pixels per millimeter
	int vdps = vertres / vertsize; // calculate the verticalpixels per millimeter
	CString s = L"TEST STRING";
	CRect r;
	r.top = 0;
	r.bottom = vdps * 5;
	r.left = 0;
	r.right = hdps * 5;
	pDC->Rectangle(&r);
	pDC->DrawText(s, &r, DT_TOP);*/

	if (pDC == NULL || pInfo == NULL)
		return;

	//TODO: get the infos from real sources, like filename and date...
	m_TitleStr = L"Test Print";
	m_DateStr = L"2019/01/01 13:25:00";
	
	/*
	//create spec. font here, if needed
	LOGFONT  lf;
	CFont * lcfont_ptr = lc->GetFont();
	lcfont_ptr->GetLogFont(&lf);
	lcFont.CreateFontIndirect(&lf);
	lf.lfWeight = FW_BOLD;
	lf.lfHeight += 22;//Make it a little bigger
	lf.lfWidth = 0;
	BoldFont.CreateFontIndirect(&lf);
	*/
	ComputeMetrics(pDC);
	int nMaxPage = m_nMaxRowCount / m_nRowsPerPage + 1;//Compute this again in case user changed printer
	pInfo->SetMaxPage(nMaxPage);
	pInfo->m_nCurPage = 1;  // start printing at page# 1

}

void CAsmView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	//De-allocate things of the beginPrinting, if any...
	//m_FontPrint.DeleteObject();
}

void CAsmView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	/*
	int horzsize = pDC->GetDeviceCaps(HORZSIZE); // gives the width of the physical display in millimeters
	int vertsize = pDC->GetDeviceCaps(VERTSIZE); // gives the height of the physical display in millimeters
	int horzres = pDC->GetDeviceCaps(HORZRES); // gives the height of the physical display in pixels
	int vertres = pDC->GetDeviceCaps(VERTRES); // gives the width of the physical display in pixels
	int hdps = horzres / horzsize; // calculate the horizontal pixels per millimeter
	int vdps = vertres / vertsize; // calculate the verticalpixels per millimeter
	CString s = L"TEST STRING";
	CRect r;
	r.top = 0;
	r.bottom = vdps * 5;
	r.left = 0;
	r.right = hdps * 5;
	pDC->Rectangle(&r);
	pDC->DrawText(s, &r, DT_TOP);
	CRect rectDraw = pInfo->m_rectDraw;
	// this assumes the page is A4, the printer can print without margins
	// (this is not very good to assume but will work for now)
	CRect rectOut(rectDraw.left, rectDraw.top, rectDraw.left + 210 * hdps, rectDraw.top + 297 * vdps);

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int nHeight = tm.tmHeight + tm.tmExternalLeading;
	//__super::OnPrint(pDC, pInfo);
	*/

	if (NULL == pDC || NULL == pInfo)
		return;

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int nHeight = tm.tmHeight + tm.tmExternalLeading;

	//This has to be in OnPrint() or else PrintPreview goes screwy
	CFont* pOldFont = pDC->GetCurrentFont();

	//Fit all columns to 1 page, regardless of column number.
	pDC->SetMapMode(MM_ANISOTROPIC);

	//For every 1 List Control pixel
	pDC->SetWindowExt(1, 1);

	//The printer has ratio more dots
	pDC->SetViewportExt(m_PrintRatioX, m_PrintRatioY);

	int nStartRow = (pInfo->m_nCurPage - 1)*m_nRowsPerPage;
	int nEndRow = nStartRow + m_nRowsPerPage;
	if (nEndRow > m_nMaxRowCount)
		nEndRow = m_nMaxRowCount;

	pDC->SetWindowOrg(-1 * m_RcPage.left, 0);
	PrintHeader(pDC, pInfo);     //print the header
	PrintFooter(pDC, pInfo);	  //Print the footer

	pDC->SetWindowOrg(-1 * m_RcPage.left, -1 * HEADER_HEIGHT*m_nRowHeight);
	//PrintHeader(pDC, pInfo);//Print the header Control, Manually

	pDC->SelectObject(&m_FontPrint);
	pDC->SetTextColor(RGB(0, 0, 0));//Black text on
	pDC->SetBkColor(RGB(255, 255, 255));//White paper

	/*
	CRect     rcBounds;
	//lc->GetItemRect(nStartRow, &rcBounds, LVIR_BOUNDS);

	//offset top margin of rcBounds by ListControl header
	CRect     rc;
	//lc->GetHeaderCtrl()->GetClientRect(&rc);
	GetClientRect(&rc);
	rcBounds.OffsetRect(0, -rc.Height());
	pDC->OffsetWindowOrg(rcBounds.left, rcBounds.top);
	*/
	pDC->OffsetWindowOrg(-m_RcPage.left, - HEADER_HEIGHT *nHeight);
	//start printing rows
	//for (int i = nStartRow; i < nEndRow; i++) DrawRow(pDC, i);

	OnDraw(pDC); // The common draw function (body of the printed page)

	//SetWindowOrg back for next page
	pDC->SetWindowOrg(0, 0);
	pDC->SelectObject(pOldFont);//Put the old font back
}

void CAsmView::GoToSourceLine(int line, BOOL bCursorUpdate)
{
	if ( (m_FirstLine > line) ||
		 (m_FirstLine + m_SizeLines <= line) )
	{
		m_FirstLine = line;
		m_CurY = 0;
		if (bCursorUpdate) {
			GetParentFrame()->ActivateFrame(SW_RESTORE);
		}
		Invalidate();
		return;
	}
	if (bCursorUpdate) m_CurY= line - m_FirstLine;
	Invalidate();
	if (bCursorUpdate) {
		GetParentFrame()->ActivateFrame(SW_RESTORE);
	}
}

//Right-click for context menu
void CAsmView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

//Context menu
void CAsmView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

//should be handled, to eliminate the flickering
BOOL CAsmView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE; //CView::OnEraseBkgnd(pDC); //removed - do not clear background, it is double buffered.
}

//aah, yeah... We need this for GDI and wnd handle related fn calls really to work...
void CAsmView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	UpdateScollbars();
	SetTimer(ID_ASMEDITORTIMER, 600, NULL);
}

//keydown handler of the asm view.
void CAsmView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CAsmDoc* pDoc= GetDocument();
	int maxlines = (int)pDoc->m_lines.GetCount();
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

//maybe later we need this obviously, but not now.
void CAsmView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	/*CAsmDoc* pDoc = GetDocument();
	if (pDoc) {
		//pDoc->OnKeyUp(nChar, nRepCnt, nFlags);
	}*/
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

//Convert Mouse point to line number, it will be more trickier later, when multi rows are possible!
int CAsmView::GetLineAtMouse(CPoint point) {
	int offsy = 0;
	int n = (point.y - offsy) / 15;
	//todo: handle multi rows and line wrap thing later!
	return n+m_FirstLine;
}

//Left mouse button double click handler, especially on the brakepoint are, it is toggle the brakepoint.
void CAsmView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_BreakpointsAreaVisible) {
		CSimulator* pSim = GetSimulator();
		if (pSim) {
			if (point.x <= 15) {
				int line = GetLineAtMouse(point);
				CAsmDoc::LineData& ld = GetDocument()->m_lines.GetAt(line);
				if (ld.memAddress >= 0) {
					pSim->SetBrakePC(ld.memAddress);
					Invalidate();
				}
			}
		}
	}
	//Todo: also double click on a token may have some sense...
	__super::OnLButtonDblClk(nFlags, point);
}

/*OnLButtonDown:
Left mouse button is actually pressed down, and not moving.
This will move to the character cursor as a required position.
The cursor x pos also will be save as a wish. This is good to remember, where the user
clicked before, even if the row is shorter than the location right now, the cursor up/down
will use this information, to choose the appropriate x location next time...
*/
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

/*OnSize:
Chage the maximal row counts.
MAYBUG: Review and debug this more, somete usecase is not covered well!
*/
void CAsmView::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	//TODO: use char height from metrics rather than constant number, this is wrong.
	m_SizeLines = cy / 15 ;
}

//OnTimer: Now only for cursor blink... Btw: all of the screen will be updated.
void CAsmView::OnTimer(UINT_PTR nIDEvent)
{
	if (ID_ASMEDITORTIMER == nIDEvent) {
		m_CursorVisible ^= 1;
		Invalidate();
	}

	__super::OnTimer(nIDEvent);
}

//Vertical scroll
void CAsmView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//TODO: something is missing from the implementation, because it is never called by the MFC.
	__super::OnVScroll(nSBCode, nPos, pScrollBar);
}

/**OnMouseWheel: works, but...
TODO: this is actually a workaround, because of the scrollbars are not working as I thought...
The original CScrollView implementation is not what we want in one hand, in other hand the CView
have no scrollbar behaviours by default, sadly...
*/
BOOL CAsmView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta > 0) {
		m_FirstLine -= 10;
		if (m_FirstLine < 0) m_FirstLine = 0;
		else {
			m_CurY += 10;
			if (m_CurY >= m_SizeLines) m_CurY = m_SizeLines - 1;
		}
	}
	else {
		m_FirstLine += 10;
		int n =(int) GetDocument()->m_lines.GetCount();
		int m = n - m_SizeLines;
		if (m_FirstLine >m ) m_FirstLine = m;
		else {
			m_CurY -= 10;
			if (m_CurY < 0) m_CurY = 0;
		}
	}
	GetDocument()->CursorValidate(m_CurX, m_CurY, m_CurXWish, 0);
	SetScrollPos(SB_VERT, m_FirstLine);
	Invalidate();
	return __super::OnMouseWheel(nFlags, zDelta, pt);
}

