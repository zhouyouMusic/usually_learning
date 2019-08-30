self.encoder = opus_encoder_create(24000, 1, OPUS_APPLICATION_VOIP, &opusError);
opus_encoder_ctl(self.encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_SUPERWIDEBAND));



- (void) encodeBufferList:(AudioBufferList *)bufferList {
    BOOL success = TPCircularBufferProduceBytes(_circularBuffer, bufferList->mBuffers[0].mData, bufferList->mBuffers[0].mDataByteSize);
    if (!success) {
        NSLog(@"insufficient space in circular buffer!");
    }

    if (!_encoding) {
            _encoding = YES;

            dispatch_async(self.processingQueue, ^{
                [self startEncodingLoop];
            });
    }
}


-(void)startEncodingLoop
{
    int32_t availableBytes = 0;
    opus_int16 *data = (opus_int16*)TPCircularBufferTail(_circularBuffer, &availableBytes);
    int availableSamples = availableBytes / _inputASBD.mBytesPerFrame;

    /*!
     *  Use dynamic duration
     */
//    int validSamples[6] = {2.5, 5, 10, 20, 40, 60}; // in milisecond
//    int esample = validSamples[0] * self.sampleRate / 1000;
//    for (int i = 0; i < 6; i++) {
//        int32_t samp = validSamples[i] * self.sampleRate / 1000;
//        if (availableSamples < samp) {
//            break;
//        }
//        esample = samp;
//    }

    /*!
     *  Use 20ms
     */
    int esample = 20 * self.sampleRate / 1000;

    if (availableSamples < esample) {
        /*!
         *  Out of data. Finish encoding
         */
        self.encoding = NO;
        [self.eDelegate didFinishEncode];
        return;
    }

//    printf("raw input value for packet \n");
//    for (int i = 0; i < esample * self.numberOfChannels; i++) {
//        printf("%d :", data[i]);
//    }

    int returnValue = opus_encode(_encoder, data, esample, _encoderOutputBuffer, 1000);

    TPCircularBufferConsume(_circularBuffer, esample * sizeof(opus_int16) * self.numberOfChannels);

//    printf("output encode \n");
//    for (int i = 0; i < returnValue; i++) {
//        printf("%d :", _encoderOutputBuffer[i]);
//    }

    NSMutableData *outputData = [NSMutableData new];
    NSError *error = nil;
    if (returnValue <= 0) {
        error = [OKUtilities errorForOpusErrorCode:returnValue];
    }else {
        [outputData appendBytes:_encoderOutputBuffer length:returnValue * sizeof(unsigned char)];
        unsigned char int_field[4];
        int_to_char(returnValue , int_field);
        NSData *header = [NSData dataWithBytes:&int_field[0] length:4 * sizeof(unsigned char)];
        if (self.eDelegate) {
            [self.eDelegate didEncodeWithData:header];
        }
    }

    if (self.eDelegate) {
        [self.eDelegate didEncodeWithData:outputData];
    }

    [self startEncodingLoop];
}


#include "android_log.h"  
#include "opus.h"  
#include "opus_types.h"  
#include "opus_multistream.h"  
  
#define SAMPLE_RATE 16000  
#define CHANNEL_NUM 1  
#define BIT_RATE 16000  
#define BIT_PER_SAMPLE 16  
#define WB_FRAME_SIZE 320  
#define DATA_SIZE 1024 * 1024 * 4  
  
int encode(char* in, int len, unsigned char* opus, int* opus_len) {  
    int err = 0;  
    opus_int32 skip = 0;  
  
    OpusEncoder *enc = opus_encoder_create(SAMPLE_RATE, CHANNEL_NUM,  
            OPUS_APPLICATION_VOIP, &err);  
    if (err != OPUS_OK) {  
        fprintf(stderr, "cannnot create opus encoder: %s\n",  
                opus_strerror(err));  
        enc = NULL;  
        return -1;  
    }  
  
    opus_encoder_ctl(enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));  
    opus_encoder_ctl(enc, OPUS_SET_BITRATE(BIT_RATE));  
    opus_encoder_ctl(enc, OPUS_SET_VBR(1));  
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));  
    opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));  
    opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));  
    opus_encoder_ctl(enc, OPUS_SET_DTX(0));  
    opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(0));  
    opus_encoder_ctl(enc, OPUS_GET_LOOKAHEAD(&skip));  
    opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));  
  
    short frame_size = WB_FRAME_SIZE;  
    int frame_bytes = (frame_size << 1);  
  
    opus_int16 *frame = (opus_int16 *) in;  
    unsigned char *cbits = opus;  
  
    while (len > frame_bytes) {  
        int nbytes = opus_encode(enc, frame, frame_size, cbits + sizeof(char),  
                640 - sizeof(short));  
        if (nbytes > frame_size * 2 || nbytes < 0) {  
            return -1;  
        }  
        cbits[0] = nbytes;  
        frame += WB_FRAME_SIZE;  
        cbits += nbytes + sizeof(char);  
        len -= frame_bytes;  
        *opus_len += nbytes + sizeof(char);  
    }  
    opus_encoder_destroy(enc);  
    return 0;  
}  
  
int decode(unsigned char* in, int len, short* out, int* out_len) {  
    int err = 0;  
    opus_int32 skip = 0;  
    *out_len = 0;  
  
    OpusDecoder *dec = opus_decoder_create(SAMPLE_RATE, 1, &err);  
    if (err != OPUS_OK) {  
        fprintf(stderr, "cannnot decode opus: %s\n", opus_strerror(err));  
        dec = NULL;  
        return -1;  
    }  
  
    short frame_size = WB_FRAME_SIZE;  
  
    opus_int16 *frame = (opus_int16 *) in;  
  
    while (len > 0) {  
        int nbytes = in[0];  
        if (nbytes <= 0) {  
            return -1;  
        }  
        int decode_len = opus_decode(dec, in + sizeof(char), nbytes, out,  
                frame_size, 0);  
        if (decode_len != frame_size) {  
            return -1;  
        }  
  
        in += sizeof(char) + nbytes;  
        out += frame_size;  
        len -= nbytes - sizeof(char);  
        *out_len += frame_size;  
    }  
    opus_decoder_destroy(dec);  
    return 0;  
}  
  
int encode_wav_file(char *in_file_path, char *out_file_path) {  
    FILE *fin = fopen(in_file_path, "rb");  
  
    if (fin == NULL || fin == 0) {  
        return -1;  
    }  
    char *in = (char*) malloc(DATA_SIZE);  
    memset(in, 0, DATA_SIZE);  
    int len = fread(in, 1, DATA_SIZE, fin);  
    if (len == 0) {  
        return -1;  
    }  
    FILE *fout = fopen(out_file_path, "wb");  
  
    if (fout == NULL || fout == 0) {  
        return -1;  
    }  
  
    unsigned char *out = (unsigned char*) malloc(DATA_SIZE);  
    memset(out, 0, DATA_SIZE);  
    int out_len = 0;  
    encode(in, len, out, &out_len);  
    if (len < 0) {  
        return -1;  
    }  
    fwrite(out, 1, out_len * sizeof(unsigned char), fout);  
  
    free(in);  
    free(out);  
    fclose(fin);  
    fclose(fout);  
    return len;  
}  
  
int make_wav_header(FILE *out, int len) {  
    int size = 0;  
    int *sz = &size;  
    int number;  
    int * nm = &number;  
  
    // RIFF  4 bytes  
    fseek(out, 0, SEEK_SET);  
    fputs("RIFF", out);  
  
    // len   4 bytes  
    len = (len + 44 - 8);  
    fwrite(&len, 2, 1, out);  
    number = 0;  
    fwrite(nm, 2, 1, out);  
  
    // WAVE  4 bytes  + "fmt " 4 bytes  
    fputs("WAVEfmt ", out);  
  
    // size1   4 bytes  
    number = 16;  
    fwrite(nm, 2, 1, out);  
    number = 0;  
    fwrite(nm, 2, 1, out);  
  
    // format tag       2 bytes  
    number = 1;  
    fwrite(nm, 2, 1, out);  
  
    // channel    2 bytes  
    number = CHANNEL_NUM;  
    fwrite(nm, 2, 1, out);  
  
    // sample rate          4 bytes  
    number = SAMPLE_RATE;  
    fwrite(nm, 2, 1, out);  
    number = 0;  
    fwrite(nm, 2, 1, out);  
  
    //byte per seconds   4 bytes  
    number = 22664;  
    fwrite(nm, 2, 1, out);  
    number = 0;  
    fwrite(nm, 2, 1, out);  
  
    // block align   2 bytes  
    number = CHANNEL_NUM * BIT_PER_SAMPLE / 8;  
    fwrite(nm, 2, 1, out);  
  
    // bitPerSample   2 bytes  
    number = 16;  
    fwrite(nm, 2, 1, out);  
  
    // "data"      4 bytes  
    fputs("data", out);  
  
    // size2    4 bytes  
    size = (size - 36);  
    fwrite(sz, 2, 1, out);  
    number = 0;  
    fwrite(nm, 2, 1, out);  
  
    return 0;  
}  
  
int decode_opus_file(char *in_file_path, char *out_file_path) {  
    printf("%s\n", in_file_path);  
    FILE *fin = fopen(in_file_path, "rb");  
    if (fin == NULL || fin == 0) {  
        return -1;  
    }  
    unsigned char *in = (unsigned char *) malloc(DATA_SIZE);  
    memset(in, 0, DATA_SIZE);  
    int len = fread(in, 1, DATA_SIZE, fin);  
  
    FILE *fout = fopen(out_file_path, "wb");  
    if (fout == NULL || fout == 0) {  
        return -1;  
    }  
    short *out = (short *) malloc(DATA_SIZE);  
    memset(out, 0, DATA_SIZE);  
  
    int out_len = 0;  
    out += 44;  
    decode(in, len, (short *) out, &out_len);  
    if (len < 0) {  
        return -1;  
    }  
    fwrite(out, 1, out_len * sizeof(short), fout);  
    int err = make_wav_header(fout, out_len);  
  
    free(in);  
    free(out);  
    fclose(fin);  
    fclose(fout);  
    return out_len;  
} 


