# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <string.h>
# include <fcntl.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <alsa/asoundlib.h>
# include <linux/soundcard.h>

# include "mad.h"

snd_mixer_t	* mixer = NULL;
snd_mixer_elem_t * pcm_element = NULL;


long readSounds()
{
	long ll,lr;
	//处理事件
	snd_mixer_handle_events(mixer);
	//左声道
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_LEFT,&ll);
	//右声道
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_RIGHT,&lr);
	return (ll+lr) >> 1;
}
	 

void writeSounds(int value)
{
	//左声道
	snd_mixer_selem_set_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_LEFT,value);
	//右声道
	snd_mixer_selem_set_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_RIGHT,value);
}


int main(int argc,char **argv)
{
	snd_mixer_open(&mixer, 0);
	snd_mixer_attach(mixer, "default");
	snd_mixer_selem_register(mixer, NULL, NULL);
	snd_mixer_load(mixer);
	//找到Pcm对应的element,方法比较笨拙
	pcm_element = snd_mixer_first_elem(mixer);

	long int a, b;
	long alsa_min_vol, alsa_max_vol;
	///处理alsa1.0之前的bug，之后的可略去该部分代码
#if 0
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_LEFT, &a);
	snd_mixer_selem_get_playback_volume(pcm_element,
	SND_MIXER_SCHN_FRONT_RIGHT, &b);

#endif
	snd_mixer_selem_get_playback_volume_range(pcm_element,
	&alsa_min_vol,
	&alsa_max_vol);

	///设定音量范围
	snd_mixer_selem_set_playback_volume_range(pcm_element, 0, 100);
	
	long ret = readSounds();
	printf("value	%ld\n",ret);
	sleep(3);
	writeSounds(ret+5);
	sleep(3);
	writeSounds(ret+10);
	snd_mixer_close(mixer);
}


