#include "stdafx.h"
#include "CSimulator.h"
#include "idebdoc.h"
static const TCHAR *gMnemonics[17] =
{ _T("mvi a,"),_T("sta"),_T("lda"),_T("ad0"),_T("ad1"),_T("adc"),_T("nand"),_T("nor"),_T("rrm"),_T("jmp"),_T("jc"),_T("jnc"),_T("jz"),_T("jnz"),_T("jm"),_T("jp"),_T("INVALID")};

static const TCHAR *gSimStates[9] =
{ _T("Halt"), _T("FetchOp"), _T("FetchImm0"), _T("FetchImm1"), _T("FetchImm2"), _T("Decode"), _T("Load"), _T("Alu"), _T("Store") };
enum {
	ADDR_UART_H = 0xc00,
	ADDR_UART_L,
	ADDR_SCREEN_X = 0xd00,
	ADDR_SCREEN_Y,
	ADDR_SCREEN_CH1,
	ADDR_SCREEN_CH0
};
typedef struct {
	char x, y;
	char ch0, ch1;
	unsigned char buf[16][16];
	TCHAR fakeChar;
} SimScreen_t;
SimScreen_t SimScreen;

TCHAR SimCode2Ascii[257] = _T(
	"0123456789abcdef" // 0
	"ghijklmnopqrstuv" // 1
	"wxyz!@#$%^&*()[]" // 2
	"-<>             " // 3
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
CSimulator::CSimulator()
	:m_pDoc(0), m_MemorySizeLoaded(0), m_Pc(0),m_CpuHz(1000000),m_Time(0), m_ClockCount(0), m_State(ss_Halt),
	m_UartFromCpuWr(0), m_CpuSnapshot_p(0), m_Stop(FALSE)
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
	}
	m_CpuSnapshot_p = &m_CpuSnapshot; //TODO: implement a debug timeline possibility later
	//m_Memory[0]=
	m_UartFromCpuBuf[0] = 0;
}

CSimulator::~CSimulator()
{
}

void CSimulator::SetDocument(CidebDoc * pDoc)
{
	m_pDoc = pDoc;
}

void CSimulator::SetPc(SimAddress_t pc)
{
	m_Pc = pc; //TODO: next pc or change state to fetch ?
}
void CSimulator::AddressBusDrive(SimAddress_t addr) {
	if (!m_CpuSnapshot_p) return;
	m_CpuSnapshot_p->addr = addr;
}

#define STARTX 16
#define STARTY 48
#define DISTANCEX 16
#define DISTANCEY 16
#define HEXDISTANCEX 9
#define HEXDISTANCEY 10

int CSimulator::OnDrawHexDump(CDC* pDC, SimAddress_t aBegin, SimAddress_t aEnd, int sy) {
	CRect r;
	int y = sy;
	TCHAR b[2];
	b[1] = 0;
	
	for (int i = aBegin; i < aEnd; i++) {
		int offs = i - aBegin;
		int x = offs & 0x3f;
		y = (offs >> 6)+sy;
		COLORREF cBk = RGB(m_HeatPc[i], 0, m_HeatWrite[i]);
		pDC->SetBkColor(cBk);
		if (m_HeatPc[i] > 16) {
			pDC->SetTextColor(RGB(255-m_HeatPc[i], 0x00, 0x00));
		}
		else {
			//unsigned char ct = m_HeatPc[i] > 128 ? 0x00 : 0xff;
			//unsigned char ct = 0xff;
			//pDC->SetTextColor(RGB(ct, ct, ct));
			pDC->SetTextColor(cBk ^ 0xffffff);
		}
		r.top = STARTY + y * HEXDISTANCEY;
		r.bottom = r.top + HEXDISTANCEY;
		r.left = 330 + x * HEXDISTANCEX;
		r.right = r.left + HEXDISTANCEX;
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
		pDC->SetBkColor(RGB(0xff, 0xff, 0xff));
		pDC->SetTextColor(COLORREF(0));
		if (m_Pc == i) {
			CPen penPc;
			penPc.CreatePen(PS_SOLID, 1, RGB(0xFF, 0x8F, 0));
			CPen* pOldPen = pDC->SelectObject(&penPc);
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH);
			pDC->SelectObject(&brush);
			pDC->Rectangle(&r);
			pDC->SelectObject(pOldPen);
		}
		if (m_HeatRead[i]) {
			CPen p(PS_SOLID, 2, RGB(0, m_HeatRead[i], 0));
			CPen* pOldPen = pDC->SelectObject(&p);
			pDC->MoveTo(r.left, r.bottom);
			pDC->LineTo(r.right, r.bottom);
			pDC->SelectObject(pOldPen);
		}
		if (!x) {
			CString s;
			r.left = 275;
			r.right = r.left + HEXDISTANCEX * 6;
			s.Format(_T("0x%03x:"), i);
			pDC->DrawTextW(s, &r, DT_TOP);
		}
	}
	return y;
}

void CSimulator::OnDraw(CDC* pDC) {
	CRect r;
	int sy = 0;
	// CSize s = pDC->GetViewportExt();
	r.right = 400;//s.cx;
	r.bottom = 400;// s.cy;
	pDC->SetTextColor(COLORREF(0));
	TCHAR b[2];
	b[1] = 0;
	for (int y = 0; y < 16; y++) {
		r.top = STARTY+y * DISTANCEY;
		r.bottom = r.top+ DISTANCEY;
		for (int x = 0; x < 16; x++) {
			unsigned char code= SimScreen.buf[x][y];
			r.left = STARTX+x * DISTANCEX;
			r.right = r.left + DISTANCEX;
			b[0] = SimCode2Ascii[code];
			pDC->DrawTextW(b, &r, DT_TOP);
		}
	}
	sy = OnDrawHexDump(pDC, 0, m_MemorySizeLoaded, sy)+1;
	sy = OnDrawHexDump(pDC, 0xc00, 0xc0f, sy) + 1;
	sy = OnDrawHexDump(pDC, 0xd00, 0xd0f, sy) + 1;
	
	r.top = 0;
	r.bottom = 15;
	r.left = 0;
	r.right = 700;
	if (m_CpuSnapshot_p) {
		CString s;
		char op = m_CpuSnapshot_p->op;
		char st = m_CpuSnapshot_p->state;
		short imm = m_CpuSnapshot_p->immediate;
		short adr = m_CpuSnapshot_p->addr;
		if (op > 15) op = 16; //invalid instruction
		if (st > 'S') st = '-';
		imm &= 0xfff;
		adr &= 0xfff;

		s.Format(_T("Tick: %06d PC: %03x Acc: %x +%d Imm: %03x Adr: %03x State: %c (%s) Op: %s "),
			m_ClockCount, m_Pc, m_CpuSnapshot_p->acc, m_CpuSnapshot_p->carry,  imm, adr,
			st, gSimStates[m_State], gMnemonics[op]);
		pDC->DrawTextW(s, &r, DT_TOP);
	}
}
BOOL CSimulator::OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
	BOOL processed = TRUE;
	if (bd_Write == dir) {
		switch (addr) {
		case ADDR_SCREEN_X: SimScreen.x = *data; break;
		case ADDR_SCREEN_Y: SimScreen.y = *data; break;
		case ADDR_SCREEN_CH0: SimScreen.ch0 = *data; break;
		case ADDR_SCREEN_CH1: SimScreen.ch1 = *data; break;
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
BOOL CSimulator::OnUartLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
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
BOOL CSimulator::OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
	SimAddress_t masked = (addr & 0xf00);
	if (ADDR_SCREEN_X == masked) {
		return OnScreenLoadStore(addr, dir, data);
	}else if (ADDR_UART_H == masked){
		return OnUartLoadStore(addr, dir, data);
	}
	return FALSE;
}
void CSimulator::ProcessHeatMaps() {
	for (int i = 0; i < SIM_MAXMEMORYSIZE; i++) {
		if (m_HeatRead[i]) m_HeatRead[i]--; //>>= 1; //--;
		if (m_HeatWrite[i]) m_HeatWrite[i]--;// >>= 1; // --;
		if (m_HeatPc[i]) m_HeatPc[i]--; //>>=1;
	}
}
SimData_t CSimulator::DataBusRead()
{
	if (!m_CpuSnapshot_p) return 0;
	if (m_CpuSnapshot_p->addr > SIM_MAXMEMORYSIZE) {
		m_CpuSnapshot_p->addr = SIM_MAXMEMORYSIZE; //Invalid address
	}
	m_HeatRead[m_CpuSnapshot_p->addr] = 0xff;
	if (m_CpuSnapshot_p->addr >= 0xc00) {
		SimData_t data = 0;
		if (OnPerifLoadStore(m_CpuSnapshot_p->addr, bd_Write, &data)) {
			m_Memory[m_CpuSnapshot_p->addr] = data;
			return data;
		}
	}
	return  m_Memory[m_CpuSnapshot_p->addr];
}
void CSimulator::DataBusDrive(SimData_t data)
{
	if (m_CpuSnapshot_p->addr > SIM_MAXMEMORYSIZE) {
		m_CpuSnapshot_p->addr = SIM_MAXMEMORYSIZE; //Invalid address
	}
	m_HeatWrite[m_CpuSnapshot_p->addr] = 0xff;
	if (m_CpuSnapshot_p->addr >= 0xc00) {
		if (OnPerifLoadStore(m_CpuSnapshot_p->addr, bd_Write, &data)) {
			m_Memory[m_CpuSnapshot_p->addr] = data;
			return;
		}
	}
	
	m_Memory[m_CpuSnapshot_p->addr] = data;
}
void CSimulator::AluSetAccumulator(SimData_t data) {
	m_CpuSnapshot_p->acc = data; //can be wider than 4 bit
	m_CpuSnapshot_p->carry = (m_CpuSnapshot_p->acc >= 0x10) ? 1 : 0; //store carry
	m_CpuSnapshot_p->acc &= 0x0f; //remove carry
}
void CSimulator::BrakePC(BOOL isSet)
{
	if (isSet) {
		m_Break[m_Pc] |= 1;
	}
	else {
		m_Break[m_Pc] &= ~1;
	}
}
BOOL CSimulator::Step()
{
	BOOL ret = TRUE;
	if (!m_CpuSnapshot_p) return FALSE;
	if (m_Stop)	return FALSE;
	if (m_CpuHz <= 0) return FALSE;
	ProcessHeatMaps();
	m_ClockCount++;
	m_Time += 1.0 / (double)m_CpuHz;
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
		m_CpuSnapshot_p->immediate = (m_CpuSnapshot_p->immediate << 4) | DataBusRead();
		m_State = ss_FetchImm2;
		break;
	case ss_FetchImm2:
		m_CpuSnapshot_p->state = cs_Fetch;
		AddressBusDrive(m_Pc + 3);
		m_CpuSnapshot_p->immediate = (m_CpuSnapshot_p->immediate << 4) | DataBusRead();
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
				TOK(T_mvi, 0) AluSetAccumulator( data ); break;
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

void CSimulator::Reset()
{
	m_Pc = 0;
	m_ClockCount = 0;
	m_Time = 0;
	m_Stop = m_StopAfterReset;
	m_State = m_Stop? ss_Halt: ss_FetchOp;
	ClearHeatMap();
}

void CSimulator::ClearHeatMap()
{
	for (int i = 0; i < SIM_MAXMEMORYSIZE; i++) {
		m_HeatPc[i] = 0;
		m_HeatRead[i] = 0;
		m_HeatWrite[i] = 0;
	}
}

void CSimulator::SetStop(BOOL isStop)
{
	m_Stop = isStop;
	if (!m_Stop) {
		if (ss_Halt == m_State) m_State = ss_FetchOp;
	}
	//TODO: halt vs stop
	// m_State = m_Stop ? ss_Halt : ss_FetchOp;
}

void CSimulator::LoadBinToMemory()
{
	if (!m_pDoc) return;
	//FILE* f=fopen(m_pDoc->m_SimTargetBinFileName, "r");
	CFile f;
	CFileException e;
	TCHAR* pszFileName = m_pDoc->m_SimTargetBinFileName.GetBuffer();
	if (!f.Open(pszFileName,CFile::modeRead| CFile::typeBinary, &e)) // CFile::modeCreate | CFile::modeWrite, &e))
	{
		TRACE(_T("File could not be opened %d\n"), e.m_cause);
	}
	m_MemorySizeLoaded = f.Read(m_Memory, SIM_MAXMEMORYSIZE);
	f.Close();
	Reset();
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			unsigned char code = 0xff;
			SimScreen.buf[x][y] = code;
		}
	}
}
