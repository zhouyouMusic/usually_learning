/*
 * 用于加密sdk_cfg
 * 编译时需要链接压缩库 libz,即添加选项-lz
 * 使用: encode_res.exe -e source_file filename
 * 		 decode_res.exe -d filename source_file
 * 加密后的文件内容布局：1Byte压缩率|文件的内容nByte
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <zlib.h>

#include "xxtea/sgn_secure_code.h"

void encode_file(const char *src_file, const char *dst_file);

void decode_file(const char *src_file, const char *dst_file);


int main(int argc, char **argv)
{
    if (argc != 4 || (!strcmp(argv[1], "-e") && !strcmp(argv[1], "-d"))) {
        printf("Usage: %s [-e | -d] srcpath destpath\n"
               "options: \n"
               "    -e encrypt\n"
               "    -d decrypt\n", argv[0]);
        return 0;
    }

    if (!strcmp(argv[1], "-e")) {
		encode_file(argv[2], argv[3]);
    } else {
		decode_file(argv[2], argv[3]);
    }
    return 0;
}

void encode_file(const char *src_file, const char *dst_file)
{

	FILE *file = NULL;
	FILE *d_file = NULL;
	char comp = 0;
	char tmp_file_path[256] = {0};
	unsigned long bef_len = 0;
	unsigned long aft_len = 0;
	unsigned long aft_en_len = 0;
	unsigned char *bef_cmp = NULL;  unsigned char *aft_cmp = NULL;
	unsigned char *bef_enc = NULL;  unsigned char *aft_enc = NULL;
	int rt = 0;

	file = fopen(src_file, "rb");
	fseek(file, 0, SEEK_END);
	bef_len = ftell(file);
	printf("len:%d, comp:%d\n", bef_len, comp);fflush(stdout);
	fseek(file, 0, SEEK_SET);
	bef_cmp = (unsigned char *)calloc(1, bef_len);
	aft_cmp = (unsigned char *)calloc(1, bef_len);
	fread(bef_cmp, 1, bef_len, file);
	printf("len:%d, comp:%d\n", bef_len, comp);fflush(stdout);
	//压缩bef_cmp到aft_cmp   aft_cmp==bef_enc
	aft_len = bef_len;
	rt = compress(aft_cmp, &aft_len, bef_cmp, bef_len);
	printf("len:%d, comp:%d\n", bef_len, comp);fflush(stdout);
	//加密aft_cmp到aft_enc
	aft_en_len = sgn_secure_code(aft_cmp, aft_len, " fuck u crack", 9, &aft_enc, 'e');
	if((float)bef_len/aft_len > 1.000001)
		comp = bef_len/aft_len + 1;		// 压缩率
	else
		comp = bef_len/aft_len;
	printf("len:%d, comp:%d\n", bef_len, comp);fflush(stdout);
	d_file = fopen(dst_file, "wb");
	fwrite(&comp, 1, 1, d_file);
	fwrite(aft_enc, 1, aft_en_len, d_file);
	fclose(file);
	fclose(d_file);
	free(bef_cmp);
	free(aft_cmp);
	free(aft_enc);
}

void decode_file(const char *src_file, const char *dst_file)
{
	char filenumber = 0;
	char tmp_filename[24] = {0};
	char full_filename[256] = {0};
	unsigned long aft_datalen = 0, dec_data_len = 0, bef_cmp_len = 0;;
	unsigned char *bef_cmp = NULL;  unsigned char *aft_cmp = NULL;
	unsigned char *bef_enc = NULL;  unsigned char *aft_enc = NULL;
	char cmp = 0;
	FILE *d_file = NULL;
	FILE *file = fopen(src_file, "rb");
	fseek(file, 0, SEEK_END);
	int file_len = ftell(file);
	fseek(file, 0, SEEK_SET);


	fread(&cmp, 1, 1, file);
	aft_enc = (unsigned char *)malloc(file_len-1);
	fread(aft_enc, 1, file_len-1, file);
	dec_data_len = sgn_secure_code(aft_enc, file_len-1, " fuck u crack", 9, &bef_enc, 'd');	// 解密
	bef_cmp_len = dec_data_len*cmp;
	bef_cmp = (unsigned char *)malloc(bef_cmp_len);
	uncompress(bef_cmp, &bef_cmp_len, bef_enc, dec_data_len);								// 解压缩

	d_file = fopen(dst_file, "wb");
	fwrite(bef_cmp, 1, bef_cmp_len, d_file);
	fclose(d_file);
	free(aft_enc); aft_enc=NULL;
	free(bef_cmp); bef_cmp=NULL;
	free(bef_enc); bef_enc=NULL;
}

