//
//  CpuIr.hpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#ifndef CpuIr_hpp
#define CpuIr_hpp
//only for basic types.
#include <SDL2/SDL.h>

#define MAXMEMORYSIZE (1<<12)
enum {
    ADDR_ARR = 0xc00,
    ADDR_RND,
    ADDR_KEY0,
    ADDR_KEY1,
    ADDR_SCREEN_X,    //mask:0xc04
    ADDR_SCREEN_Y,
    ADDR_SCREEN_CH1, //High
    ADDR_SCREEN_CH0, //Low
    ADDR_UART_H,   // mask:0xc08
    ADDR_UART_L,
    ADDR_BEEP,
    ADDR_PERIPH_MAX
};

//Key arrow type: simulated arrow keys can be in state of the directions and fire, it is like a retro joystick.
typedef enum {
    ka_Nothing,
    ka_Left,
    ka_Right,
    ka_Up,
    ka_Down,
    ka_Fire = 11, //todo: document arr periph, how it is works, meaning of the values, etc...
    ka_max
} keyArrow_t;

typedef struct {
    uint8_t brakeOnExecute:1;
    uint8_t brakeOnWrite:1;
    uint8_t brakeOnRead:1;
} traceModeFlags_s;

typedef union{
    uint8_t d;
    traceModeFlags_s f;
} traceMode_t;

class Periph_if{
public:
    virtual bool busLoad(uint16_t addr, uint8_t &data)=0;
    virtual bool busStore(uint16_t addr, uint8_t data)=0;
};

class CpuIr{
public:
    CpuIr(Periph_if& abus);
    void load(const char* fname);
    void reset();
    void step();

    bool halt;

    void traceReset();
    void statisticsReset();
    void statisticsElapseTime();
    
    //memory implemented here, but later we can move outside, acces by periph.
    uint8_t read(uint16_t addr);            // memory access
    void write(uint16_t addr, uint8_t data);// memory access
    //memory access on different clock? no statistics, used to periph internal memory store in operative memory, like keypress, etc.
    uint8_t peek(uint16_t addr);            // memory direct access
    void poke(uint16_t addr, uint8_t data);// memory direct access
    
    void dump();
    void stop(); //debug break
protected:
    Periph_if & bus; // cpu access to bus to reach peripherals and ram normally
    unsigned short pc;
    unsigned short acc;
    unsigned char carry;
    unsigned char mem[MAXMEMORYSIZE];
    traceMode_t tracemode[MAXMEMORYSIZE];
    uint64_t wasread[MAXMEMORYSIZE>>6];
    uint64_t waswritten[MAXMEMORYSIZE>>6];
    uint64_t wasexecuted[MAXMEMORYSIZE>>6];
    uint8_t countread[MAXMEMORYSIZE];
    uint8_t countwritten[MAXMEMORYSIZE];
    uint8_t countexecuted[MAXMEMORYSIZE];
    uint8_t fetchop();
    void setPC(uint16_t newpc);
    void setACC(uint8_t data);
    void buserror(uint16_t addr, uint8_t data, bool store);
};


#endif /* CpuIr_hpp */
