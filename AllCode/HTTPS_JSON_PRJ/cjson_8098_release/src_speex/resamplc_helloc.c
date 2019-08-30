#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>
#include <speex/speex_resampler.h>
  
int main()
{
 	int16_t input[4096];
 	int16_t output[8192];
 	FILE * fdr = fopen("mj.pcm","rb");
 	FILE * fdw = fopen("hello.pcm","wb");
 	int sr=16000;//原始采样率
 	int target_sr=44100;//重采样后采样率
	int in_len,out_len;
	SpeexResamplerState *resampler = speex_resampler_init(1, sr, target_sr, 10, NULL);//初始化
 	speex_resampler_skip_zeros(resampler);
 	while (1)
 	{   
 		int	readed=fread(input,2,1600,fdr);
 		if (readed<=0)
 		{   
 			break;
 		}
 		in_len=readed; 
 		out_len=6400;//输出缓冲大小
 		speex_resampler_process_int(resampler, 0, input, &in_len,output, &out_len); //output传入缓冲大小，传出实
 
 		fwrite(output,2,out_len,fdw);
 	}
 		speex_resampler_destroy(resampler);
 }

