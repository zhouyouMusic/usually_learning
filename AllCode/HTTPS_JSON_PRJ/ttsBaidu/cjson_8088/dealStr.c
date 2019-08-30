#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct
{
	unsigned chA;
	unsigned chB;
	unsigned chC;
}NUM_CODING;

NUM_CODING getNumCoding(char *num)
{
	NUM_CODING numCd = {0};
	numCd.chA = *num;
	numCd.chB = *(num + 1);
	numCd.chC = *(num + 2);
	return numCd;
}

void codingNum(char chr,char *newBuf)
{
	NUM_CODING numCd = {0};
	switch(chr)
	{
		case '0':
				numCd = getNumCoding("零");
				break;
		case '1':
				numCd = getNumCoding("一");
				break;
		case '2':
				numCd = getNumCoding("二");
				break;
		case '3':
				numCd = getNumCoding("三");
				break;
		case '4':
				numCd = getNumCoding("四");
				break;
		case '5':
				numCd = getNumCoding("五");
				break;
		case '6':
				numCd = getNumCoding("六");
				break;
		case '7':
				numCd = getNumCoding("七");
				break;
		case '8':
				numCd = getNumCoding("八");
				break;
		case '9':
				numCd = getNumCoding("九");
				break;
		case 'b':
				numCd = getNumCoding("百");
				break;
		case 's':
				numCd = getNumCoding("十");
				break;
	}	
	*newBuf = numCd.chA;
	*(newBuf+1) = numCd.chB;
	*(newBuf+2) = numCd.chC;
}


char *createTtsStr(char *postStr)
{
	unsigned char chr1 = 0,chr2 = 0,chr3 = 0;
	int i = 0,flag = 0;
	char *ttsStr = (char *)calloc(1,1024);
	char *backStr = ttsStr;
	while(*(postStr+2)!='\0')
	{	
		chr1 = *postStr;
		chr2 = *(postStr + 1);
		chr3 = *(postStr + 2);
		if(chr1>=0xb0 && chr2>=0x80 && chr3 >= 0x80) 
		{
			if((chr1==0xef) ||(chr1==0xe3 && chr2 == 0x80))
			{
				if(flag == 0)
				{
					*(ttsStr+i) = ',';			//将中文符号转换成英文 ","号,
					i++;				//将中文所占的三个字节替换成一个字节
					flag = 1;
				}		
			}
			else
			{
				*(ttsStr + i) = chr1;					//如果是汉字则保留三字节编码
				*(ttsStr + i + 1) = chr2;
				*(ttsStr + i + 2) = chr3;
				flag = 0;
				i += 3;
			}
			postStr += 3;
		}	
		else if(isalpha(chr1))				//如果是字母则保留
		{
			*(ttsStr + i) = chr1;
			i++;
			postStr = postStr + 1;
			flag = 0;
		}
		else if(chr1 >= 48 && chr1 <= 57)		//如果是数字，则转换成中文的数字
		{
			codingNum(chr1,ttsStr+i);
			i += 3;
			if((chr2 >= 48 && chr2 <= 57)&& (chr1!=48))			//判断是否是至少两位数
			{
				printf("------------115	\n");
				if(chr3 >= 48 && chr3 <= 57)			//判断是否是至少三位数,不支持三位数以上播报
				{
					printf("	-----------------118	\n");
					codingNum('b',ttsStr+i);
					i += 3;
					if((chr3 != 48)&&(chr2 != 48))								//当第三位不是0，那么这个三位数要全部读出来
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum('s',ttsStr+i);
						i += 3;					
						codingNum(chr3,ttsStr+i);
						i += 3;
					}
					else if((chr3 != 48)&&(chr2 == 48)) 
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum(chr3,ttsStr+i);
						i += 3;
					}	
					else if((chr3==48)&&(chr2!=48))				//当第三位是0,则比如三百二十，最后一个0可不读
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
						codingNum('s',ttsStr+i);
						i += 3;					
					}	
					postStr += 3;
				}
				else
				{
					codingNum('s',ttsStr+i);
					i += 3;
					if(chr2 != 48)							//两位数时，如果第二位是0，也可以不读出来，只读比如二十
					{
						codingNum(chr2,ttsStr+i);
						i += 3;
					}
					postStr += 2;
				}
			}
			else
				postStr = postStr + 1;
			flag = 0;
		}	
		else
			postStr = postStr +1;
	}
	
	printf("%s\n",backStr);
//	return backStr;
}


int main(int argc,char **argv)
{
	int i = 0;
	char pp[1000] = "";
	i = strlen(argv[1]);
	printf("%d\n",i);
	createTtsStr(argv[1]);
//	strcpy(pp,argv[1]);	
/*	while(pp[i] != '\0')
	{
		char ch = pp[i];
		if(UTF_CODING(ch))
		{
			printf("中文\n");
		}
		i++;
	}*/
//	printf("%s\n",pp);
}
