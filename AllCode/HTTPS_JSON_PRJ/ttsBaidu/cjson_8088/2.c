#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main()
{
	char * mm = "abcded";
	char * cc = "abcd";
	int a = strncmp(mm,cc,4);
	printf("%d\n",a);
	return 0;
}
