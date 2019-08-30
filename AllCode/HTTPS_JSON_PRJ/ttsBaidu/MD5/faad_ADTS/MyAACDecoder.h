/**
 *
 * filename: MyAACDecoder.h
 * summary: convert aac to wave
 * author: caosiyang 
 * email: csy3228@gmail.com
 *
 */
#ifndef __MYAACDECODER_H__
#define __MYAACDECODER_H__
 
 
#include "faad.h"
#include <iostream>
using namespace std;
 
 
class MyAACDecoder {
public:
    MyAACDecoder();
    ~MyAACDecoder();
 
    int32_t Decode(char *aacbuf, uint32_t aacbuflen);
 
    const char* WavBodyData() const {
        return _mybuffer.Data();
    }
 
    uint32_t WavBodyLength() const {
        return _mybuffer.Length();
    }
 
    const char* WavHeaderData() const {
        return _wave_format.getHeaderData();
 
    }
 
    uint32_t WavHeaderLength() const {
        return _wave_format.getHeaderLength();
    }
 
private:
    MyAACDecoder(const MyAACDecoder &dec);
    MyAACDecoder& operator=(const MyAACDecoder &rhs);
 
    //init AAC decoder
    int32_t _init_aac_decoder(char *aacbuf, int32_t aacbuflen);
 
    //destroy aac decoder
    void _destroy_aac_decoder();
 
    //parse AAC ADTS header, get frame length
    uint32_t _get_frame_length(const char *aac_header) const;
 
    //AAC decoder properties
    NeAACDecHandle _handle;
    unsigned long _samplerate;
    unsigned char _channel;
 
    Buffer _mybuffer;
    WaveFormat _wave_format;
};
 
 
#endif /*__MYAACDECODER_H__*/

