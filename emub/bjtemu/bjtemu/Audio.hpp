//
//  Audio.hpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#ifndef Audio_hpp
#define Audio_hpp
#include <SDL2/SDL_audio.h>

class AudioBuf_if{
public:
    virtual void close()=0;
    virtual bool isStarted()const=0;
    virtual void callback(Uint8 *stream, int& len)=0;

protected:
    Uint8 volume;
    SDL_AudioSpec wav_spec; // the specs of our piece of music
};


class AudioBuf_wav: public AudioBuf_if{
  //  static AudioBuf_wav* singleton;
public:
    AudioBuf_wav();
    virtual void close();
    virtual bool isStarted()const;
    virtual void callback(Uint8 *stream, int& len);

    static void s_audio_callback(void *userdata, Uint8 *stream, int len);
    int load(const char* fname, uint8_t avolume=SDL_MIX_MAXVOLUME);

protected:
    uint32_t audio_len;
    uint8_t* audio_pos;
    Uint32 wav_length; // length of our sample
    Uint8 *wav_buffer; // buffer containing our audio file
};

class AudioBuf_osc: public AudioBuf_if{
public:
    AudioBuf_osc();
    virtual void close();
    virtual bool isStarted()const;
    virtual void callback(Uint8 *stream, int& len);

    bool isFinished(){return started && (audio_pos>=audio_len);}
    void play();
    bool isPlaying()const {return (audio_pos>0) && (audio_pos<audio_len);}
    static void s_audio_callback(void *userdata, Uint8 *stream, int len);

protected:
    bool started;
    uint16_t audio_frq;
    uint32_t audio_len;
    uint32_t audio_pos;
    uint32_t adsr_pos[3];
    uint8_t adsr_vol[3];
};



#endif /* Audio_hpp */
