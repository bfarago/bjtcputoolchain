/** @file simulator.h
*
* @brief CSimulator model/controller interface
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#pragma once
#include "libbjtcpu.h"
//Forward declaration for used pointers.
class CidebDoc;

//Simulated memory size
#define SIM_MAXMEMORYSIZE (4096)
#define SIM_MAXTRACE (64*1024)
#define SIM_TRACESHIFTR (0)

#define SIM_REFRESH_TIMER (70) //ms windows, screen refresh
#define SIM_TICK_PER_MS (1500/4)  //instruction per ms
#define SIM_HZ (SIM_TICK_PER_MS*1000)
#define SIM_STEPS_PER_TIMER (SIM_REFRESH_TIMER*SIM_TICK_PER_MS)//(4000*SIM_REFRESH_TIMER)
#define SIM_HEATMAP_PERIOD (2000) //todo: think through

//Cpu Snapshot type represents all of the internal states. It will be good to simulate a digital analyser at the future.
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

//Run Mode type: simulator can be in clock step mode, instruction step mode, and run mode.
typedef enum {
	rm_StepState,
	rm_StepInstruction,
	rm_Run
} runMode_t;


typedef struct {
	unsigned int timestamp;
	unsigned short pc;
}tTraceJump;

typedef struct {
	unsigned int address;
	int index;
}tTraceLabel;

//CSimulator: implements the cpu and screen simulator.
class CSimulator
{
public:
	CSimulator();
	~CSimulator();
	void SetDocument(CidebDoc* pDoc);
	void SetPc(SimAddress_t pc);
	void SetRunMode(runMode_t rm) { m_RunMode = rm; }
	SimAddress_t GetPc()const { return m_Pc; }
	int GetLine(SimAddress_t a);
	void BrakePC(BOOL isSet);
	BOOL SetBrakePC(SimAddress_t pc);
	BOOL IsBreakPC(SimAddress_t pc)const;
	BOOL Step();
	BOOL RunQuick();
	void ResetPeriph();
	void Reset();
	void ClearHeatMap();
	void SetStop(BOOL isStop);
	BOOL GetStop()const { return m_Stop; }
	void ClearSymbolTable();
	int GetLabelForPc(int pc);
	void LoadSymbol(int len, CFile& f);
	void LoadError(int len, CFile & f);
	void LoadBinToMemory();
	void OnDraw(CDC * pDC, int mode);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void ProcessHeatMaps();
	void ProcessMeasurement();
	void ProcessDrawMeasurement();
	void ResetMeasurement();
	BOOL IsDebugFileLoaded()const { return m_DbgFileLoaded; }
	SimAddress_t SearchLine(int line, int fileId=0);
	BOOL GetDisAsm(SimAddress_t addr ,CString& s);
	SimAddress_t OnDrawDisasm(CDC* pDC, CRect & r, SimAddress_t a);
	memoryMetaData_t * GetMemoryMetaData(SimAddress_t a);
private:
	void AddressBusDrive(SimAddress_t addr);
	BOOL OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnUartLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data, SimData_t* mem);

	
	void AluSetAccumulator(SimData_t data);
	inline bool isPeriphAccess();
	inline SimData_t DataBusRead();
	inline void DataBusDrive(SimData_t data);
	int OnDrawHexDump(CDC * pDC, SimAddress_t aBegin, SimAddress_t aEnd, int sx, int sy);
	
	HICON hIconBreak;
	CFont m_FontMonospace;
	CDC m_DCTmp;
	CBitmap m_BitmapTmp;

protected:
	CidebDoc * m_pDoc;
	CTime m_TimeBinFIle;
	CTime m_TimeDbgFIle;

	SimAddress_t m_Pc;
	SimData_t m_Memory[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatPc[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatRead[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatWrite[SIM_MAXMEMORYSIZE];
	SimData_t m_Break[SIM_MAXMEMORYSIZE];
	memoryMetaData_t m_MemoryMeta[SIM_MAXMEMORYSIZE];
	unsigned int m_TracePcPos;
	tTraceJump m_TracePc[SIM_MAXTRACE];
	unsigned int m_TracePcYNum;
	int m_TracePcY[SIM_MAXTRACE];
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
	BOOL m_DisplayScreen;
	BOOL m_DisplayMeasurement;
	BOOL m_DisplayMemory;
	BOOL m_DisplayDebugMonitor;
	BOOL m_DisplayTimeLine;
	CArray<tDbgFileSymbol> m_Symbols;
	CArray<tErrorRecord> m_Errors;
	CStringArray m_Sections;
	CArray<tTraceLabel> m_TraceLabels;
	int m_TraceShiftR;
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

