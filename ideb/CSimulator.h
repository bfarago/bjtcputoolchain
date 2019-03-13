#pragma once
//#include "idebDoc.h"
class CidebDoc;

typedef int SimAddress_t;
typedef unsigned char SimData_t;

#define SIM_MAXMEMORYSIZE (4096)

#define SIM_REFRESH_TIMER (70) //ms windows, screen refresh
#define SIM_TICK_PER_MS (1500/4)  //instruction per ms
#define SIM_HZ (SIM_TICK_PER_MS*1000)
#define SIM_STEPS_PER_TIMER (SIM_REFRESH_TIMER*SIM_TICK_PER_MS)//(4000*SIM_REFRESH_TIMER)
#define SIM_HEATMAP_PERIOD (2000) //todo: think through
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType_t;

typedef enum {
	bd_Tristate,
	bd_Read,
	bd_Write,
	bd_max
} busDirection_t;

typedef enum {
	cs_Fetch = 'F',
	cs_Load = 'L',
	cs_Store= 'S',
	cs_Decode= 'D'
} cpuState_t;

typedef enum {
	ka_Nothing,
	ka_Left,
	ka_Right,
	ka_Up,
	ka_Down,
	ka_Fire=11, //todo: document arr periph, how it is works, meaning of the values, etc...
	ka_max
} keyArrow_t;

typedef enum {
	ss_Halt=0,
	ss_FetchOp,		//Fetch Operation/Instruction from actual PC
	ss_FetchImm0,	//Fetch Immediate from actual PC+n
	ss_FetchImm1,	//Fetch Immediate from actual PC+n
	ss_FetchImm2,	//Fetch Immediate from actual PC+n
	ss_Decode,		//Setup internal states
	ss_Load,		//Load Acc from memory pointed by Address register
	ss_Alu,			//Do the math.
	ss_Store,		//Store Acc to memory pointed by Address register
} SimState_t;

/*
typedef struct {
	int address;
	char data;
	busDirection_t dir;
} bus_t;
*/
typedef struct {
	DWORD cpuTick;
	double cpuTime;
	SimAddress_t pc;
	SimAddress_t pcnext;
	SimAddress_t addr;
	SimAddress_t immediate; //can hold data and address
	SimData_t    acc;
	char carry;
	char op;
	cpuState_t state;
	
	char ilen;
} cpu_SnapShot_t;

typedef enum {
	rm_StepState,
	rm_StepInstruction,
	rm_Run
} runMode_t;

class CSimulator
{
public:
	CSimulator();
	~CSimulator();
	void SetDocument(CidebDoc* pDoc);
	void SetPc(SimAddress_t pc);
	void SetRunMode(runMode_t rm) { m_RunMode = rm; }
	SimAddress_t GetPc()const { return m_Pc; }
	void BrakePC(BOOL isSet);
	BOOL SetBrakePC(SimAddress_t pc);
	BOOL IsBreakPC(SimAddress_t pc)const;
	BOOL Step();
	BOOL RunQuick();
	void Reset();
	void ClearHeatMap();
	void SetStop(BOOL isStop);
	BOOL GetStop()const { return m_Stop; }
	void LoadBinToMemory();
	void OnDraw(CDC * pDC, int mode);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void ProcessHeatMaps();
	void ProcessMeasurement();
	void ProcessDrawMeasurement();
	void ResetMeasurement();
	BOOL IsDebugFileLoaded()const { return m_DbgFileLoaded; }
	SimAddress_t SearchLine(int line);
	BOOL GetDisAsm(SimAddress_t addr ,CString& s);
	SimAddress_t OnDrawDisasm(CDC* pDC, CRect & r, SimAddress_t a);
private:
	void AddressBusDrive(SimAddress_t addr);
	BOOL OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnUartLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data, SimData_t* mem);

	void DataBusDrive(SimData_t data);
	void AluSetAccumulator(SimData_t data);
	SimData_t DataBusRead();
	int OnDrawHexDump(CDC * pDC, SimAddress_t aBegin, SimAddress_t aEnd, int sx, int sy);
	
	HICON hIconBreak;
	CFont m_FontMonospace;
	CDC m_DCTmp;
	CBitmap m_BitmapTmp;
protected:
	CidebDoc * m_pDoc;
	SimAddress_t m_Pc;
	SimData_t m_Memory[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatPc[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatRead[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatWrite[SIM_MAXMEMORYSIZE];
	SimData_t m_Break[SIM_MAXMEMORYSIZE];
	memoryType_t m_MemoryType[SIM_MAXMEMORYSIZE];
	int m_MemoryLine[SIM_MAXMEMORYSIZE];

	UINT m_MemorySizeLoaded;
	DWORD m_CpuHz;
	DWORD m_ClockCount;
	double m_Time;
	SimState_t m_State;
	BOOL m_Stop;
	BOOL m_StopAfterReset;
	cpu_SnapShot_t* m_CpuSnapshot_p;
	cpu_SnapShot_t m_CpuSnapshot;
	TCHAR m_UartFromCpuBuf[512];
	int m_UartFromCpuWr;
	runMode_t m_RunMode;
	CBitmap m_bitmapScopeBg;
	CBitmap m_bitmapAbc;
public: //temporary
	int m_ExecTimeActual;
	BOOL m_DisplayMeasurement;
	BOOL m_DisplayMemory;
	BOOL m_DisplayDebugMonitor;
protected:
	ULONG64 m_ExecTimeSum;
	int m_ExecTimeAvg;
	int m_ExecTimeMax;
	int m_ExecTimeMin;
	int m_DrawTimeActual;
	ULONG64 m_DrawTimeSum;
	int m_DrawTimeAvg;
	int m_DrawTimeMax;
	int m_DrawTimeMin;
	BOOL m_DbgFileLoaded;
};

