//
//  Audio.cpp
//  bjtemu
//
//  Created by Farago Barna on 2023. 01. 15..
//

#include "Audio.hpp"
#include <SDL2/SDL_log.h>
#define DEFAULT_OSC_SAMPLERATE (44100)

AudioBuf_wav::AudioBuf_wav(){}
void AudioBuf_wav::close(){
    // shut everything down
    SDL_CloseAudio();
    SDL_FreeWAV(wav_buffer);
}
bool AudioBuf_wav::isStarted() const {return audio_len>0;}
void AudioBuf_wav::s_audio_callback(void *userdata, Uint8 *stream, int len){
    AudioBuf_wav* obj=(AudioBuf_wav*)userdata;
    obj->callback(stream, len);
}
int AudioBuf_wav::load(const char* fname, uint8_t avolume){
    /* Load the WAV */
    // the specs, length and buffer of our wav are filled
    if( SDL_LoadWAV(fname, &wav_spec, &wav_buffer, &wav_length) == NULL ){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Audio file open error: %s", fname);
        return -2;
    }
    // set the callback function
    wav_spec.callback = AudioBuf_wav::s_audio_callback;
    wav_spec.userdata = (void*)this;
    audio_pos = wav_buffer; // copy sound buffer
    audio_len = wav_length; // copy file length
    volume=avolume;
    
    /* Open the audio device */
    if ( SDL_OpenAudio(&wav_spec, NULL) < 0 ){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Audio device open error: %s", SDL_GetError());
        //fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return -1;
    }
    
    /* Start playing */
    SDL_PauseAudio(0);
    
    // wait until we're don't playing
    //while ( audio_len > 0 ) {
       // SDL_Delay(100);
    //}
    return 0;
}


void AudioBuf_wav::callback(Uint8 *stream, int& len){
    len = ( len > audio_len ? audio_len : len );
    //SDL_memcpy (stream, audio_pos, len);                     // simply copy from one buffer into the other
    SDL_MixAudio(stream, audio_pos, len, volume);// mix from one buffer into another
    audio_pos += len;
    audio_len -= len;
}


AudioBuf_osc::AudioBuf_osc()
:started(false),audio_frq(1500),audio_len(2000),audio_pos(0)
{
    adsr_pos[0]= 10; adsr_vol[0]=127; // attack 0..10
    adsr_pos[1]= 300; adsr_vol[1]=80; // decay
    adsr_pos[2]=1000; adsr_vol[2]=30; // sustain, and release until the end.
    volume=127; // gain if you like
}
void AudioBuf_osc::close(){
    SDL_CloseAudio();
    started=false;
    audio_pos=0;
}
void AudioBuf_osc::play(){
    if (started) close();
    wav_spec.format=AUDIO_S16;
    wav_spec.freq=DEFAULT_OSC_SAMPLERATE;
    wav_spec.samples=audio_len;
    wav_spec.userdata=(void*)this;
    wav_spec.callback=AudioBuf_osc::s_audio_callback;
    wav_spec.channels=2;
    wav_spec.silence=0;
    audio_pos=0;
    started=true;
    if ( SDL_OpenAudio(&wav_spec, NULL)<0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Audio.play error: %s\n", SDL_GetError());
        started=false;
    }else{
        SDL_PauseAudio(0);
    }
}
void AudioBuf_osc::callback(Uint8 *stream, int& len){
    if (!started) return;
    int samples=len>>2;
    samples = ( samples > audio_len ? audio_len : samples );
    int actual_pos=audio_pos; //0
    int actual_env=0;
    
    int previous_pos=0;
    int previous_env=0;
    int target_pos=adsr_pos[0];
    int target_env=adsr_vol[0];
    int delta_pos=adsr_pos[0];
    int delta_env=target_env;
    int sum1=0;
    int iadsr=0; //index
    int inc_env=1;
    double divperiod=(double)wav_spec.freq/(6.28*audio_frq);
    double mulgain=(double)volume*2.0;

    for (int i=0; i<samples; i++)
    {
        Sint16 value;
        if (actual_pos >= audio_len){
            value=wav_spec.silence; //end of adsr add silence
        }else{
            // linear appropx for the envelop
            while(actual_pos>=target_pos){
                previous_pos=target_pos;
                previous_env=target_env;
                iadsr++;
                if (iadsr<3){
                    target_pos=adsr_pos[iadsr];
                    target_env=adsr_vol[iadsr];
                }else{
                    target_pos=audio_len;
                    target_env=0;
                }
                if (previous_env < target_env){
                    delta_env=target_env-previous_env;
                    inc_env=1;
                }else{
                    delta_env=previous_env-target_env;
                    inc_env=-1;
                }
                delta_pos=target_pos-previous_pos;
                actual_env=previous_env; //actual value is on the previous point
                sum1=0;
            }
            sum1+=delta_env; // cumulate modulus
            while (sum1>delta_pos){
                sum1-=delta_pos; // one step on y axis
                actual_env+=inc_env;
            }
            double rad=(double)actual_pos/divperiod; // base osc period
            value= (double)actual_env * sin(rad)* mulgain;
        }
        //format conv
        uint8_t low, high;
        low=value&0xff;
        high=value>>8;// sign on bit7
        *stream++=low;
        *stream++=high;
        *stream++=low;
        *stream++=high;
        actual_pos++; //next sample
    }
    audio_pos=actual_pos;
}
bool AudioBuf_osc::isStarted() const {return started;}

void AudioBuf_osc::s_audio_callback(void *userdata, Uint8 *stream, int len){
    AudioBuf_osc* obj=(AudioBuf_osc*)userdata;
    obj->callback(stream, len);
}
