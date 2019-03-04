#pragma once
//#include "idebDoc.h"
class CidebDoc;

typedef int SimAddress_t;
typedef unsigned char SimData_t;

#define SIM_MAXMEMORYSIZE (4096)

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
/*
bus_t bus;
cpu_t cpu;
*/

class CSimulator
{
public:
	CSimulator();
	~CSimulator();
	void SetDocument(CidebDoc* pDoc);
	void SetPc(SimAddress_t pc);

	SimAddress_t GetPc()const { return m_Pc; }
	void BrakePC(BOOL isSet);
	BOOL Step();
	void Reset();
	void ClearHeatMap();
	void SetStop(BOOL isStop);
	BOOL GetStop()const { return m_Stop; }
	void LoadBinToMemory();
	void OnDraw(CDC * pDC);
private:
	void AddressBusDrive(SimAddress_t addr);
	BOOL OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnUartLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	BOOL OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t * data);
	void ProcessHeatMaps();
	void DataBusDrive(SimData_t data);
	void AluSetAccumulator(SimData_t data);
	SimData_t DataBusRead();
	int OnDrawHexDump(CDC * pDC, SimAddress_t aBegin, SimAddress_t aEnd, int sy);
protected:
	CidebDoc * m_pDoc;
	SimAddress_t m_Pc;
	SimData_t m_Memory[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatPc[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatRead[SIM_MAXMEMORYSIZE];
	SimData_t m_HeatWrite[SIM_MAXMEMORYSIZE];
	SimData_t m_Break[SIM_MAXMEMORYSIZE];
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
};

