/** @file simulator.cpp
*
* @brief CSimulator model/controller implementation
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "stdafx.h"
#include "CSimulator.h"
#include "idebdoc.h"

#include "Resource.h"

//this defines are obsolate, will be removed soon
#define OLD_TEXT_STARTX 16
#define OLD_TEXT_STARTY 16
#define OLD_TEXT_DISTANCEX 16

//this defines used by OnDraw related codes
#define DISTANCEY 16
#define HEXDISTANCEX 8
#define HEXDISTANCEY 17

//these defines are used by simulated screen drawer code
#define BM_SRT_Y (40) // start y (top) on the window
#define BM_SRT_X (50) // start x (left) on the window
#define BM_OFS_Y (13) // original height of a character in the bitmap in pixels
#define BM_OFS_X (13) // original width of a character in the bitmap in pixels

//textual representation of each instructions.
static const TCHAR *gMnemonics[17] =
{ _T("mvi a,"),_T("sta"),_T("lda"),_T("ad0"),_T("ad1"),_T("adc"),_T("nand"),_T("nor"),_T("rrm"),_T("jmp"),_T("jc "),_T("jnc"),_T("jz "),_T("jnz"),_T("jm "),_T("jp "),_T("INVALID")};

//textual representation of each internal states of the clock accurate simulator
static const TCHAR *gSimStates[9] =
{ _T("Halt     "), _T("FetchOp  "), _T("FetchImm0"), _T("FetchImm1"), _T("FetchImm2"),
  _T("Decode   "), _T("Load     "), _T("Alu      "), _T("Store    ") };

//CPU peripheral addresses
enum {
	ADDR_ARR =0xc00,
	ADDR_RND,
	ADDR_KEY0,
	ADDR_KEY1,
	ADDR_SCREEN_X,	//mask:0xc04
	ADDR_SCREEN_Y,
	ADDR_SCREEN_CH1, //High
	ADDR_SCREEN_CH0, //Low
	ADDR_UART_H,   // mask:0xc08
	ADDR_UART_L,
	ADDR_PERIPH_MAX
};
/*
arr		equ 3072
rnd		equ 3073
key0	equ 3074
key1	equ 3075
cord_x	equ 3076
cord_y	equ 3077
ch_h	equ 3078
ch_l	equ 3079
*/

//SimScreen type: holds the all internal states of the video card.
typedef struct {
	char x, y;
	char ch0, ch1;
	unsigned char buf[16][16];
	TCHAR fakeChar;
} SimScreen_t;

//SimScreen global singleton. Todo: move to class if we wana run multiple instances...
SimScreen_t SimScreen;

/**SimCode2Ascii global array to resolve the simulated video card character generator charset.
*/
TCHAR SimCode2Ascii[257] = _T(
	"0123456789ABCDEF" // 0
	"GHIJKLMNOPQRSTUV" // 1
	"WXYZ.,:;!?><=+-/" // 2
	"\\()#©½¤»[_]/\\/\\f" // 3
	"                " // 4
	"                " // 5
	"                " // 6
	"                " // 7
	"                " // 8
	"                " // 9
	"                " // a
	"                " // b
	"                " // c
	"                " // d
	"                " // e
	"               ." // f
);

/**CSimulator constructor
*/
CSimulator::CSimulator()
	:m_pDoc(0), m_MemorySizeLoaded(0), m_Pc(0),m_CpuHz(SIM_HZ),m_Time(0), m_ClockCount(0), m_State(ss_Halt),
	m_UartFromCpuWr(0), m_CpuSnapshot_p(0), m_Stop(FALSE), m_DisplayMeasurement(TRUE), m_DisplayMemory(TRUE), m_DisplayDebugMonitor(TRUE),
	m_ExecTimeActual(0), m_DrawTimeActual(0)
{
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			unsigned char code = 0xff;
			SimScreen.buf[x][y] = code;
		}
	}
	for (int i = 0; i < SIM_MAXMEMORYSIZE; i++) {
		m_HeatPc[i] = 0;
		m_HeatRead[i] = 0;
		m_HeatWrite[i] = 0;
		m_Memory[i] = 0;
		m_Break[i] = 0;
		m_MemoryMeta[i].sectionId = 0;
		m_MemoryMeta[i].fileId = 0;
		m_MemoryMeta[i].line = 0;
		m_MemoryMeta[i].sectionType = MT_code;
	}
	m_CpuSnapshot_p = &m_CpuSnapshot; //TODO: implement a debug timeline possibility later
	m_CpuSnapshot.op = 16;
	m_CpuSnapshot.state= cs_Fetch;
	m_CpuSnapshot.acc = 0;
	m_CpuSnapshot.addr = 0;
	m_CpuSnapshot.carry = 0;
	m_CpuSnapshot.cpuTick = 0;
	m_CpuSnapshot.cpuTime = 0;
	m_CpuSnapshot.immediate = 0;
	m_CpuSnapshot.pc = 0;
	m_CpuSnapshot.pcnext = 0;
	m_UartFromCpuBuf[0] = 0;
	hIconBreak = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON_BREAK), 
		IMAGE_ICON, 16, 16, 0);
	
	m_bitmapScopeBg.LoadBitmap(IDB_SCOPE_BG);
	m_bitmapAbc.LoadBitmap(IDB_ABC);
	ResetMeasurement();
	m_FontMonospace.CreateStockObject(SYSTEM_FIXED_FONT);
}

CSimulator::~CSimulator()
{
	m_FontMonospace.Detach();
	m_DCTmp.DeleteDC();
	m_BitmapTmp.DeleteObject();
}

void CSimulator::SetDocument(CidebDoc * pDoc)
{
	m_pDoc = pDoc;
}

void CSimulator::SetPc(SimAddress_t pc)
{
	m_Pc = pc; //TODO: next pc or change state to fetch ?
}

inline void CSimulator::AddressBusDrive(SimAddress_t addr) {
	if (!m_CpuSnapshot_p) return;
	m_CpuSnapshot_p->addr = addr;
}

//OnDrawHexDump: View part of the monitor functionality. Displays memory contents.
int CSimulator::OnDrawHexDump(CDC* pDC, SimAddress_t aBegin, SimAddress_t aEnd, int sx, int sy) {
	CRect r;
	int y = sy;
	TCHAR b[2];
	b[1] = 0;
	for (int i = aBegin; i < aEnd; i++) {
		int offs = i - aBegin;
		int x = offs & 0x3f;
		y = (offs >> 6) + sy;
		COLORREF cBk = 0; 
		
		if (pDC->IsPrinting())
		{
			cBk = RGB(0xff, 0xff, 0xff);
			pDC->SetBkColor(cBk);
			pDC->SetTextColor(0);
		}else
		{
			if (m_Break[i])cBk = RGB(0xc0, 0, 0);
			pDC->SetBkColor(cBk);
			pDC->SetTextColor(cBk ^ 0xffffff);
		}
		r.top = 2 + y * HEXDISTANCEY;
		r.bottom = r.top + HEXDISTANCEY;
		r.left = 38+sx + x * HEXDISTANCEX;
		r.right = r.left + HEXDISTANCEX;
		if (m_HeatPc[i]) 
		{
			CPen p(PS_SOLID, 4, RGB(m_HeatPc[i], 0, 0));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left+1, r.bottom - 2);
			pDC->LineTo(r.right-1, r.bottom - 2);
			pDC->SelectObject(pOldPen);
		}

		SimData_t d = m_Memory[i];
		if (d > 15) {
			b[0] = '-';
			pDC->DrawTextW(b, &r, DT_TOP);
		}
		else
		{
			if (d > 9) d += '@' - '9';
			b[0] = 0x30 + d;
			pDC->DrawTextW(b, &r, DT_TOP);
		}
		//pDC->SetBkColor(RGB(0xff, 0xff, 0xff));
		//pDC->SetTextColor(COLORREF(0));
		if (m_Pc == i) {
			CPen penPc;
			penPc.CreatePen(PS_SOLID, 2, RGB(0xFF, 0x7F, 0x4f));
			CPen* pOldPen = pDC->SelectObject(&penPc);
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH);
			pDC->SelectObject(&brush);
			pDC->Rectangle(&r);
			pDC->SelectObject(pOldPen);
		}

		if (m_HeatRead[i]) 
		{
			CPen p(PS_SOLID, 2, RGB(0, m_HeatRead[i], 0));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left, r.bottom);
			pDC->LineTo(r.right, r.bottom);
			pDC->SelectObject(pOldPen);
		}
		if (m_HeatWrite[i]) 
		{
			CPen p(PS_SOLID, 2, RGB(0, 0, m_HeatWrite[i]));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left, r.bottom-3);
			pDC->LineTo(r.right, r.bottom-3);
			pDC->SelectObject(pOldPen);
		}
		if (!x) {
			CString s;
			r.left = sx+2;
			r.right = r.left + HEXDISTANCEX * 5;
			s.Format(_T("%03x:"), i);
			pDC->DrawTextW(s, &r, DT_TOP);
		}
	}
	return y;
}

/**OnDrawDisasm: View part of the monitor functionality, displays the disassembled list of a specific
memory location. It is used from different perspectives, like project view and asm view classes. */
SimAddress_t CSimulator::OnDrawDisasm(CDC* pDC, CRect& r, SimAddress_t a)
{
	CString s;
	char op = m_Memory[a];
	memoryMetaData_t* meta = &m_MemoryMeta[a];
	memoryType_t mt = MT_code;
	if (m_DbgFileLoaded) {
		mt = meta->sectionType; //m_MemoryType[a];
	}
	if (MT_code == mt) {
		short operand = m_Memory[a + 1];
		if (op) {
			operand |= m_Memory[a + 2] << 4 | m_Memory[a + 3] << 8;
			s.Format(_T("%03x %s 0x%x"), a, gMnemonics[op], operand);
		}
		else {
			s.Format(_T("%03x %s0x%x"), a, gMnemonics[op], operand);
		}

		pDC->DrawTextW(s, &r, DT_TOP);
		if (m_HeatPc[a])
		{
			CPen p(PS_SOLID, 2, RGB(m_HeatPc[a], 0, 0));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left, r.bottom - 2);
			pDC->LineTo(r.left + 52, r.bottom - 2);
			pDC->SelectObject(pOldPen);
		}
		if (m_HeatWrite[a + 1])
		{
			CPen p(PS_SOLID, 2, RGB(0, 0, m_HeatWrite[a + 1]));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left + HEXDISTANCEX * 12, r.bottom - 2);
			pDC->LineTo(r.left + HEXDISTANCEX * 13, r.bottom - 2);
			pDC->SelectObject(pOldPen);
		}
		if (m_HeatWrite[a + 2])
		{
			CPen p(PS_SOLID, 2, RGB(0, 0, m_HeatWrite[a + 2]));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left + HEXDISTANCEX * 11, r.bottom - 2);
			pDC->LineTo(r.left + HEXDISTANCEX * 12, r.bottom - 2);
			pDC->SelectObject(pOldPen);
		}
		if (m_HeatWrite[a + 3])
		{
			CPen p(PS_SOLID, 2, RGB(0, 0, m_HeatWrite[a + 3]));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left + HEXDISTANCEX * 10, r.bottom - 2);
			pDC->LineTo(r.left + HEXDISTANCEX * 11, r.bottom - 2);
			pDC->SelectObject(pOldPen);
		}
		a += (op) ? 4 : 2;
	}
	else {
		s.Format(_T("%03x"), a);
		pDC->DrawTextW(s, &r, DT_TOP);
		a += 1;
	}
	return a;
}
/**OnDraw: View part of the simulator, display the monitor functionality, like:
screen, memory map, disasm list. This blocks visibility are switchable by member variables.
*/
void CSimulator::OnDraw(CDC* pDC, int mode) {
	CRect r;
	int sy = 0;
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	CDC dcBg;
	dcBg.CreateCompatibleDC(pDC);
	dcBg.SelectObject(m_bitmapScopeBg);

	CDC dcAbc;
	dcAbc.CreateCompatibleDC(pDC);
	dcAbc.SelectObject(m_bitmapAbc);

	pDC->BitBlt(0, DISTANCEY, 301, 252, &dcBg, 0, 0, SRCCOPY);
	r.right = 400;//s.cx;
	r.bottom = 400;// s.cy;
	int sx = 0;
	int firstXDisasm = 0;
	if (1) {
		sx += 310;
	}
	if (m_DisplayMemory) {
		sx+= (4 + 64)*HEXDISTANCEX + 10;
	}
	firstXDisasm = sx;
	pDC->SetTextColor(COLORREF(0));
	TCHAR b[2];
	b[1] = 0;
	if (0) {
		//This section is obsolate now. Draw textual view of the screen.
		for (int y = 0; y < 16; y++) {
			r.top = OLD_TEXT_STARTY + y * DISTANCEY;
			r.bottom = r.top + DISTANCEY;
			for (int x = 0; x < 16; x++) {
				unsigned char code = SimScreen.buf[x][y];
				r.left = OLD_TEXT_STARTX + x * OLD_TEXT_DISTANCEX;
				r.right = r.left + OLD_TEXT_DISTANCEX;
				b[0] = SimCode2Ascii[code];
				pDC->DrawTextW(b, &r, DT_TOP);
			}
		}
	}

	if (1) {
		//This is the only used implementation of the screen emulator view part.
		for (int y = 0; y < 16; y++) {
			r.top = BM_SRT_Y + y * BM_OFS_Y;
			r.bottom = r.top + BM_OFS_Y;
			for (int x = 0; x < 16; x++) {
				unsigned char code = SimScreen.buf[x][y];
				unsigned char lx = code & 0xf;
				unsigned char ly = code >> 4;
				r.left = BM_SRT_X + x * BM_OFS_X;
				r.right = r.left + BM_OFS_X;
				pDC->TransparentBlt(r.left, r.top, BM_OFS_X, BM_OFS_Y, &dcAbc, lx*BM_OFS_X, ly*BM_OFS_Y, BM_OFS_X, BM_OFS_Y, RGB(0, 0, 0)); //RGB(0xff, 0xff, 0xff)
			}
		}
	}
	if (m_DisplayMemory){
		static int dmcount = 0;
		r.left = 310;
		r.right = r.left + (4+64)*HEXDISTANCEX+10;
		r.top = 15;
		r.bottom = r.top + (m_MemorySizeLoaded/64+2)*HEXDISTANCEY;
		if (pDC->IsPrinting()) {
			sy = OnDrawHexDump(pDC, 0, m_MemorySizeLoaded, r.left, 1) + 1;
			sy = OnDrawHexDump(pDC, ADDR_ARR, ADDR_PERIPH_MAX, r.left, sy) + 1;
		}
		else {
			if (!dmcount) {
				m_DCTmp.CreateCompatibleDC(pDC);
				CPalette* pPalette = pDC->GetCurrentPalette();
				m_BitmapTmp.CreateCompatibleBitmap(pDC, r.Width(), r.Height());
				m_DCTmp.SelectObject(&m_BitmapTmp);
				m_DCTmp.SelectObject(&m_FontMonospace);
				sy = OnDrawHexDump(&m_DCTmp, 0, m_MemorySizeLoaded, 0, sy) + 1;
				sy = OnDrawHexDump(&m_DCTmp, ADDR_ARR, ADDR_PERIPH_MAX, 0, sy) + 1;
			}
			pDC->BitBlt(r.left, r.top, r.Width(), r.Height(), &m_DCTmp, 0, 0, SRCCOPY);
			if (!dmcount) {
				m_DCTmp.DeleteDC();
				m_BitmapTmp.DeleteObject();
			}
		}
		
	}
	if (m_DisplayMeasurement) {
		CString s;
		r.top = 300;
		r.bottom = r.top+15;
		r.left = 0;
		r.right = 900;
		s.Format(_T("Draw period:%dms Refresh rate:%03.3f"),
			SIM_REFRESH_TIMER, 1000.0/ SIM_REFRESH_TIMER);
		pDC->DrawTextW(s, &r, DT_TOP);
		r.top += 15; r.bottom += 15;
		s.Format(_T("Exec[ms]:%03.3f Min:%03.3f Max:%03.3f"),
			m_ExecTimeAvg /1000.0, m_ExecTimeMin /1000.0, m_ExecTimeMax /1000.0);
		pDC->DrawTextW(s, &r, DT_TOP);
		r.top += 15; r.bottom += 15;
		s.Format(_T("Draw[ms]:%03.3f Min:%03.3f Max:%03.3f"),
			m_DrawTimeAvg /1000.0, m_DrawTimeMin /1000.0, m_DrawTimeMax /1000.0);
		pDC->DrawTextW(s, &r, DT_TOP);
		if (m_Time > 0) {
			r.top += 15; r.bottom += 15;
			int n = (int)(m_ClockCount / m_Time);
			s.Format(_T("MHz:%3.3f Instr/sec:%d"),
				4*n/1000000.0, n);
			pDC->DrawTextW(s, &r, DT_TOP);
		}
	}
	
	if (m_DisplayDebugMonitor) {
		r.top = 0;
		r.bottom = 15;
		r.left = 0;
		r.right = 900;
		if (rm_Run == m_RunMode) {
			CString s;
			s.Format(_T("Tick: %08dus Time: %03.6fs"),
				m_ClockCount, m_Time);
			pDC->DrawTextW(s, &r, DT_TOP);
		}

		if (m_CpuSnapshot_p) {
			CString s;
			char op = m_Memory[m_Pc]; //m_CpuSnapshot_p->op;
			char st = m_CpuSnapshot_p->state;
			short imm = m_CpuSnapshot_p->immediate;
			short adr = m_CpuSnapshot_p->addr;
			short operand;
			if (op > 15) op = 16; //invalid instruction
			if (st > 'S') st = '-';
			imm &= 0xfff;
			adr &= 0xfff;
			if (op) {
				operand = m_Memory[m_Pc + 1] | m_Memory[m_Pc + 2] << 4 | m_Memory[m_Pc + 3] << 8;
			}
			else {
				operand = m_Memory[m_Pc + 1];
			}
			if (rm_StepState == m_RunMode) {
				s.Format(_T("Tick: %08dus Time: %03.6fs PC: %03x Acc: %x +%d Imm: %03x Adr: %03x State: %c (%s) Op: %s 0x%x"),
					m_ClockCount, m_Time, m_Pc, m_CpuSnapshot_p->acc, m_CpuSnapshot_p->carry, imm, adr,
					st, gSimStates[m_State], gMnemonics[op], operand);
			}
			else {
				memoryMetaData_t* pMeta = GetMemoryMetaData(m_Pc);
				CString sSection;
				CString sFile;
				if (m_Sections.GetSize() > pMeta->sectionId) {
					sSection= m_Sections[pMeta->sectionId];
				}
				if (m_pDoc)
				if (m_pDoc->m_AsmFiles.GetSize() > pMeta->fileId) {
					sFile = m_pDoc->m_AsmFiles[pMeta->fileId];
				}
				int line = pMeta->line -1; //todo: asnb line wrong
				s.Format(_T("Tick: %08dus Time: %03.6fs PC: %03x Acc: %x +%d Op: %s 0x%x \t(%s) %s:%d"),
					m_ClockCount, m_Time, m_Pc, m_CpuSnapshot_p->acc, m_CpuSnapshot_p->carry,
					gMnemonics[op], operand,
					sSection, sFile, line
					);

			}
			pDC->DrawTextW(s, &r, DT_TOP| DT_EXPANDTABS);
			SimAddress_t a = m_Pc;
			for (int row = 0; row < 32; row++) {
				r.top = row * 16+16;
				r.bottom = r.top + 15;
				r.left = firstXDisasm + 10;//870+10;
				r.right = r.left + 130;
				if (m_Break[a]) {
					pDC->DrawIcon(r.left - 28, r.top - 8, hIconBreak);
				}
				a=OnDrawDisasm(pDC, r, a);
			}
		}
	}
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	m_DrawTimeActual = (int)ElapsedMicroseconds.QuadPart;
	ProcessDrawMeasurement();


}
void CSimulator::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	BOOL hit = FALSE;
	if (nChar == VK_UP) {
		hit = TRUE;
		m_Memory[ADDR_ARR] = ka_Fire;
	}
	if (hit) return;
	if (nChar == VK_SPACE) {
		unsigned char key = 0x11; //Todo: conversion?
		m_Memory[ADDR_KEY1] = (key >> 4) & 0x0f;
		m_Memory[ADDR_KEY0] = key & 0x0f;
	}
}
void CSimulator::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	/*
	//not used, better to acknowladge the keycode from cpu bus read operation, 
	//rather than windows keyboard up event.
	//Obviously a timeout will be also needed, if we want to base the keyup on this way...
	if (nChar == VK_UP) {
		m_Memory[ADDR_ARR] = 0;
	}
	if (nChar == VK_SPACE) {
		m_Memory[ADDR_KEY1] = 0;
		m_Memory[ADDR_KEY0] = 0;
	}
	*/
}
inline BOOL CSimulator::OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
	BOOL processed = TRUE;
	if (bd_Write == dir) {
		switch (addr) {
		case ADDR_SCREEN_X: SimScreen.x = *data; break;
		case ADDR_SCREEN_Y: SimScreen.y = *data; break;
		case ADDR_SCREEN_CH0: SimScreen.ch0 = *data; break;
		case ADDR_SCREEN_CH1: SimScreen.ch1 = *data; 
			;
		default: processed = FALSE;	break;
		}
		//activate shadow buffer write
		switch (addr) {
		case ADDR_SCREEN_CH0:
			{
				unsigned char code = (SimScreen.ch1 << 4) | SimScreen.ch0;
				SimScreen.buf[SimScreen.x][SimScreen.y] = code;
				SimScreen.fakeChar = SimCode2Ascii[code];
			}
			break;
		}
	}
	if (bd_Read == dir) {
		switch (addr) {
		case ADDR_SCREEN_X: *data=SimScreen.x; break;
		case ADDR_SCREEN_Y: *data = SimScreen.y; break;
		case ADDR_SCREEN_CH0: *data = SimScreen.ch0; break;
		case ADDR_SCREEN_CH1: *data = SimScreen.ch1; break;
		default: processed = FALSE;	break;
		}
	}
	return processed;
}

inline BOOL CSimulator::OnUartLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
	BOOL processed = TRUE;
	static unsigned char uart_ch;
	if (bd_Write == dir) {
		switch (addr) {
		case ADDR_UART_H: uart_ch = (*data)<<4; break;
		case ADDR_UART_L: uart_ch |= (*data)&0xf; break;
		default: processed = FALSE;	break;
		}
		//activate shadow buffer write
		switch (addr) {
		case ADDR_UART_L:
		{
			TCHAR c = uart_ch;
			m_UartFromCpuBuf[m_UartFromCpuWr] = c;
			m_UartFromCpuWr++;
			if (m_UartFromCpuWr >= 512) {
				//overrun
				m_UartFromCpuWr = 511;
			}
			m_UartFromCpuBuf[m_UartFromCpuWr] = 0;
			if (c == 0x0D) {
				//trigger log
				TRACE(m_UartFromCpuBuf);
				m_UartFromCpuWr = 0;
			}
		}
		break;
		}
	}
	if (bd_Read == dir) {
		switch (addr) {
		case ADDR_UART_H: *data= uart_ch >> 4; break;
		case ADDR_UART_L: *data= uart_ch  & 0xf; break;
		default: processed = FALSE;	break;
		}
	}
	return processed;
}

inline BOOL CSimulator::OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data, SimData_t* mem) {
	SimAddress_t masked = (addr & 0xffc);
	if (ADDR_SCREEN_X == masked) {
		return OnScreenLoadStore(addr, dir, data);
	}else if (ADDR_UART_H == masked){
		return OnUartLoadStore(addr, dir, data);
	}
	else {
		BOOL processed = TRUE;
		if (bd_Read == dir) {
			switch (addr) {
			case ADDR_RND: *mem= *data = rand() & 0x0f; break;
			case ADDR_ARR: *data = m_Memory[ADDR_ARR]; *mem = 0; break;
			case ADDR_KEY0: *data = m_Memory[ADDR_KEY0];  *mem = 0; break;
			case ADDR_KEY1: *data = m_Memory[ADDR_KEY1];  *mem = 0; break;
			default:processed = FALSE;	break;
			}
			if (processed) return TRUE;
		}else if (bd_Write ==dir){
			// not writable
		}
	}
	return FALSE;
}
/**ProcessHeatMap:
Call periodically to get the heat map aged.
*/
void CSimulator::ProcessHeatMaps() {
	for (UINT i = 0; i < m_MemorySizeLoaded; i++) {
		if (m_HeatRead[i]) m_HeatRead[i]--;
		if (m_HeatWrite[i]) m_HeatWrite[i]--;
		if (m_HeatPc[i]) m_HeatPc[i]--;
	}
	for (int i = ADDR_ARR; i < ADDR_PERIPH_MAX; i++) {
		if (m_HeatRead[i]) m_HeatRead[i]--;
		if (m_HeatWrite[i]) m_HeatWrite[i]--;
		if (m_HeatPc[i]) m_HeatPc[i]--;
	}
}

/**ProcessMeasurement: cyclic function of the measurement of the cpu simulation
execution time.
*/
void CSimulator::ProcessMeasurement()
{
	if (m_ExecTimeActual < m_ExecTimeMin) m_ExecTimeMin = m_ExecTimeActual;
	if (m_ExecTimeActual > m_ExecTimeMax) m_ExecTimeMax = m_ExecTimeActual;
	m_ExecTimeSum += m_ExecTimeActual;
	m_ExecTimeSum -= m_ExecTimeAvg;
	m_ExecTimeAvg = (int)(m_ExecTimeSum >> 8);
}

/**ProcessDrawMeasurement: cyclic function of the measurement of the screen simulation
execution time.
*/
void CSimulator::ProcessDrawMeasurement()
{
	if (m_DrawTimeActual < m_DrawTimeMin) m_DrawTimeMin = m_DrawTimeActual;
	if (m_DrawTimeActual > m_DrawTimeMax) m_DrawTimeMax = m_DrawTimeActual;
	m_DrawTimeSum += m_DrawTimeActual;
	m_DrawTimeSum -= m_DrawTimeAvg;
	m_DrawTimeAvg = (int)( m_DrawTimeSum >> 8);
}

void CSimulator::ResetMeasurement() {
	m_ExecTimeMin = m_DrawTimeMin = 999999;
	m_ExecTimeMax = 0;
	m_ExecTimeAvg = m_ExecTimeActual;
	m_ExecTimeSum = m_ExecTimeAvg << 8;
	m_DrawTimeMax = 0;
	m_DrawTimeAvg = m_DrawTimeActual;
	m_DrawTimeSum = m_DrawTimeAvg << 8;
}

/**SearchLine: additional dbg file contains detailed informations, like
original asm file source line numbers, which related to the binary content.
This function iterates through the list, and return by the line number.
*/
SimAddress_t CSimulator::SearchLine(int line, int fileId)
{
	line += 1;
	if (line < 0) line = 2; //TODO: bugfix in asmb ??
	for (SimAddress_t i = 0; i < SIM_MAXMEMORYSIZE; i++) {
		if ((m_MemoryMeta[i].line == line)
			&& (m_MemoryMeta[i].fileId == fileId))
		{
			return i;
		}
	}
	return SimAddress_t(-1);
}

//TODO: obsolate or can be used somewhere?
BOOL CSimulator::GetDisAsm(SimAddress_t addr, CString & s)
{
	s = L"-";
	char op = m_Memory[addr];
	memoryType_t mt = MT_code;
	if (m_DbgFileLoaded) {
		mt = m_MemoryMeta[addr].sectionType;
	}
	if (MT_code == mt) {
		//m_DbgFileLoaded
		short operand = m_Memory[addr + 1];
		if (op) {
			operand |= m_Memory[addr + 2] << 4 | m_Memory[addr + 3] << 8;
			s.Format(_T("%03x %s 0x%x"), addr, gMnemonics[op], operand);
		}
		else {
			s.Format(_T("%03x %s0x%x"), addr, gMnemonics[op], operand);
		}
	}else
	if (MT_data == mt) {
		s.Format(_T("%03x DATA"), addr);
	}
	else return FALSE;
	return TRUE;
}

//DataBusRead: simulation of the cpu's data bus read.
inline SimData_t CSimulator::DataBusRead()
{
	if (!m_CpuSnapshot_p) return 0;
	if (m_CpuSnapshot_p->addr > SIM_MAXMEMORYSIZE) {
		m_CpuSnapshot_p->addr = SIM_MAXMEMORYSIZE; //Invalid address
	}
	m_HeatRead[m_CpuSnapshot_p->addr] = 0xff;
	if (m_CpuSnapshot_p->addr >= 0xc00) {
		SimData_t data = 0;
		if (OnPerifLoadStore(m_CpuSnapshot_p->addr, bd_Read, &data, &m_Memory[m_CpuSnapshot_p->addr])) {
			return data;
		}
	}
	return  m_Memory[m_CpuSnapshot_p->addr];
}

//DataBusDrive: simulation of the cpu's data bus write (drive the bus).
inline void CSimulator::DataBusDrive(SimData_t data)
{
	if (m_CpuSnapshot_p->addr > SIM_MAXMEMORYSIZE) {
		m_CpuSnapshot_p->addr = SIM_MAXMEMORYSIZE; //Invalid address
	}
	m_HeatWrite[m_CpuSnapshot_p->addr] = 0xff;
	if (m_CpuSnapshot_p->addr >= 0xc00) {
		if (OnPerifLoadStore(m_CpuSnapshot_p->addr, bd_Write, &data, &m_Memory[m_CpuSnapshot_p->addr])) {
			 return;
		}
	}
	
	m_Memory[m_CpuSnapshot_p->addr] = data;
}

//AluSetAccumulator: simulation of the cpu's ALU operation, when Accumulator register will be written.
inline void CSimulator::AluSetAccumulator(SimData_t data) {
	m_CpuSnapshot_p->acc = data; //can be wider than 4 bit
	m_CpuSnapshot_p->carry = (m_CpuSnapshot_p->acc >= 0x10) ? 1 : 0; //store carry
	m_CpuSnapshot_p->acc &= 0x0f; //remove carry
}
memoryMetaData_t*  CSimulator::GetMemoryMetaData(SimAddress_t a) {
	return &m_MemoryMeta[a];
}
int CSimulator::GetLine(SimAddress_t a)
{
	int l = m_MemoryMeta[a].line - 2;
	if (l < 0) l = 0;
	return l;
}

//BrakePC: brakes the cpu simulation, when PC reach the same location next time.
void CSimulator::BrakePC(BOOL isSet)
{
	SetBrakePC(m_Pc);
}

//SetBrakePC: toggles the PC brakepoint on the specific address.
BOOL CSimulator::SetBrakePC(SimAddress_t pc)
{
	if (!m_Break[pc] & 1) {
		m_Break[pc] |= 1;
	}
	else {
		m_Break[pc] &= ~1;
	}
	return m_Break[pc];
}

//IsBreakPC: is it a PC breakpoint active on the specific address ?
BOOL CSimulator::IsBreakPC(SimAddress_t pc) const
{
	return (m_Break[pc]) ? TRUE : FALSE;
}

//Step: Clock simulation step.
BOOL CSimulator::Step()
{
	BOOL ret = TRUE;
	if (!m_CpuSnapshot_p) return FALSE;
	if (m_Stop)	return FALSE;
	if (m_CpuHz <= 0) return FALSE;
	ProcessHeatMaps();
	m_ClockCount++;
	m_Time += 1.0 / (double)m_CpuHz;
	if (m_Pc < 0) {
		m_Pc = 0; //Invalid address in PC
	}
	if (m_Pc > SIM_MAXMEMORYSIZE) {
		m_Pc = 0; //Invalid address in PC
	}
	m_CpuSnapshot_p->cpuTick = m_ClockCount;
	m_CpuSnapshot_p->cpuTime = m_Time;
	m_CpuSnapshot_p->pc = m_Pc;
	
	switch (m_State) {
	case ss_Halt:
		ASSERT(0); //This is normally not a usecase, because the m_Stop should be true
		break;
	case ss_FetchOp:
		m_HeatPc[m_Pc] = 0xFf;
		m_CpuSnapshot_p->state = cs_Fetch;
		AddressBusDrive(m_Pc); // m_CpuSnapshot_p->addr = m_Pc;
		m_CpuSnapshot_p->op = DataBusRead();
		m_State = ss_FetchImm0;
		if (m_CpuSnapshot_p->op) {
			m_CpuSnapshot_p->ilen = 4;
		}
		else {
			m_CpuSnapshot_p->ilen = 2;
		}
		m_CpuSnapshot_p->pcnext = m_Pc + m_CpuSnapshot_p->ilen;
		break;
	case ss_FetchImm0:
		m_CpuSnapshot_p->state = cs_Fetch;
		AddressBusDrive(m_Pc+1);
		m_CpuSnapshot_p->immediate = DataBusRead();
		if (m_CpuSnapshot_p->op) {
			m_State = ss_FetchImm1;
		} else {
			m_State = ss_Decode;
		}
		break;
	case ss_FetchImm1:
		m_CpuSnapshot_p->state = cs_Fetch;
		AddressBusDrive(m_Pc + 2);
		m_CpuSnapshot_p->immediate = (m_CpuSnapshot_p->immediate ) | DataBusRead() << 4;
		m_State = ss_FetchImm2;
		break;
	case ss_FetchImm2:
		m_CpuSnapshot_p->state = cs_Fetch;
		AddressBusDrive(m_Pc + 3);
		m_CpuSnapshot_p->immediate = (m_CpuSnapshot_p->immediate ) | DataBusRead() << 8;
		m_State = ss_Decode;
		break;
	case ss_Decode:
		m_CpuSnapshot_p->state = cs_Decode;
		if ((m_CpuSnapshot_p->op > 1) && (m_CpuSnapshot_p->op< 9))
		{
			m_State = ss_Load;
		} else {
			m_State = ss_Alu;
		}
		break;
	case ss_Load:
		m_CpuSnapshot_p->state = cs_Load;
		AddressBusDrive(m_CpuSnapshot_p->immediate);
		m_CpuSnapshot_p->immediate = DataBusRead();
		m_State = ss_Alu;
		break;
	case ss_Alu:
		{
			m_CpuSnapshot_p->state = cs_Decode; //shouldn't change, we comes from decode
			SimData_t data= m_CpuSnapshot_p ->immediate & 0x0f;
			m_State = ss_FetchOp;
			switch (m_CpuSnapshot_p->op)
			{
				#undef TOK
				#define TOK(x, xop) case xop:
				TOK(T_mvi, 0) m_CpuSnapshot_p->acc = data & 0xf; break;//AluSetAccumulator( data ); break;
				TOK(T_sta, 1) m_State = ss_Store; break;
				TOK(T_lda, 2) AluSetAccumulator( data );  break;
				TOK(T_ad0, 3) AluSetAccumulator( m_CpuSnapshot_p->acc + data); break;
				TOK(T_ad1, 4) AluSetAccumulator( m_CpuSnapshot_p->acc + data + 1); break;
				TOK(T_adc, 5) AluSetAccumulator( m_CpuSnapshot_p->acc + data + m_CpuSnapshot_p->carry); break;
				TOK(T_nand, 6) AluSetAccumulator(~(m_CpuSnapshot_p->acc & data)); break;
				TOK(T_nor, 7) AluSetAccumulator(~(m_CpuSnapshot_p->acc | data));	break;
				TOK(T_rrm, 8) AluSetAccumulator(data >> 1); break;

				TOK(T_jmp, 9) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate;	break;
				TOK(T_jc, 10) if (m_CpuSnapshot_p->carry) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate;	break;
				TOK(T_jnc, 11) if (!m_CpuSnapshot_p->carry) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate; break;
				TOK(T_jz, 12)  if (!m_CpuSnapshot_p->acc) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate; break;
				TOK(T_jnz, 13) if (m_CpuSnapshot_p->acc) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate; break;
				TOK(T_jm, 14) if (m_CpuSnapshot_p->acc & 0x8) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate; break;
				TOK(T_jp, 15) if (!(m_CpuSnapshot_p->acc & 0x8)) m_CpuSnapshot_p->pcnext = m_CpuSnapshot_p->immediate; break;
			}
			if (ss_FetchOp == m_State) {
				m_Pc = m_CpuSnapshot_p->pcnext;
				ret= !(m_Break[m_Pc] & 1);
			}
		}
		break;
	case ss_Store:
		m_CpuSnapshot_p->state = cs_Store;
		AddressBusDrive(m_CpuSnapshot_p->immediate);
		DataBusDrive(m_CpuSnapshot_p->acc);
		m_State = ss_FetchOp;
		m_Pc = m_CpuSnapshot_p->pcnext;
		ret = !(m_Break[m_Pc] & 1);
		break;
	default:
		ASSERT(0);
		break;
	}
	return ret;
}

//RunQuick: not clock accurate, instruction simulation step. (vast faster)
BOOL CSimulator::RunQuick() {
	//if (!m_CpuSnapshot_p) m_CpuSnapshot_p = &m_CpuSnapshot;
	ASSERT(m_CpuSnapshot_p);
	m_ClockCount++;
	m_Time += 1 / (double)m_CpuHz;
	if (!(m_ClockCount%SIM_HEATMAP_PERIOD)) {
		ProcessHeatMaps();
	}
	if (m_Pc < 0) {
		m_Pc = 0; //Invalid address in PC
		return FALSE;
	}
	if (m_Pc >= SIM_MAXMEMORYSIZE) {
		m_Pc = 0; //Invalid address in PC
		return FALSE;
	}
	m_HeatPc[m_Pc] = 0xFf;
	SimData_t op = m_Memory[m_Pc];
	SimData_t data = m_Memory[m_Pc + 1];
	SimAddress_t imm = data | (m_Memory[m_Pc + 2]<<4)| (m_Memory[m_Pc + 3]<<8);
	SimData_t acc = 0;
	acc=m_CpuSnapshot_p->acc;
	m_Pc += (op) ? 4 : 2;
	if ((2 <= op)&&(op<=8))//lda..rrm
	{
		AddressBusDrive(imm);
		data = DataBusRead();
	}
	switch (op)
	{
#undef TOK
#define TOK(x, xop) case xop:
		TOK(T_mvi, 0) m_CpuSnapshot_p->acc = data; break;
		TOK(T_lda, 2) AluSetAccumulator(data);  break;
		TOK(T_ad0, 3) AluSetAccumulator(acc + data); break;
		TOK(T_ad1, 4) AluSetAccumulator(acc + data + 1); break;
		TOK(T_adc, 5) AluSetAccumulator(acc + data + m_CpuSnapshot_p->carry); break;
		TOK(T_nand, 6) AluSetAccumulator(~(acc & data)); break;
		TOK(T_nor, 7) AluSetAccumulator(~(acc | data));	break;
		TOK(T_rrm, 8) AluSetAccumulator(data >> 1); break;

		TOK(T_jmp, 9) m_Pc = imm;	break;
		TOK(T_jc, 10) if (m_CpuSnapshot_p->carry) m_Pc = imm;	break;
		TOK(T_jnc, 11) if (!m_CpuSnapshot_p->carry) m_Pc = imm; break;
		TOK(T_jz, 12)  if (!m_CpuSnapshot_p->acc) m_Pc = imm; break;
		TOK(T_jnz, 13) if (m_CpuSnapshot_p->acc) m_Pc = imm; break;
		TOK(T_jm, 14) if (m_CpuSnapshot_p->acc & 0x8) m_Pc = imm; break;
		TOK(T_jp, 15) if (!(m_CpuSnapshot_p->acc & 0x8)) m_Pc = imm; break;
	}
	if (1 == op)//sta
	{
		AddressBusDrive(imm);
		DataBusDrive(m_CpuSnapshot_p->acc);
	}
	if (m_Break[m_Pc]) return FALSE;
	return TRUE;
}

void CSimulator::ResetPeriph() {
	//these are needed beacuase of the simulation doesn't release the keyboard...
	m_Memory[ADDR_ARR] = 0;
	m_Memory[ADDR_KEY1] = 0;
	m_Memory[ADDR_KEY0] = 0;
	//screen maybe also requres a reset, but it is not documented how the reset
	//takes effect on the video card... So, we dont reset the screen now. 
	//We reset only the simulator...
}

//Reset: resets the cpu simulator
void CSimulator::Reset()
{
	m_Pc = 0;
	m_ClockCount = 0;
	m_Time = 0;
	m_Stop = m_StopAfterReset;
	m_State = m_Stop? ss_Halt: ss_FetchOp;
	ClearHeatMap();
	ResetMeasurement();
	ResetPeriph();
}

//ClearHeatMap: forget all of the heatmaps informations.
void CSimulator::ClearHeatMap()
{
	for (int i = 0; i < SIM_MAXMEMORYSIZE; i++) {
		m_HeatPc[i] = 0;
		m_HeatRead[i] = 0;
		m_HeatWrite[i] = 0;
	}
}

//TODO: maybe obsolate
void CSimulator::SetStop(BOOL isStop)
{
	m_Stop = isStop;
	if (!m_Stop) {
		if (ss_Halt == m_State) m_State = ss_FetchOp;
	}
	//TODO: halt vs stop
	// m_State = m_Stop ? ss_Halt : ss_FetchOp;
}
////////////////////////////////////////////////////
// Internal Representation file format
//todo: separate to a lib > internal representation

//tDbgFileBlockId enum. IDs for the Dbg file blocks.
typedef enum {
	DF_VERSION,
	DF_NAME,
	DF_FNAME_BIN,
	DF_TIME_BIN,
	DF_FNAME_ASM,
	DF_TIME_ASM,
	DF_SYM,
	DF_LINES,//obsolate
	DF_MEMTYPES, //obsolate
	DF_SECTIONNAME,
	DF_MEMORYMETA
} tDbgFileBlockId;


typedef enum {
	E_OK,
    E_NOT_OK
}Std_ReturnType;


/**dbgfile_rd: reads one block from the file.
Blocks are in common format. Each block starts with the block header, then content are follows.
Header:
from 0 to 4bytes long: id in little endian format, or first byte is the id and rest of the 3 bytes are reserved.
from 4 to 4bytes long: size of the payload.
from 8 to nbytes long: payload
When b buffer pointer is NULL, then payload will be not read by this function, that will be loaded consecutively
from the caller code.
*/
Std_ReturnType dbgfile_rd(CFile*f, tDbgFileBlockId& id, void* b, int& len) {
	unsigned int r=4;
	if (f->Read(&id, r) != r) return E_NOT_OK;
	if (f->Read(&len, r) != r) return E_NOT_OK;
	if (b) { //provided or pending 
		if (f->Read(b, len) != len) return E_NOT_OK;
	}
	return E_OK;
}

void CSimulator::ClearSymbolTable() {
	m_Symbols.RemoveAll();
}

void CSimulator::LoadSymbol(int len, CFile& f ) {
	tDbgFileSymbol sym;
	sym.memtype = MT_undef;
	sym.symtype = ST_Unknown;
	sym.sectionid = 0;
	sym.symcontexts = 0;
	sym.fileId = 0;
	sym.lineno = 0;
	sym.name[0] = '-';
	sym.name[1] = 0;
	sym.len = 1;
	if (len > sizeof(tDbgFileSymbol)) {
		//error
		f.Seek(len, CFile::current);
		return;
	}
	f.Read(&sym, len);
	if (sym.len >= MAXSYMBOLENAME) {
		//to long symbol name
		sym.len = MAXSYMBOLENAME - 1;
	}
	sym.name[sym.len] = 0;
	sym.lineno -= 2;//TODO: bugfix in asmb ??
	if (sym.lineno < 0)sym.lineno = 0;

	m_Symbols.Add(sym);
}
/**LoadBinToMemory
First step, It loads the .bin file to the operative memory of the simulator.
Then, it tries to open the .dbg file as well.
TODO: add more informations to the .dbg file, like: compiler warnings and additional symbole related things.
*/
#include "MainFrm.h"
#include "OutputWnd.h"
void CSimulator::LoadBinToMemory()
{
	if (!m_pDoc) return;
	m_Symbols.RemoveAll();
	m_Sections.RemoveAll();

	CString s;
	s.Format(L"%s%s", m_pDoc->m_AsmbDirOut, m_pDoc->m_SimTargetBinFileName);
	CFile f;
	CFileException e;
	TCHAR*  pszFileName  = s.GetBuffer();
	if (!f.Open(pszFileName,CFile::modeRead| CFile::typeBinary, &e)) // CFile::modeCreate | CFile::modeWrite, &e))
	{
		CString err;
		TRACE(_T("File could not be opened %d\n"), e.m_cause);
		COutputWnd* ow = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
		err.Format(L"File not found: %s", pszFileName);
		ow->FillBuildWindow(err);
		return;
	}

	m_MemorySizeLoaded = f.Read((void*)m_Memory, SIM_MAXMEMORYSIZE);
	CFileStatus status;
	if (f.GetStatus(status))
	{
		m_TimeBinFIle = status.m_mtime;
	}
	f.Close();
	
	Reset();
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			unsigned char code = 0xff;
			SimScreen.buf[x][y] = code;
		}
	}
	m_DbgFileLoaded = FALSE;
	int pos = s.ReverseFind('.');
	if (pos > 0) {
		s = s.Left(pos);
		s.Format(L"%s.dbg",s);
		if (!f.Open(s.GetBuffer(), CFile::modeRead | CFile::typeBinary, &e)) // CFile::modeCreate | CFile::modeWrite, &e))
		{
			TRACE(_T("File could not be opened %d\n"), e.m_cause);
		}
		if (f.GetStatus(status))
		{
			m_TimeDbgFIle = status.m_mtime;
		}

		int len = 0;
		int id = 0;
		int r = 4;
		CFileStatus fs;
		//fs.m_mtime
		//f.GetStatus(&fs);
		int DbgFormatVersion;
		BOOL process = TRUE;
		ClearSymbolTable();
		CStringArray* asmFileList = m_pDoc->GetAsmFileList();
		asmFileList->RemoveAll();
		while (process) {
			tDbgFileBlockId id;
			int len;
			if (E_OK == dbgfile_rd(&f, id, 0, len)) {
				switch (id) {
				case DF_VERSION:
					if (4 == len) {
						f.Read(&DbgFormatVersion, len);
					}
					else {
						process = FALSE;
					}
					if (1 != DbgFormatVersion) {
						process = FALSE;
					}
					break;
				case DF_MEMORYMETA:
					f.Read(m_MemoryMeta, len);
					break;
				case DF_SYM:
					LoadSymbol(len,f);
					break;
				case DF_SECTIONNAME:
					{
						char secName[MAXSYMBOLENAME];
						if (len > MAXSYMBOLENAME) {
							f.Seek(len, CFile::current); //error
						} else {
							CString s;
							f.Read(secName, len);
							secName[len] = 0;
							s.Format(L"%S", secName);
							m_Sections.Add(s);
						}
					}
					break;
				case DF_NAME:
					{
						char aName[MAXSYMBOLENAME];
						CString s;
						f.Read(aName, len);
						aName[len] = 0;
						s.Format(L"%S", aName);
						m_pDoc->SetProjectName(s);
					}
					break;
				case DF_FNAME_ASM:
					{
						char aName[MAXSYMBOLENAME];
						CString s;
						f.Read(aName, len);
						aName[len] = 0;
						s.Format(L"%S", aName);
						if (s.GetLength() > 4) {
							asmFileList->Add(s);
						}
					}
					break;
				case DF_FNAME_BIN:
					{
						char aName[MAXSYMBOLENAME];
						CString s;
						f.Read(aName, len);
						aName[len] = 0;
						s.Format(L"%S", aName);
						m_pDoc->SetTargetBinFileName(s);
					}
					break;
				case DF_MEMTYPES:	//obsolate
				case DF_LINES:		//obsolate
				case DF_TIME_BIN:	//not yet implemented
				case DF_TIME_ASM:	//not yet implemented
				default:
					f.Seek(len, CFile::current);
					break;
				}
			}
			else process = FALSE;
			
		}
		m_DbgFileLoaded = TRUE;
		// m_MemorySizeLoaded = f.Read((void*)m_Memory, SIM_MAXMEMORYSIZE);
		f.Close();

	}
}
