#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void func(char **buf,int *p)
{
//	char *ptr = malloc(15);
//	char *sy = ptr;
//	strcpy(ptr,*buf);
	char * ptr = *buf;
	ptr = ptr + 1;
	*buf = ptr;
//	*buf = *buf + 1;
	
	*p = 1;
//	free(sy);
}
int main()
{
	int flag = 1;
	int a = 0;
	char  *p = malloc(16);
	char bbc[5] = "abcd";
	strcpy(p,"abc");
	func(&p,&a);
	func(&bbc,&a);
	printf("%s\n",p);
	printf("%s\n",bbc);
	printf("\u6211\u6ca1\u6709\u6210\u529f\u7406\u89e3\u60a8\u7684\u610f\u601d");
	printf("{\"msg\": \"\u6211\u6ca1\u6709\u6210\u529f\u7406\u89e3\u60a8\u7684\u610f\u601d\", \"data\": {}, \"retcode\": 0}");
}
