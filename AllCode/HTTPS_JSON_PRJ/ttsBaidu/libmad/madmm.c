# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>


#include <string.h>
#include<fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

# include "mad.h"
# include "madAlsa.h"


#include "alsaInterface.h"
/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */
int set_pcm(int rateSet,int channelSet);
snd_pcm_t*             handle=NULL;        //PCI设备句柄
snd_pcm_hw_params_t*   params=NULL;//硬件信息和PCM流配置
int main(int argc,char **argv)
{
	FILE * fp = fopen("mj.wav","rb");
	char buff[1024*20] = "";
	char *ptr = buff;
	int ret = 0,sum = 0;
  	if(set_pcm(44100,2)!=0)                 //设置pcm 参数
    {
        printf("set_pcm fialed:\n");
        return -1;   
   	}
	while(1)
	{
		
		ret = fread(buff,2,1023,fp);
		if(ret<1)
			break;
    	snd_pcm_writei (handle, buff, ret);  
	}
	
  	snd_pcm_drain(handle);
  	snd_pcm_close(handle);
}


int set_pcm(int rateSet,int channelSet)
{
    int    rc;     
    int  dir=0;
    int rate = rateSet;	//16000  44100            /* 采样频率 44.1KHz*/
    int format = SND_PCM_FORMAT_S16_LE; /*     量化位数 16      */
    int channels = channelSet;     // 1 or 2            /*     声道数 2           */
    int buffer_time = 50000;// 500000;
    int period_time = 10000;///10000;
	 
    rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if(rc<0)
        {
                perror("\nopen PCM device failed:");
                exit(1);
        }
    snd_pcm_hw_params_alloca(&params); //分配params结构体
        
    rc=snd_pcm_hw_params_any(handle, params);//初始化params
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_any:");
                exit(1);
        }
	
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);                                 //初始化访问权限
        if(rc<0)
        {
                perror("\nsed_pcm_hw_set_access:");
                exit(1);

        }
        
    rc=snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);             //设置16位采样精度  
        if(rc<0)
       {
            perror("snd_pcm_hw_params_set_format failed:");
            exit(1);
        } 
        
    rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_channels:");
                exit(1);
        }    
        
     rc=snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);  //设置>频率
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_rate_near:");
                exit(1);
        }   
        
	rc=snd_pcm_hw_params_set_buffer_time_near(handle,params,&buffer_time, &dir);
		if(rc<0)
		{
			perror("\nnd_pcm_buffer_errr:	");
			exit(1);
		}
         
	rc=snd_pcm_hw_params_set_period_time_near(handle,params,&period_time, &dir);
		if(rc<0)
		{
			perror("\nnd_pcm_period_errr:	");
			exit(1);
		}
    rc = snd_pcm_hw_params(handle, params);
        if(rc<0)
        {
        perror("\nsnd_pcm_hw_params: ");
        exit(1);
        } 
   
    return 0;              
}

