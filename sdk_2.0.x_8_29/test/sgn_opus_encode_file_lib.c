/* Copyright (c) 2016-2018 Jean-Marc Valin 
 *	author: weicong.liu
 * 
*/

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "sgn_opus_audio_enc.h"

__declspec(dllexport) int __stdcall opus_encode_file(const char* wavfile, const char* opusfile)
{
    if (wavfile==NULL || opusfile==NULL )
    {
        fprintf(stdout, "Invalid file path\n");
        return -1;
    }
    int len = 0;
    char buf[40960] = {0};
    FILE *fin = fopen(wavfile, "rb");
    FILE *fout = fopen(opusfile, "wb");
    if(fout==NULL || fin==NULL){
        return -1;
    }
    // if(strstr(wavfile, ".wav")){
        fseek(fin, 44, SEEK_SET);
    // }
    buffer_t *out_buf = buffer_new();
    sgn_opus_encode_t *encoder = sgn_opus_encode_new(16000);
    sgn_opus_encode_start(encoder, out_buf);
    while((len=fread(buf, 1, 1024, fin)) > 0){
        sgn_opus_encode_append(encoder, buf, len, 0, out_buf);
        if(out_buf->data_size>0){
            fwrite(out_buf->data, 1, out_buf->data_size, fout);
            buffer_reset(out_buf);
        }
        memset(buf, 0, 40960);
    }
    sgn_opus_encode_append(encoder, out_buf->data, 0, 1, out_buf);
    fwrite(buf, 1, out_buf->data_size, fout);
    buffer_delete(out_buf);
    sgn_opus_encode_delete(encoder);
    if(fout)fclose(fout);
    if(fin)fclose(fin);
    return 0;
}
