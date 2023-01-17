//
//  App.cpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#include "App.hpp"

#ifdef _MSC_VER
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#endif

#define DEFAULT_LOOP_PERIOD (20)
#define DEFAULT_CPU_SPEED (200000)
#define LETTER_LEFTOFFSET (0)
#define LETTER_TOPOFFSET (0)
#define LETTER_WIDTH (13)
#define LETTER_HEIGHT (16)
#define LETTER_BKG_KEY_R (0)
#define LETTER_BKG_KEY_G (0)
#define LETTER_BKG_KEY_B (0)
#define TEXTSCREEN_OFFSET_LEFT (45)
#define TEXTSCREEN_OFFSET_TOP (29)
#define TEXTLETTER_WIDTH (12)
#define TEXTLETTER_HEIGHT (12)
#define TEXTSCREEN_INITVALUE (0xffu)
#define WINDOWINITWIDTH (640)
#define WINDOWINITHEIGHT (480)

App* App::singleton=NULL;
App::App()
:cpuir(*this),quit(false), window(NULL), winSurface(NULL), image1(NULL), image2(NULL), scrNeedToRefresh(true), arrows(15)
{
    singleton=this;
    init();
}
App::~App(){
    kill();
}
int App::init(){
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"SDL initialization error: %s", SDL_GetError());
        //cout << "Error initializing SDL: " << SDL_GetError() << endl;
        return -1;
    }

    window = SDL_CreateWindow( "BjtCPU emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOWINITWIDTH, WINDOWINITHEIGHT,
        SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE );
    if ( !window ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"SDL window creation error: %s", SDL_GetError());
        //cout << "Error creating window: " << SDL_GetError()  << endl;
        return -2;
    }

    onWindowSizeChanged();
    for (int i=0; i<TEXTSCREEN_COLUMNS; i++){
        for (int j=0; j<TEXTSCREEN_ROWS; j++){
            scrbuf[i][j]=TEXTSCREEN_INITVALUE;
        }
    }
    return 0;
}
void App::onWindowSizeChanged(){
    if (winSurface) SDL_FreeSurface(winSurface);
    winSurface = SDL_GetWindowSurface( window );
    if ( !winSurface ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"SDL surface error: %s", SDL_GetError());
        //cout << "Error getting surface: " << SDL_GetError() << endl;
        quit=1;
    }else{
        scrNeedToRefresh=true;
    }
}
int App::load(const char * fname){
    // Temporary surfaces to load images into
    SDL_Surface *temp1, *temp2;

    // Load images
    temp1 = SDL_LoadBMP("scopebg.bmp");
    temp2 = SDL_LoadBMP("abc.bmp");
    SDL_SetColorKey( temp2 , SDL_TRUE, SDL_MapRGB( winSurface->format,
        LETTER_BKG_KEY_R, LETTER_BKG_KEY_G, LETTER_BKG_KEY_B) );

    // Make sure loads succeeded
    if ( !temp1 || !temp2 ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"SDL bitmap file error: %s", SDL_GetError());
        quit=1;
        return -1;
    }

    // Format surfaces
    image1 = SDL_ConvertSurface( temp1, winSurface->format, 0 );
    image2 = SDL_ConvertSurface( temp2, winSurface->format, 0 );

    // Free temporary surfaces
    SDL_FreeSurface( temp1 );
    SDL_FreeSurface( temp2 );

    // Make sure format succeeded
    if ( !image1 || !image2 ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"SDL ConvertSurface error: %s", SDL_GetError());
        quit=1;
        return -2;
    }
    filenameBin=fname;
    cpuir.load(fname);
    return 0;
}

void App::update(){
    
    if (!scrNeedToRefresh) return;
    scrNeedToRefresh=false;
    
    SDL_Rect ims, imr;
    ims.x=ims.y=0;
    SDL_GetWindowSize(window, &ims.w, &ims.h);
    SDL_GetClipRect(image1, &imr);
    
    // Blit image to entire window  SDL_BlitSurface
    int ret=SDL_BlitScaled( image1, &imr, winSurface, &ims );
    if (ret){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"app.update BLT error: %s", SDL_GetError());
        quit=1;
        return;
    }
    double mx= (double)ims.w/imr.w;
    double my= (double)ims.h/imr.h;
    int wx=LETTER_WIDTH;
    int wy=LETTER_HEIGHT;
    int ox=TEXTSCREEN_OFFSET_LEFT;
    int oy=TEXTSCREEN_OFFSET_TOP;
    
    // Blit image to scaled portion of window
    SDL_Rect src;
    src.w = wx;
    src.h = wy;
    SDL_Rect dest;
    dest.w = wx*mx;
    dest.h = wy*my;
    for (int px=0; px<TEXTSCREEN_COLUMNS; px++){
        for (int py=0; py<TEXTSCREEN_ROWS; py++){
            unsigned char code=scrbuf[px][py];
            if (TEXTSCREEN_INITVALUE == code) continue; //space or empty content
            int cx=code & 0x0Fu;
            int cy=code >> 4;
            src.x = wx*cx + LETTER_LEFTOFFSET;
            src.y = wy*cy + LETTER_TOPOFFSET;
            dest.x =(px*TEXTLETTER_WIDTH+ox)*mx;
            dest.y = (py*TEXTLETTER_HEIGHT+oy)*my;
            ret=SDL_BlitScaled( image2, &src, winSurface, &dest );
            if (ret){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"app.update Scale error: %s", SDL_GetError());
                quit=1;
                return;
            }
        }
    }

    // Update window
    if (SDL_UpdateWindowSurface( window )){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"app.update WindowSurface error: %s", SDL_GetError());
        quit=1;
    }
}

bool App::busLoad(uint16_t addr, uint8_t &adata){
    bool bError=false;
    uint8_t data= cpuir.read(addr); // memory implemented now in cpu ir class
    // system level peripheral access, like keyboard, joystick, rnd, etc
    switch (addr){
        case ADDR_RND: data= rand() & 0x0f;  break;
        case ADDR_KEY0:
        case ADDR_KEY1:cpuir.poke(addr,0); break;
        case ADDR_ARR: data=arrows; break;
        case ADDR_SCREEN_X:
        case ADDR_SCREEN_Y: break;
        default:
            // other peripherals like screen and sound is not implementing
            // a load bus cycle, so CPU can not access the content from
            // the screen memory in example. (Discuss w HW guy :D needed)
            if ((ADDR_ARR<= addr) &&(addr<=ADDR_ARR+0xff)){
                bError=true; //buserror
            }
            break;
    }
    adata=data;
    return bError;
}

bool App::busStore(uint16_t addr, uint8_t data){
    bool bError=false;
    switch (addr){
        case ADDR_BEEP:
        case ADDR_OUT1:
            break;
        default:
            cpuir.write(addr, data); // memory implemented now in cpu ir class
            break;
    };
    // system level peripheral access, like screen, sound dev, etc
    switch (addr){
        case ADDR_SCREEN_X:
        case ADDR_SCREEN_Y:
        case ADDR_SCREEN_CH1:
            break;
        case ADDR_SCREEN_CH0:{
            //change the screen memory content when ch0 store bus cycle happened.
            uint8_t x,y, ch0, ch1;
            x=cpuir.peek(ADDR_SCREEN_X);
            y=cpuir.peek(ADDR_SCREEN_Y);
            ch0=cpuir.peek(ADDR_SCREEN_CH0);
            ch1=cpuir.peek(ADDR_SCREEN_CH1);
            scrbuf[x][y]= ch0 | (ch1<<4);
            scrNeedToRefresh=true;
            }
            break;
        case ADDR_OUT2: break;
        case ADDR_OUT1:{
                SDL_Log("Out:%02x", (cpuir.peek(ADDR_OUT2)<<4) | data);
            }
            break;
        case ADDR_BEEP:
            audioBeep.play();
            break;
        default:
            // there are no other peripherals accessable to store bus cycle
            if ((ADDR_ARR<= addr) &&(addr<=ADDR_ARR+0xff)){
                bError=true; //buserror
            }
            break;
    }
    return bError;
}

void App::handleEvent(SDL_Event& event){
    switch( event.type ){
        case SDL_WINDOWEVENT:
            switch(event.window.event){
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "app.handleEvent Window Size changed.");
                    onWindowSizeChanged();
                    break;
            }
            break;
        /* Keyboard event */
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_DOWN: arrows&= ~(1 << ArrowBit_Down); break;
                case SDLK_LEFT: arrows&= ~(1 << ArrowBit_Left); break;
                case SDLK_UP: arrows&= ~(1 << ArrowBit_Up); break;
                case SDLK_RIGHT: arrows&= ~(1 << ArrowBit_Right); break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym){
                case SDLK_DOWN: arrows |= (1 << ArrowBit_Down); break;
                case SDLK_LEFT: arrows |= (1 << ArrowBit_Left); break;
                case SDLK_UP: arrows |= (1 << ArrowBit_Up); break;
                case SDLK_RIGHT: arrows |= (1 << ArrowBit_Right); break;
                case SDLK_ESCAPE: quit=1; break;
                case SDLK_KP_MULTIPLY: cpuir.halt=false; break;
                case SDLK_KP_DIVIDE: cpuir.halt=true; break;
                case SDLK_KP_ENTER: cpuir.dump();break;
                case SDLK_DELETE:
                    if (event.key.keysym.mod & KMOD_SHIFT){
                        cpuir.reset();
                    }else{
                        cpuir.load(filenameBin);
                    }
                    arrows=15;
                    break;
                
                case SDLK_SPACE:
                    cpuir.poke(ADDR_KEY0, 1);
                    cpuir.poke(ADDR_KEY1, 1);
                    break;
            }
            break;

        /* SDL_QUIT event (window close) */
        case SDL_QUIT:
            quit = 1;
            break;

        default:
            break;
    }
}

void App::run(){
    if (!image1) return;
    int periodLoopMs=DEFAULT_LOOP_PERIOD;
    int instructionPerLoop= DEFAULT_CPU_SPEED * DEFAULT_LOOP_PERIOD / 1000;
    SDL_Event event;
    bool needprocess=true;
    
    /* Loop until an SDL_QUIT event is found */
    while( !quit ){
        static Uint64 nextrun=0;
        Uint64 now= SDL_GetTicks64();
        if (now > nextrun){
            nextrun=now+periodLoopMs;
            needprocess=true;
            // SDL_Delay(periodLoopMs/2+1);
        }else{
            //adaptive wait.
            long long idle=nextrun-now;
            if (idle>DEFAULT_LOOP_PERIOD) idle=DEFAULT_LOOP_PERIOD;
            if (idle<1) idle=1;
            SDL_Delay((Uint32)idle);
        }
        if (audioBeep.isFinished()){
            audioBeep.close();
        }
        if (!cpuir.halt){
            if (needprocess){
                needprocess=false;
                for(int i=0; i< instructionPerLoop; i++){
                    cpuir.step();
                    if (cpuir.halt) break;
                }
                cpuir.statisticsElapseTime();
            }
        }
        update();
        
        /* Poll for events */
        while( SDL_PollEvent( &event ) ){
            handleEvent(event);
        }
    }
}
void App::kill(){
    if (image1) SDL_FreeSurface( image1 );
    if (image2) SDL_FreeSurface( image2 );

    if (audioBeep.isStarted()){
        audioBeep.close();
    }
    // Quit
    SDL_DestroyWindow( window );
    SDL_Quit();
}
void App::s_audio_callback( void *userdata, Uint8 *stream, int len){
    if (App::singleton){
        App::singleton->_audio_callback((AudioBuf_if*)userdata, stream, len);
    }
}
void App::_audio_callback(AudioBuf_if *data, Uint8 *stream, int len){
    if (!data) return;
    data->callback(stream, len);
    
}
