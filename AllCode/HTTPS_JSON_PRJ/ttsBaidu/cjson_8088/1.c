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
	printf("----------- %s\n",ptr);	
	while(*ptr != '\0')
	{
		**buf = *ptr;
		*ptr = *ptr + 1;
	}
	printf("----------- %s\n",ptr);	
//	*buf = *buf + 1;
	
	*p = 1;
//	free(sy);
}
int main()
{
	int a = 0;
	char  *p = malloc(16);
	strcpy(p,"abc");
	func(&p,&a);
	printf("%s\n",p);
	printf("%d\n",a);
}
