/*
*/

#ifdef _WIN32
//#include "stdafx.h"
#endif

#include "libbjtcpu_cfg.h"
#include "libbjtcpu.h"

//textual representation of each instructions.
const TCHAR *gMnemonics[17] =
{ _T("mvi a,"),_T("sta"),_T("lda"),_T("ad0"),_T("ad1"),_T("adc"),_T("nand"),_T("nor"),_T("rrm"),
_T("jmp"),_T("jc "),_T("jnc"),_T("jz "),_T("jnz"),_T("jm "),_T("jp "),_T("INVALID") };

//textual representation of each internal states of the clock accurate simulator
const TCHAR *gSimStates[9] =
{ _T("Halt     "), _T("FetchOp  "), _T("FetchImm0"), _T("FetchImm1"), _T("FetchImm2"),
_T("Decode   "), _T("Load     "), _T("Alu      "), _T("Store    ") };