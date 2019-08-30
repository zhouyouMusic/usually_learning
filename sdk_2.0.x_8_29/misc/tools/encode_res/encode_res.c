/*
 * 用于加密离线版资源
 * 编译时需要链接压缩库 libz,即添加选项-lz
 * 使用: encode_res.exe -e source_dir filename
 * 		 decode_res.exe -d filename soruce_dir
 * 加密后的文件内容布局：1Byte文件数|第n个文件名24Byte|该文件的压缩率1Byte|该文件的长度m 4Byte|文件的内容mByte
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <zlib.h>

#include "xxtea/sgn_secure_code.h"

struct res_data
{
	char filename[24];		//文件名
	char comp;				//压缩率
	int datalen;			//数据长度
	struct res_data *next;
	char data[];			//数据
};

void encode_dir_to_file(const char *dir_path, const char *file_path);

void decode_file_to_dir(const char *file_path, const char *dir_path);


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
		encode_dir_to_file(argv[2], argv[3]);
    } else {
		decode_file_to_dir(argv[2], argv[3]);
    }
    return 0;
}

static void write_encoded_res_data_to_file(struct res_data *res_data_head, const char *file_path)
{
	FILE *file = fopen(file_path, "wb");
	char filenumber = 0;
	fwrite(&filenumber, 1, 1, file);
	struct res_data *p = res_data_head;
	struct res_data *tmp = NULL;
	while(p)
	{
		fwrite(p->filename, 1, 24, file);
		fwrite(&p->comp, 1, 1, file);
		fwrite(&p->datalen, 1, 4, file);
		fwrite(p->data, 1, p->datalen, file);
		tmp = p->next;
		free(p);
		p = tmp;
		filenumber++;
	}
	fseek(file, 0 , SEEK_SET);
	fwrite(&filenumber, 1, 1, file);
	fclose(file);
}

void encode_dir_to_file(const char *dir_path, const char *file_path)
{
	DIR *dir = opendir(dir_path);
	struct dirent *entry;
	FILE *file = NULL;
	char tmp_file_path[256] = {0};
	unsigned long bef_len = 0;
	unsigned long aft_len = 0;
	unsigned long aft_en_len = 0;
	unsigned char *bef_cmp = NULL;  unsigned char *aft_cmp = NULL;
	unsigned char *bef_enc = NULL;  unsigned char *aft_enc = NULL;
	int rt = 0;
	struct res_data *res_data_head = NULL;
	struct res_data **pp = &res_data_head;
	while((entry=readdir(dir))!= NULL)
	{
		if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)continue;
		sprintf(tmp_file_path, "%s/%s", dir_path, entry->d_name);
		file = fopen(tmp_file_path, "rb");
		fseek(file, 0, SEEK_END);
		bef_len = ftell(file);
		fseek(file, 0, SEEK_SET);
		bef_cmp = (unsigned char *)calloc(1, bef_len);
		aft_cmp = (unsigned char *)calloc(1, bef_len);
		fread(bef_cmp, 1, bef_len, file);
		//压缩bef_cmp到aft_cmp   aft_cmp==bef_enc
		aft_len = bef_len;
		rt = compress(aft_cmp, &aft_len, bef_cmp, bef_len);
		//加密aft_cmp到aft_enc
		aft_en_len = sgn_secure_code(aft_cmp, aft_len, " Fuck u crak", 9, &aft_enc, 'e');
		*pp = (struct res_data *)malloc(aft_en_len + sizeof(*res_data_head));
		memcpy((*pp)->filename, entry->d_name, 24);
		if((float)bef_len/aft_len > 1.000001)
			(*pp)->comp = bef_len/aft_len + 1;		// 压缩率
		else
			(*pp)->comp = bef_len/aft_len;
		//将加密好的数据复制到res_data链表上
		memcpy((*pp)->data, aft_enc, aft_en_len);
		(*pp)->datalen = aft_en_len;
		pp = &(*pp)->next;	//取next的地址，为赋值备用
		free(bef_cmp);
		free(aft_cmp);
		free(aft_enc);
	}
	closedir(dir);
	*pp = NULL;
	//将加密好的数据链表按规则写到一个文件上
	write_encoded_res_data_to_file(res_data_head, file_path);

}

void decode_file_to_dir(const char *file_path, const char *dir_path)
{
	char filenumber = 0;
	char tmp_filename[24] = {0};
	char full_filename[256] = {0};
	unsigned long aft_datalen = 0, dec_data_len = 0, bef_cmp_len = 0;;
	unsigned char *bef_cmp = NULL;  unsigned char *aft_cmp = NULL;
	unsigned char *bef_enc = NULL;  unsigned char *aft_enc = NULL;
	char cmp = 0;
	FILE *res = fopen(file_path, "rb");
	fread(&filenumber, 1, 1, res);
	struct res_data *res_data_head = NULL;
	struct res_data **p = &res_data_head;
	while(filenumber)
	{
		fread(tmp_filename, 1, 24, res);
		fread(&cmp, 1, 1, res);
		fread(&aft_datalen, 1, 4, res);
		aft_enc = (unsigned char *)malloc(aft_datalen);
		fread(aft_enc, 1, aft_datalen, res);
		dec_data_len = sgn_secure_code(aft_enc, aft_datalen, " Fuck u crak", 9, &bef_enc, 'd');	// 解密
		bef_cmp_len = dec_data_len*cmp;
		bef_cmp = (unsigned char *)malloc(bef_cmp_len);
		uncompress(bef_cmp, &bef_cmp_len, bef_enc, dec_data_len);								// 解压缩

		//将解密好的数据放到链表上
		*p = (struct res_data *)malloc(sizeof(**p)+bef_cmp_len);
		(*p)->datalen = bef_cmp_len;
		memcpy((*p)->filename, tmp_filename, 24);
		memcpy((*p)->data, bef_cmp, bef_cmp_len);
		p = &(*p)->next;

		free(aft_enc); aft_enc=NULL;
		free(bef_cmp); bef_cmp=NULL;
		free(bef_enc); bef_enc=NULL;
		memset(tmp_filename, 0, 24);
		cmp = 0;
		aft_datalen = 0;
		dec_data_len = 0;
		bef_cmp_len = 0;
		filenumber--;
	}
	*p = NULL;

	// 将解密好的数据按原名称分别恢复到各个文件
	mkdir(dir_path);
	FILE *tmpfile = NULL;
	int i = 0;
	struct res_data *tmp_p = NULL;
	int w_size = 0;
	while(res_data_head)
	{
		sprintf(full_filename, "%s/%s", dir_path, res_data_head->filename);
		tmpfile = fopen(full_filename, "wb");
		while(w_size != res_data_head->datalen){
			w_size += fwrite(res_data_head->data+w_size, 1, res_data_head->datalen-w_size, tmpfile);
		}
//		printf("write data len:%d, writed:%d \n", res_data_head->datalen, w_size);
		w_size = 0;
		fclose(tmpfile);
		tmpfile = NULL;
		tmp_p = res_data_head->next;
		free(res_data_head);
		res_data_head = tmp_p;
		i++;
	}
}

