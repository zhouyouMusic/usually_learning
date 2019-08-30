#include<stdio.h>
#include<string.h>
#include<stdlib.h>
	
int isCharIn(char *str1,const char *str2,int cmpLen)
{
	int retVal = 0;
	while(*str1!='\0')
	{
		if(*str1==*str2)
		{	
			if(!(strncmp(str1,str2,cmpLen)))
			{
				return 0;
			}
				
		}
		str1++;
	}
	return -1;
}
int main()
{	
	char *st1="ab";
	char *st2="ddac";
	int flag = isCharIn(st2,st1,2);
	printf("%d\n",flag);
	printf("%d\n",',');
	return 0;
}
