#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    printf("millisecond:%ld\n",tv.tv_sec*1000 + tv.tv_usec/1000);  //毫秒
    return 0;
}
