#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<error.h>
#include "/opt/faad_aac/faad2-2.7/include/faad.h"


  
#define FRAME_MAX_LEN 1024*5    
#define BUFFER_MAX_LEN 1024*1024   


static char* file_buf;
static int file_size;
static int fd_input;


int open_file(char* file_input)
{
	fd_input=open(file_input, O_RDONLY);
	if(fd_input<=0)
	{
		fprintf(stderr, "open %s fail\n", file_input);
		return  -1;
	}
	return  0;
}

int close_file(void)
{
	if(fd_input>0)
	{
		close(fd_input);
	}
	return 0;
}


int file_mem_alloc(void)
{
	struct stat file_st;
	if(fstat(fd_input, &file_st)<0)
	{
		perror("get file size fail\n");
		return  -1;
	}		
	file_size=file_st.st_size;
	printf("input file size: %d\n", file_size);
	file_buf=(char*)malloc(file_size);
	if(file_buf==NULL)
	{
		printf("file mem malloc fail\n");
		return  -1;
	}
	return  0;
}

int read_file_to_mem(void)
{
	int len;
	int bytes;
	len=file_size;
	bytes=0;
	do
	{
		bytes+=read(fd_input, file_buf + bytes, len - bytes);
	}while(bytes<file_size);
	return 0;
}

int file_mem_free(void)
{
	free(file_buf);
	return 0;
}


void show_usage()  
{  
    printf("usage\nfaaddec src_file dst_file");  
}  
  
int get_one_ADTS_frame(unsigned char* buffer, int buf_size, unsigned char* data ,int* data_size)  
{  
    int size = 0;  
  
    if(!buffer || !data || !data_size )  
    {  
        return -1;  
    }  
    while(1)  
    {  
		if(buf_size  < 7 )  
		{  
			return -1;  
		}		  
		if((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0) )  
		{  
			size |= (((buffer[3] & 0x03)) <<11);     
			//high 2 bit
			size |= (buffer[4]<<3);                
			//middle 8 bit
			size |= ((buffer[5] & 0xe0)>>5);        //low 3bit
			printf("len1=%x\n", (buffer[3] & 0x03));
			printf("len2=%x\n", buffer[4]);
			printf("len3=%x\n", (buffer[5] & 0xe0)>>5);
			printf("size=%d\r\n", (int)size);
			break;  
		}  
		--buf_size;  
		++buffer;  
    }  
    if(buf_size < size)  
    {  
        return -1;  
    }  
    memcpy(data, buffer, size);  
    *data_size = size;  
    return 0;  
}  
  
int main(int argc, char* argv[])  
{  
    static unsigned char frame[FRAME_MAX_LEN];  
    static unsigned char frame_mono[FRAME_MAX_LEN];
  
    char* fin_name;  
    char* fout_name;  
    int fout=0;  
    int      i;
    int      j;

	unsigned long samplerate;  
	unsigned char channels;  
	NeAACDecHandle decoder = 0;  
  
    int  data_size = 0;  
    int size = 0;  
  
    NeAACDecFrameInfo frame_info;  
    unsigned char* input_data; 
    unsigned char* pcm_data = NULL;  
  
    //analyse parameter   
    if(argc<3)  
    {  
        show_usage();  
        return -1;  
    }  

	fin_name=argv[1];
	fout_name=argv[2];  
    fout = open(fout_name, O_RDWR|O_CREAT|O_TRUNC);
    if(!fout)  
    {  
        printf("open %s fail\n", fout_name);  
        return -1;  
    }  
  
    open_file(fin_name);
	file_mem_alloc();
	read_file_to_mem();
     //open decoder   
    decoder = NeAACDecOpen();      
    if(get_one_ADTS_frame((unsigned char*)file_buf, file_size, frame, &size) < 0)  
    {  
    	printf("can't get one ADTS frame\n");
        return -1;  
    }  
}
