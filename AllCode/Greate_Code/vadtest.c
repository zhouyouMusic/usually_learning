#include "huwenvadInterface.h"
#include <string.h>
#include <stdio.h>
int main(int argc,char **argv) {
	HuwenVADHandle handle;
	huwenVadInit(&handle,16000,0.70f,0.5f,2,10);
	FILE *fp = fopen("./beam1.wav","rb");
	fseek(fp,44,SEEK_SET);
	char buffer[256];
	int length = -1;
	int number;
	long times = 0;
	float sorces[2];
	FILE *fp1 = fopen("./vadtestesp.txt","wb");
	printf("===========\n");
	while ((length = fread(buffer,1024,1,fp))) {
		number = huwenVadFeedData(&handle,buffer,1024,sorces);
//		switch(number) {
//			case 1:
//				fprintf(fp1,"%f\n",sorces[0]);
//				break;
//			case 2:
				fprintf(fp1,"%f\n",sorces[0]);
				fprintf(fp1,"%f\n",sorces[1]);
//				break;
//		}
		printf("%ldms %d %f %f\n",times,number,sorces[0],sorces[1]);
		times += 16;
	}
	fclose(fp1);
	fclose(fp);
	huwenVadDestroy(&handle);
}
