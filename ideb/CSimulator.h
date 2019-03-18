/** @file simulator.h
*
* @brief CSimulator model/controller interface
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#pragma once

//Forward declaration for used pointers.
class CidebDoc;

typedef int SimAddress_t;
typedef unsigned char SimData_t;

//Simulated memory size
#define SIM_MAXMEMORYSIZE (4096)

#define SIM_REFRESH_TIMER (70) //ms windows, screen refresh
#define SIM_TICK_PER_MS (1500/4)  //instruction per ms
#define SIM_HZ (SIM_TICK_PER_MS*1000)
#define SIM_STEPS_PER_TIMER (SIM_REFRESH_TIMER*SIM_TICK_PER_MS)//(4000*SIM_REFRESH_TIMER)
#define SIM_HEATMAP_PERIOD (2000) //todo: think through

#define MAXSYMBOLENAME 255

//Memory Type: a memory location can be code or data type.
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType_t;

typedef enum {
	ST_Unknown,
	ST_Label,
	ST_EQU,
	ST_DB,
	ST_JMP,
	ST_LOAD,
	ST_STORE
} SType_e;
typedef unsigned char SContexts_t; // bitmask : 1<<SType_e

//Bus direction type: an actual state of a bus can be read,write or tristate.
typedef enum {
	bd_Tristate,
	bd_Read,
	bd_Write,
	bd_max
} busDirection_t;

//Cpu state type: Fetch, Load, Decode, Store states of the Cpu.
typedef enum {
	cs_Fetch = 'F',
	cs_Load = 'L',
	cs_Store= 'S',
	cs_Decode= 'D'
} cpuState_t;

//Key arrow type: simulated arrow keys can be in state of the directions and fire, it is like a retro joystick.
typedef enum {
	ka_Nothing,
	ka_Left,
	ka_Right,
	ka_Up,
	ka_Down,
	ka_Fire=11, //todo: document arr periph, how it is works, meaning of the values, etc...
	ka_max
} keyArrow_t;

//Cpu simulator internal states: used internally, it describes every clock phase and states. Detailed than the cpuState_t.
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

//tDbgFileSymbol: Symbol block format in dbg file.
typedef struct {
	unsigned int value;
	unsigned int lineno;
	unsigned short fileId;
	unsigned char memtype;
	unsigned char sectionid;
	unsigned char symtype;
	unsigned char symcontexts;
	unsigned char len;
	unsigned char name[MAXSYMBOLENAME];
}tDbgFileSymbol;

//Memory Meta Data structure
typedef struct {
	short sectionId;		// 
	memoryType_t sectionType; // todo:check the size!!!
	short fileId;			// 0:base asm, 1..0xffff: includes
	int line;				// 32 bits
} memoryMetaData_t;

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
	void LoadSymbol(int len, CFile& f);
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
	CTime m_TimeBinFIle;
	CTime m_TimeDbgFIle;

	SimAddress_t m_Pc;
	SimData_t m_Memory[SIM_MAXMEMORYSIZE];

	SimData_t m_HeatPc[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatRead[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatWrite[SIM_MAXMEMORYSIZE];
	SimData_t m_Break[SIM_MAXMEMORYSIZE];
	memoryMetaData_t m_MemoryMeta[SIM_MAXMEMORYSIZE];
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
	CArray<tDbgFileSymbol> m_Symbols;
	CStringArray m_Sections;
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

