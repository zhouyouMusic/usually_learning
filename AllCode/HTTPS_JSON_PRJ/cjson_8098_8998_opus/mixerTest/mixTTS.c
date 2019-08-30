#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main()
{
/*
	 Y = A + B - (A * B / (-(2 pow(n-1) -1))) 
	Else Y = A + B - (A * B / (2 pow(n-1)) 
*/



	FILE * fF = fopen("FailConnect.pcm","rb");
	FILE * fS = fopen("SucConnect.pcm","rb");
	fseek(fF,0,SEEK_END);
	int lenFail = ftell(fF);
	fseek(fF,0,SEEK_SET);
	printf("Fail Length 	%d\n",lenFail);

	fseek(fS,0,SEEK_END);
	int lenSuc = ftell(fS);
	fseek(fS,0,SEEK_SET);
	printf("Suc Length    	 %d\n",lenSuc);
	
	short * failBuf = (short*)malloc(lenFail+1);
	short * sucBuf = (short*)malloc(lenSuc+1);
	
	short * outBuf[lenFail+lenSuc];
	FILE * fO = fopen("outBuf.pcm","wb");
	
	fread(failBuf,lenFail,1,fF);
	fread(sucBuf,lenSuc,1,fS);

	int maxLen = lenFail + lenSuc;
	int i = 0,j = 0;
	while(1)
	{
		if((lenFail > 0) && (lenSuc > 0))
		{
			int valueY = 0;
			if((failBuf[i] < 0)&&(sucBuf[i] < 0))
			{
				valueY = (failBuf[i] + sucBuf[i]) - (failBuf[i] * sucBuf[i] / (-(pow(2,15)-1)));
			}else{
				valueY = (failBuf[i] + sucBuf[i]) - (failBuf[i] * sucBuf[i] / (pow(2,15)-1));	
			}
			outBuf[i] = (short)valueY;
			i++;
		}
		lenFail--;
		lenSuc--;
		if((lenSuc <=0 )&&(lenSuc <=0))
			break;

	}
	fwrite(outBuf,maxLen,1,fO);

}
