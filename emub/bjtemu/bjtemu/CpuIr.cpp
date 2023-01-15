//
//  CpuIr.cpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#include "CpuIr.hpp"
#include <SDL2/SDL_log.h>

#ifdef _MSC_VER
#include <stdio.h>
#endif

CpuIr::CpuIr(Periph_if& abus)
:bus(abus)
{
    reset();
    traceReset();
}
void CpuIr::reset(){
    pc=0;
    statisticsReset();
}
void CpuIr::traceReset(){
    for(int i=0; i<MAXMEMORYSIZE;i++){
        tracemode[i].d=0;
    }
}
void CpuIr::statisticsReset(){
    for (int i=0; i<(MAXMEMORYSIZE>>6); i++){
        wasread[i]=0;
        waswritten[i]=0;
        wasexecuted[i]=0;
    }
    for (int i=0; i<MAXMEMORYSIZE; i++){
        countread[i]=0;
        countwritten[i]=0;
        countexecuted[i]=0;
    }
}
void CpuIr::statisticsElapseTime(){
    for (int i=0; i<MAXMEMORYSIZE; i++){
        if (countread[i]) countread[i]--;
        if (countwritten[i]) countwritten[i]--;
        if (countexecuted[i]) countexecuted[i]--;
    }
}
void CpuIr::load(const char *fname){
    if (!fname) return;
#ifdef _MSC_VER
    FILE* f=NULL;
    fopen_s(&f, fname, "rb");
#else
    FILE* f = fopen(fname, "rb");
#endif
    if (!f){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "File open error:%s", fname);
    }else{
        fread(mem, MAXMEMORYSIZE, 1, f);
        fclose(f);
    }
    reset();
    halt=false;
}
void CpuIr::step(){
    // Fetch
    uint8_t op=fetchop(); //load+statistics
    uint8_t iwidth=(op)?4:2;
    uint8_t data= mem[pc+1] & 0xf; //fetching actually without stat
    uint16_t imm= data | (mem[pc+2]<<4) | (mem[pc+3]<<8); //immediate load, no stat

    // Load
    pc+= iwidth; // can be later
    if ((2 <= op) && (op <= 8))//lda..rrm
    {
        if (bus.busLoad(imm, data)){ //data = sim_read(imm);
            buserror(imm, data, false);
            data=0xf; // Question: what the cpu read when invalid address?
        }
    }

    // Execute
    switch (op)
    {
#undef TOK
#define TOK(x, xop) case xop:
#define Sim_Acc acc                     // direct rw access to acc
#define Sim_SetPc(X) setPC(X)           // jump or change the pc w statistics
#define Sim_SetACC(X) setACC( (X) )     // cpu change the acc, and flags
#define Sim_Carry (carry)               // carry
        TOK(T_mvi,  0) Sim_Acc = data; break; // does not change the carry
        TOK(T_lda,  2) Sim_SetACC(data);  break;
        TOK(T_ad0,  3) Sim_SetACC(acc + data); break;
        TOK(T_ad1,  4) Sim_SetACC(acc + data + 1); break;
        TOK(T_adc,  5) Sim_SetACC(acc + data + Sim_Carry); break;
        TOK(T_nand, 6) Sim_SetACC(~(acc & data)); break;
        TOK(T_nor,  7) Sim_SetACC(~(acc | data));    break;
        TOK(T_rrm,  8) Sim_SetACC(data >> 1); break;
        TOK(T_jmp,  9) Sim_SetPc(imm);    break;
        TOK(T_jc,  10) if (Sim_Carry) Sim_SetPc(imm);    break;
        TOK(T_jnc, 11) if (!Sim_Carry) Sim_SetPc(imm); break;
        TOK(T_jz,  12) if (!Sim_Acc) Sim_SetPc(imm); break;
        TOK(T_jnz, 13) if (Sim_Acc) Sim_SetPc(imm); break;
        TOK(T_jm,  14) if (Sim_Acc & 0x8) Sim_SetPc(imm); break;
        TOK(T_jp,  15) if (!(Sim_Acc & 0x8)) Sim_SetPc(imm); break;
        default: break;
    }

    // Store
    switch (op) {
        TOK(T_sta, 1) {
            uint8_t a8 = (uint8_t)acc;
            if (bus.busStore(imm, a8)){
                buserror(imm, a8, true);
            }
        }
        break;
        default: break;
    }
}

uint8_t CpuIr::peek(uint16_t addr){
    if (addr>=MAXMEMORYSIZE){
        buserror(addr, 0xf, false);
        return 0x0f;
    }
    return mem[addr];
}
void CpuIr::poke(uint16_t addr, uint8_t data){
    if (addr>=MAXMEMORYSIZE){
        buserror(addr, 0xf, true);
        return;
    }
    mem[addr]=data;
}

uint8_t CpuIr::read(uint16_t addr){
    wasread[addr>>6]|=1ULL<<(addr&63);
    if (tracemode[addr].f.brakeOnRead) stop();
    return peek(addr);
}

void CpuIr::write(uint16_t addr, uint8_t data){
    //memory direct acccess. In case of separation,
    //there should be direct and bus access for memory.
    //Question: HW, display subsystem access the operative mem how and when ?
    poke(addr,data);
    if (tracemode[addr].f.brakeOnWrite) stop();
    waswritten[addr>>6]|=1ULL<<(addr&63);
}
void CpuIr::buserror(uint16_t addr, uint8_t data, bool store){
    if (1){
        SDL_Log("buserror  addr:%03x data:%x %s", addr,data,store?"store":"load");
        stop();
    }
}
void CpuIr::stop(){
    halt=true;
    dump();
}
void CpuIr::dump(){
    SDL_Log("PC:%03x ACC:%x FLAGS:%c%c%s EMU:%c\n", pc, acc, carry?'C':' ',acc&8?'M':' ', acc?"N ":" Z", halt?'H':'R' );
}
uint8_t CpuIr::fetchop(){
    if (pc >= MAXMEMORYSIZE) pc=0;
    wasexecuted[pc>>6]|=1ULL<<(pc&63);
    if (tracemode[pc].f.brakeOnExecute) stop();
    return mem[pc] & 0xf;
}
void CpuIr::setPC(uint16_t newpc){
    pc=newpc;
}
void CpuIr::setACC(uint8_t data){
    acc= data & 15; //without overflow and carry
    carry= (data >= 16) ? 1 : 0; //calculate and store carry
}
