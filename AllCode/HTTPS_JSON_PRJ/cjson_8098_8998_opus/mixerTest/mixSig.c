#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
int main(int argc,char *argv[]) 
{
	char mixname[255];
	FILE *pcm1, *pcm2, *mix;
	char sample1, sample2;
	int value;

	pcm1 = fopen(argv[1],"r");
	pcm2 = fopen(argv[2],"r");

	strcpy (mixname, argv[1]);
	strcat (mixname, "_temp.wav");
	mix = fopen(mixname, "w");

	while(!feof(pcm1)) 
	{
		sample1 = fgetc(pcm1);
		sample2 = fgetc(pcm2);

		if((sample1 < 0) && (sample2 < 0)) 
		{
			value = sample1 + sample2 - (sample1 * sample2 / -(pow(2,16-1)-1));
		}else{
			value = sample1 + sample2 - (sample1 * sample2 / (pow(2,16-1)-1));
		}

		fputc(value, mix);
	}
	
	fclose(pcm1);
	fclose(pcm2);
	fclose(mix);
	
	return 0;
}








