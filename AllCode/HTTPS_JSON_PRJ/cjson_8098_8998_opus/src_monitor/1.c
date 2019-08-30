#include <stdio.h>
union temp
{
	short int a;
	char b;
}temp;

int main()
{
	temp.a = 0x1234;
	if(temp.b==0x12)
	{
		printf("big duan\n");

	}else{
		printf("small duan\n");
	}

}

