/* Copyright (C)2002-2011 Jean-Marc Valin
 Copyright (C)2007-2013 Xiph.Org Foundation
 Copyright (C)2008-2013 Gregory Maxwell
 File: opusenc.c
 */
//gcc src/opusenc.c src/opus_header.c -I src  -I ../opus-1.3.1/include/ -I ../libogg-1.3.3/include/ ../opus-1.3.1/.libs/libopus.a ../libogg-1.3.3/src/.libs/libogg.a

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <getopt.h>

#include <stdlib.h>
#include <string.h>
#if (!defined WIN32 && !defined _WIN32) || defined(__MINGW32__)
#include <unistd.h>
#include <time.h>
#endif
#include <math.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
//# include "unicode_support.h"
/* We need the following two to set stdout to binary */
#include <windows.h>
# include <io.h>
# include <fcntl.h>
# define I64FORMAT "I64d"
#else
# define I64FORMAT "lld"
# define fopen_utf8(_x,_y) fopen((_x),(_y))
# define argc_utf8 argc
# define argv_utf8 argv
#endif

#include <opus.h>
#include <opus_multistream.h>
#include <ogg/ogg.h>
#include "lib/sgn_buf.h"


#ifdef VALGRIND
#include <valgrind/memcheck.h>
#define VG_UNDEF(x,y) VALGRIND_MAKE_MEM_UNDEFINED((x),(y))
#define VG_CHECK(x,y) VALGRIND_CHECK_MEM_IS_DEFINED((x),(y))
#else
#define VG_UNDEF(x,y)
#define VG_CHECK(x,y)
#endif


typedef struct {
   int version;
   int channels; /* Number of channels: 1..255 */
   int preskip;
   ogg_uint32_t input_sample_rate;
   int gain; /* in dB S7.8 should be zero whenever possible */
   int channel_mapping;
   /* The rest is only used if channel_mapping != 0 */
   int nb_streams;
   int nb_coupled;
   unsigned char stream_map[255];
} OpusHeader;

typedef struct sgn_opus_encode {
    int samplerate;
    int channel;
    int sampbyte;
    int frame_size;
    int complexity;
    int samplesize;
    int with_cvbr;
    int bigendian;
    OpusMSEncoder *encoder;
    unsigned char *packet;
    OpusHeader header;
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    int max_frame_bytes;
    int serialno;
    char *comments;
    int comments_length;
    long long enc_granulepos;
    long long original_samples;
    long long last_granulepos;
    int last_segments;
    int max_ogg_delay;
    int id;
    struct sgn_buf *buffer;
} sgn_opus_encode_t;

typedef struct {
   unsigned char *data;
   int maxlen;
   int pos;
} Packet;

static int write_uint32(Packet *p, ogg_uint32_t val)
{
   if (p->pos>p->maxlen-4)
      return 0;
   p->data[p->pos  ] = (val    ) & 0xFF;
   p->data[p->pos+1] = (val>> 8) & 0xFF;
   p->data[p->pos+2] = (val>>16) & 0xFF;
   p->data[p->pos+3] = (val>>24) & 0xFF;
   p->pos += 4;
   return 1;
}

static int write_uint16(Packet *p, ogg_uint16_t val)
{
   if (p->pos>p->maxlen-2)
      return 0;
   p->data[p->pos  ] = (val    ) & 0xFF;
   p->data[p->pos+1] = (val>> 8) & 0xFF;
   p->pos += 2;
   return 1;
}

static int write_chars(Packet *p, const unsigned char *str, int nb_chars)
{
   int i;
   if (p->pos>p->maxlen-nb_chars)
      return 0;
   for (i=0;i<nb_chars;i++)
      p->data[p->pos++] = str[i];
   return 1;
}

static int opus_header_to_packet(const OpusHeader *h, unsigned char *packet, int len)
{
   int i;
   Packet p;
   unsigned char ch;

   p.data = packet;
   p.maxlen = len;
   p.pos = 0;
   if (len<19)return 0;
   if (!write_chars(&p, (const unsigned char*)"OpusHead", 8))
      return 0;
   /* Version is 1 */
   ch = 1;
   if (!write_chars(&p, &ch, 1))
      return 0;

   ch = h->channels;
   if (!write_chars(&p, &ch, 1))
      return 0;

   if (!write_uint16(&p, h->preskip))
      return 0;

   if (!write_uint32(&p, h->input_sample_rate))
      return 0;

   if (!write_uint16(&p, h->gain))
      return 0;

   ch = h->channel_mapping;
   if (!write_chars(&p, &ch, 1))
      return 0;

   if (h->channel_mapping != 0)
   {
      ch = h->nb_streams;
      if (!write_chars(&p, &ch, 1))
         return 0;

      ch = h->nb_coupled;
      if (!write_chars(&p, &ch, 1))
         return 0;

      /* Multi-stream support */
      for (i=0;i<h->channels;i++)
      {
         if (!write_chars(&p, &h->stream_map[i], 1))
            return 0;
      }
   }

   return p.pos;
}

FILE *opus;
static inline int oe_copy_page(ogg_page *page, struct sgn_buf *out_buf) {
    sgn_buf_append(out_buf, page->header, page->header_len);
    sgn_buf_append(out_buf, page->body, page->body_len);
    return page->header_len + page->body_len;
}

/*Write an Ogg page to a file pointer*/

#define MAX_FRAME_BYTES 61295
#define IMIN(a,b) ((a) < (b) ? (a) : (b))   /**< Minimum int value.   */
#define IMAX(a,b) ((a) > (b) ? (a) : (b))   /**< Maximum int value.   */

#define writeint(buf, base, val) do{ buf[base+3]=((val)>>24)&0xff; \
                                     buf[base+2]=((val)>>16)&0xff; \
                                     buf[base+1]=((val)>>8)&0xff; \
                                     buf[base]=(val)&0xff; \
                                 }while(0)


static void comment_init(char **comments, int* length, const char *vendor_string) {
    /*The 'vendor' field should be the actual encoding library used.*/
    int ret = -1;
    int vendor_length = strlen(vendor_string);
    int user_comment_list_length = 0;
    int len = 8 + 4 + vendor_length + 4;
    char *p = (char*) malloc(len);
    if (p == NULL) {
        fprintf(stderr, "malloc failed in comment_init()\n");
        goto end;
    }
    memcpy(p, "OpusTags", 8);
    writeint(p, 8, vendor_length);
    memcpy(p + 12, vendor_string, vendor_length);
    writeint(p, 12 + vendor_length, user_comment_list_length);
    *length = len;
    *comments = p;
    ret = 0;
end:
    if (ret) {
        if (p)free(p);
    }
}

sgn_opus_encode_t *sgn_opus_encode_new(int samplerate)
{
// #ifdef WIN32
//     int nArgs;
//     LPWSTR *szArglist;
//     szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
//     LocalFree(szArglist);
// #endif
    sgn_opus_encode_t *opus_enc = (sgn_opus_encode_t *) malloc(sizeof(*opus_enc));
    // opus_enc->fp = fopen(output_filename, "wb");
    int ret = -1;
    // opus_enc->samplerate = 16000;
    opus_enc->samplerate = samplerate;
    opus_enc->channel = 1;
    opus_enc->complexity = 10;
    opus_enc->with_cvbr = 0;
    opus_enc->frame_size = 480;
    opus_enc->samplesize = 16;
    opus_enc->sampbyte = opus_enc->samplesize / 8;
    opus_enc->max_frame_bytes = 0;
    opus_enc->max_ogg_delay = 48000;
    opus_enc->buffer = sgn_buf_new();
    opus_enc->bigendian = 0;
//    opus_enc->max_ogg_delay = 16000;

    opus_enc->header.channels = opus_enc->channel;
    opus_enc->header.channel_mapping = 0;
    opus_enc->header.input_sample_rate = opus_enc->samplerate;
    opus_enc->header.gain = 0;
    comment_init(&opus_enc->comments, &opus_enc->comments_length, "libopus 1.2.1");
//    comment_pad(&opus_enc->comments, &opus_enc->comments_length, 512);
    opus_enc->encoder = opus_multistream_surround_encoder_create(
            opus_enc->samplerate, opus_enc->channel,
            opus_enc->header.channel_mapping, &opus_enc->header.nb_streams,
            &opus_enc->header.nb_coupled, opus_enc->header.stream_map,
            OPUS_APPLICATION_AUDIO, &ret);

    opus_enc->max_frame_bytes = (1275 * 3 + 7) * opus_enc->header.nb_streams;
    opus_enc->packet = malloc(sizeof(unsigned char) * opus_enc->max_frame_bytes);

    int lookahead = 0;
    int bitrate = ((64000 * opus_enc->header.nb_streams + 32000 * opus_enc->header.nb_coupled) * (IMIN(48, IMAX(8,((opus_enc->samplerate<44100?opus_enc->samplerate:48000)+1000)/1000)) + 16) + 32) >> 6;

    ret = opus_multistream_encoder_ctl(opus_enc->encoder, OPUS_SET_BITRATE(bitrate));
    if (ret != OPUS_OK) {
        fprintf(stderr, "Error OPUS_SET_BITRATE returned: %s\n", opus_strerror(ret));
        goto end;
    }

    ret = opus_multistream_encoder_ctl(opus_enc->encoder,
            OPUS_SET_VBR_CONSTRAINT(opus_enc->with_cvbr));
    if (ret != OPUS_OK) {
        fprintf(stderr, "Error OPUS_SET_VBR_CONSTRAINT returned: %s\n", opus_strerror(ret));
        goto end;
    }

    ret = opus_multistream_encoder_ctl(opus_enc->encoder, OPUS_SET_COMPLEXITY(opus_enc->complexity));
    if (ret != OPUS_OK) {
        fprintf(stderr, "Error OPUS_SET_COMPLEXITY returned: %s\n", opus_strerror(ret));
        goto end;
    }

#ifdef OPUS_SET_LSB_DEPTH
    ret = opus_multistream_encoder_ctl(opus_enc->encoder, OPUS_SET_LSB_DEPTH(IMAX(8, IMIN(24, opus_enc->samplesize))));
    if (ret != OPUS_OK) {
        fprintf(stderr, "Warning OPUS_SET_LSB_DEPTH returned: %s\n",
                opus_strerror(ret));
    }
#endif

    /*We do the lookahead check late so user CTLs can change it*/
    ret = opus_multistream_encoder_ctl(opus_enc->encoder, OPUS_GET_LOOKAHEAD(&lookahead));
    if (ret != OPUS_OK) {
        fprintf(stderr, "Error OPUS_GET_LOOKAHEAD returned: %s\n", opus_strerror(ret));
        goto end;
    }
    opus_enc->header.preskip = lookahead * (48000. / opus_enc->samplerate);
    ret = 0;
end:
    if (ret != 0) {

    }
    return opus_enc;
}

void sgn_opus_encode_delete(sgn_opus_encode_t *opus_enc)
{
    opus_multistream_encoder_destroy(opus_enc->encoder);
    ogg_stream_clear(&opus_enc->os);
    free(opus_enc->packet);
    free(opus_enc->comments);
    sgn_buf_delete(opus_enc->buffer);
    free(opus_enc);
}

// FILE *back_fp;
int sgn_opus_encode_start(sgn_opus_encode_t *opus_enc, struct sgn_buf *out_buf)
{
    // opus = fopen("back2.opus", "wb")
    int ret = -1;
    opus_enc->enc_granulepos = 0;
    opus_enc->original_samples = 0;
    opus_enc->last_granulepos = 0;
    opus_enc->last_segments = 0;
    opus_enc->id = -1;
//    sleep(1);
    // back_fp = fopen("back_file.pcm", "wb");
    // srand(((getpid() & 65535) << 15) ^ time(NULL));
    // opus_enc->serialno = rand();
    opus_enc->serialno = 160730;
    ret = opus_multistream_encoder_ctl(opus_enc->encoder, OPUS_RESET_STATE);
//    ogg_stream_reset(&opus_enc->os);
    if (ogg_stream_init(&opus_enc->os, opus_enc->serialno) == -1) {
        fprintf(stderr, "Error: stream init failed\n");
        goto end;
    }
    sgn_buf_reset(opus_enc->buffer);
    /*Write header*/
    {
        /*The Identification Header is 19 bytes, plus a Channel Mapping Table for
         mapping families other than 0. The Channel Mapping Table is 2 bytes +
         1 byte per channel. Because the maximum number of channels is 255, the
         maximum size of this header is 19 + 2 + 255 = 276 bytes.*/
        unsigned char header_data[276];
        int packet_size = opus_header_to_packet(&opus_enc->header, header_data, sizeof(header_data));
        opus_enc->op.packet = header_data;
        opus_enc->op.bytes = packet_size;
        opus_enc->op.b_o_s = 1;
        opus_enc->op.e_o_s = 0;
        opus_enc->op.granulepos = 0;
        opus_enc->op.packetno = 0;
        ogg_stream_packetin(&opus_enc->os, &opus_enc->op);
        while ((ret = ogg_stream_flush(&opus_enc->os, &opus_enc->og))) {
            if (!ret)break;
            ret = oe_copy_page(&opus_enc->og, out_buf);
            if (ret != opus_enc->og.header_len + opus_enc->og.body_len) {
                fprintf(stderr, "Error: failed writing header to output stream\n");
                goto end;
            }
        }
        opus_enc->op.packet = (unsigned char *) opus_enc->comments;
        opus_enc->op.bytes = opus_enc->comments_length;
        opus_enc->op.b_o_s = 0;
        opus_enc->op.e_o_s = 0;
        opus_enc->op.granulepos = 0;
        opus_enc->op.packetno = 1;
        ogg_stream_packetin(&opus_enc->os, &opus_enc->op);
        while ((ret = ogg_stream_flush(&opus_enc->os, &opus_enc->og))) {
            if (!ret)break;
            ret = oe_copy_page(&opus_enc->og, out_buf);
            if (ret != opus_enc->og.header_len + opus_enc->og.body_len) {
                fprintf(stderr, "Error: failed writing header to output stream\n");
                goto end;
            }
        }
    }
    ret = 0;
end:
    return ret;
}

static int pcm_read(float *encode_buffer, char *tmp_buffer, int realsamples,
        int bigendian) {
    int i;
    if (!bigendian) {
        for (i = 0; i < realsamples; i++) {
            encode_buffer[i] = ((tmp_buffer[i * 2 + 1] << 8)
                    | (tmp_buffer[i * 2] & 0xff)) / 32768.0f;
        }
    } else {
        for (i = 0; i < realsamples; i++) {
            encode_buffer[i] = ((tmp_buffer[i * 2] << 8)
                    | (tmp_buffer[i * 2 + 1] & 0xff)) / 32768.0f;
        }
    }
    return 0;
}

int sgn_opus_do_encode(sgn_opus_encode_t *opus_enc, int cur_frame_size, char *encode_buffer, struct sgn_buf  *out_buf)
{
    int ret;
    int size_segments;

    opus_enc->id++;
    int nbBytes = opus_multistream_encode(opus_enc->encoder, (opus_int16 *)encode_buffer, cur_frame_size, opus_enc->packet, opus_enc->max_frame_bytes);
    opus_enc->enc_granulepos += cur_frame_size * 48000 / opus_enc->samplerate;
    size_segments = (nbBytes + 255) / 255;

    opus_enc->op.packet = (unsigned char *) opus_enc->packet;
    opus_enc->op.bytes = nbBytes;
    opus_enc->op.b_o_s = 0;
    opus_enc->op.granulepos = opus_enc->enc_granulepos;
    if (opus_enc->op.e_o_s) {
        /*We compute the final GP as ceil(len*48k/input_rate)+preskip. When a
         resampling decoder does the matching floor((len-preskip)*input_rate/48k)
         conversion, the resulting output length will exactly equal the original
         input length when 0<input_rate<=48000.*/
        opus_enc->op.granulepos = ((opus_enc->original_samples * 48000 + opus_enc->samplerate - 1) / opus_enc->samplerate) + opus_enc->header.preskip;
    }
    opus_enc->op.packetno = 2 + opus_enc->id;
    ogg_stream_packetin(&opus_enc->os, &opus_enc->op);
    opus_enc->last_segments += size_segments;
    while ((opus_enc->op.e_o_s || (opus_enc->enc_granulepos + (opus_enc->frame_size * 48000 / opus_enc->samplerate) - opus_enc->last_granulepos > opus_enc->max_ogg_delay)
            || (opus_enc->last_segments >= 255)) ?
            ogg_stream_flush_fill(&opus_enc->os, &opus_enc->og, 255 * 255) :
            ogg_stream_pageout_fill(&opus_enc->os, &opus_enc->og, 255 * 255)) {
        if ((opus_enc->enc_granulepos
                + (opus_enc->frame_size * 48000 / opus_enc->samplerate)
                - opus_enc->last_granulepos > opus_enc->max_ogg_delay))
//            printf("-----bbbbbbbbbbbbbbbb:%d--------\n", opus_enc->max_ogg_delay);
        if (ogg_page_packets(&opus_enc->og) != 0)
            opus_enc->last_granulepos = ogg_page_granulepos(&opus_enc->og);
        opus_enc->last_segments -= opus_enc->og.header[26];
        ret = oe_copy_page(&opus_enc->og, out_buf);
//        printf("---max_ogg_delay1---\n");
    }
    ret = 0;
    return ret;
}

// FILE *back_fp;
// FILE *back_fp2;
// todo: 当in_data_len小于realsamples/2 会丢数据
int sgn_opus_encode_append(sgn_opus_encode_t *opus_enc, char *data_in, int in_data_len, int eof, struct sgn_buf *out_buf) {
    // static char append_buffer[512] = { 0 };       // 用于保留上次append剩余的数据(长度不够编码打包)
    // static int append_sgn_buf_data_len = 0;      // 上次append剩余的数据长度
    char tmp_buffer[960] = { 0 };
    float encode_buffer[opus_enc->frame_size];
    int realsamples = opus_enc->frame_size / 3;
    int cur_frame_size = 0;
    // int nbBytes = 0;
    // int bytes_encoded = 0;
    int i = 0;
    int ret;
    sgn_buf_append(opus_enc->buffer, data_in, in_data_len);
    int in_samples = opus_enc->buffer->data_len / opus_enc->sampbyte;
    // if (append_sgn_buf_data_len > 0) {
    //     memcpy(tmp_buffer, append_buffer, append_sgn_buf_data_len);
    // }
    while (in_samples >= realsamples) {
        // memcpy(tmp_buffer + append_sgn_buf_data_len, data_in + bytes_encoded, realsamples * opus_enc->sampbyte);
        // fwrite(tmp_buffer, 2, realsamples, back_fp);
        // bytes_encoded += (realsamples * opus_enc->sampbyte - append_sgn_buf_data_len);      // 已编码的数据长度
        // in_data_len -= (realsamples * opus_enc->sampbyte - append_sgn_buf_data_len);        // 本次缓冲区剩余的数据长度
        // if (append_sgn_buf_data_len > 0)append_sgn_buf_data_len = 0;

        memcpy(tmp_buffer, opus_enc->buffer->buf, realsamples * opus_enc->sampbyte);
        sgn_buf_remove(opus_enc->buffer, 0, realsamples * opus_enc->sampbyte);
//        pcm_read(encode_buffer, tmp_buffer, realsamples, opus_enc->bigendian);
        cur_frame_size = realsamples;
        opus_enc->original_samples += realsamples;
        //going encode
        sgn_opus_do_encode(opus_enc, cur_frame_size, tmp_buffer, out_buf);
        in_samples = opus_enc->buffer->data_len / opus_enc->sampbyte;
    }
    if (eof) {
        // memcpy(tmp_buffer + append_sgn_buf_data_len, data_in + bytes_encoded, in_samples * opus_enc->sampbyte);
        // fwrite(tmp_buffer, 2, in_samples, back_fp);
        memcpy(tmp_buffer, opus_enc->buffer->buf, opus_enc->buffer->data_len);
//        pcm_read(encode_buffer, tmp_buffer, in_samples, opus_enc->bigendian);
        sgn_buf_remove(opus_enc->buffer, 0, opus_enc->buffer->data_len);
        opus_enc->original_samples += in_samples;
        cur_frame_size = realsamples - ((realsamples - (in_samples > 0 ? in_samples : 1)) / (opus_enc->samplerate / 50)) * (opus_enc->samplerate / 50);
        for (i = cur_frame_size; i < realsamples; i++)tmp_buffer[i] = 0;
        opus_enc->op.e_o_s = 1;
        sgn_opus_do_encode(opus_enc, cur_frame_size, tmp_buffer, out_buf);
        // fclose(opus);
        // if (append_sgn_buf_data_len > 0)append_sgn_buf_data_len = 0;
    }
    //  else if (in_data_len > 0)    // 保留append剩余的数据
    // {
    //     memcpy(append_buffer, data_in + bytes_encoded, in_data_len);
    //     append_sgn_buf_data_len = in_data_len;
    // }
    ret = 0;
    return ret;
}

// int main(int argc, char **argv) {
//     if (argc < 2)
//         return 0;
//     back_fp = fopen("back_file.pcm", "wb");
//     back_fp2 = fopen("back_file2.pcm", "wb");
//     int len = 0;
//     char buf[2096] = { 0 };
//     long long read_len = 0;
//     sgn_opus_encode_t *opus_enc = sgn_opus_encode_new(argv[2]);
//     sgn_opus_encode_start(opus_enc);
//     FILE *fin = fopen(argv[1], "rb");
//     while ((len = fread(buf, 1, 160, fin)) == 160) {
//         sgn_opus_encode_append(opus_enc, buf, len, 0);
//         read_len += len;
//     }
//     printf("last read_len%d\n", len);
//     sgn_opus_encode_append(opus_enc, buf, len, 1);
//     read_len += len;
//     printf("read_len%d\n", read_len);
//     opus_enc->fp = fopen("twice.opus", "wb");
//     fclose(back_fp);
//     back_fp = back_fp2;
//     sgn_opus_encode_start(opus_enc);
// //    FILE *fin2 = fopen(argv[1], "rb");
//     fseek(fin, 0, SEEK_SET);
// //    printf("3 start\n");fflush(stdout);
//     while ((len = fread(buf, 1, 160, fin)) == 160) {
//         sgn_opus_encode_append(opus_enc, buf, len, 0);
//         read_len += len;
//     }
//     printf("last read_len%d\n", len);
//     sgn_opus_encode_append(opus_enc, buf, len, 1);
//     sgn_opus_encode_delete(opus_enc);
//     read_len += len;
//     printf("read_len%d\n", read_len);
//     fclose(fin);
//     fclose(back_fp);
// }

