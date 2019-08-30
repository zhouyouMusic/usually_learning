#include "MyAACDecoder.h"
 
 
MyAACDecoder::MyAACDecoder(): _handle(NULL), _samplerate(44100), _channel(2), _mybuffer(4096, 4096) {
}
 
 
MyAACDecoder::~MyAACDecoder() {
    _destroy_aac_decoder();
}
 
 
int32_t MyAACDecoder::Decode(char *aacbuf, uint32_t aacbuflen) {
    int32_t res = 0;
    if (!_handle) {
        if (_init_aac_decoder(aacbuf, aacbuflen) != 0) {
            ERR1(":::: init aac decoder failed ::::");
            return -1;
        }
    }
 
    //clean _mybuffer
    _mybuffer.Clean();
 
    uint32_t donelen = 0;
    uint32_t wav_data_len = 0;
    while (donelen < aacbuflen) {
        uint32_t framelen = _get_frame_length(aacbuf + donelen);
 
        if (donelen + framelen > aacbuflen) {
            break;
        }
 
        //decode
        NeAACDecFrameInfo info;
        void *buf = NeAACDecDecode(_handle, &info, (unsigned char*)aacbuf + donelen, framelen);
        if (buf && info.error == 0) {
            if (info.samplerate == 44100) {
                //44100Hz
                //src: 2048 samples, 4096 bytes
                //dst: 2048 samples, 4096 bytes
                uint32_t tmplen = info.samples * 16 / 8;
                _mybuffer.Fill((const char*)buf, tmplen);
                wav_data_len += tmplen;
            } else if (info.samplerate == 22050) {
                //22050Hz
                //src: 1024 samples, 2048 bytes
                //dst: 2048 samples, 4096 bytes
                short *ori = (short*)buf;
                short tmpbuf[info.samples * 2];
                uint32_t tmplen = info.samples * 16 / 8 * 2;
                for (int32_t i = 0, j = 0; i < info.samples; i += 2) {
                    tmpbuf[j++] = ori[i];
                    tmpbuf[j++] = ori[i + 1];
                    tmpbuf[j++] = ori[i];
                    tmpbuf[j++] = ori[i + 1];
                }
                _mybuffer.Fill((const char*)tmpbuf, tmplen);
                wav_data_len += tmplen;
            }
        } else {
            ERR1("NeAACDecDecode() failed");
        }
 
        donelen += framelen;
    }
 
    //generate Wave header
    _wave_format.setSampleRate(_samplerate);
    _wave_format.setChannel(_channel);
    _wave_format.setSampleBit(16);
    _wave_format.setBandWidth(_samplerate * 16 * _channel / 8);
    _wave_format.setDataLength(wav_data_len);
    _wave_format.setTotalLength(wav_data_len + 44);
    _wave_format.GenerateHeader();
 
    return 0;
}
 
 
uint32_t MyAACDecoder::_get_frame_length(const char *aac_header) const {
    uint32_t len = *(uint32_t *)(aac_header + 3);
    len = ntohl(len); //Little Endian
    len = len << 6;
    len = len >> 19;
    return len;
}
 
 
int32_t MyAACDecoder::_init_aac_decoder(char* aacbuf, int32_t aacbuflen) {
    unsigned long cap = NeAACDecGetCapabilities();
    _handle = NeAACDecOpen();
    if (!_handle) {
        ERR1("NeAACDecOpen() failed");
        _destroy_aac_decoder();
        return -1;
    }
 
    NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(_handle);
    if (!conf) {
        ERR1("NeAACDecGetCurrentConfiguration() failed");
        _destroy_aac_decoder();
        return -1;
    }
    NeAACDecSetConfiguration(_handle, conf);
 
    long res = NeAACDecInit(_handle, (unsigned char *)aacbuf, aacbuflen, &_samplerate, &_channel);
    if (res < 0) {
        ERR1("NeAACDecInit() failed");
        _destroy_aac_decoder();
        return -1;
    }
    //fprintf(stdout, "SampleRate = %d\n", _samplerate);
    //fprintf(stdout, "Channel    = %d\n", _channel);
    //fprintf(stdout, ":::: init aac decoder done ::::\n");
 
    return 0;
}
 
 
void MyAACDecoder::_destroy_aac_decoder() {
    if (_handle) {
        NeAACDecClose(_handle);
        _handle = NULL;
    }
}
