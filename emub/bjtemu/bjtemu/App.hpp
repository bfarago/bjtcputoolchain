//
//  App.hpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#ifndef App_hpp
#define App_hpp

#include <SDL2/SDL.h>
//#include <SDL2/SDL_ttf.h>

#include "Audio.hpp"
#include "CpuIr.hpp"

#define TEXTSCREEN_COLUMNS (16)
#define TEXTSCREEN_ROWS (16)

class App :public Periph_if
{
public:
    App();
    ~App();
    int load(const char *fname);
    void run();
    static void s_audio_callback(void *userdata, Uint8 *stream, int len);
    virtual bool busLoad(uint16_t addr, uint8_t &data);
    virtual bool busStore(uint16_t addr, uint8_t data);
private:
    int init();
    void kill();
    void update();
    void handleEvent(SDL_Event& event);
    void onWindowSizeChanged();
    
protected:
    static App* singleton;
    int quit;
    // Pointers to our window and surfaces
    SDL_Window* window;
    SDL_Surface* winSurface;
    SDL_Surface* image1;
    SDL_Surface* image2;
    unsigned char scrbuf[TEXTSCREEN_COLUMNS][TEXTSCREEN_ROWS];
    bool scrNeedToRefresh;
    CpuIr cpuir;
    void _audio_callback(AudioBuf_if *userdata, Uint8 *stream, int len);
    const char* filenameBin;
    AudioBuf_osc audioBeep;
    uint8_t arrows;
};

#endif /* App_hpp */
