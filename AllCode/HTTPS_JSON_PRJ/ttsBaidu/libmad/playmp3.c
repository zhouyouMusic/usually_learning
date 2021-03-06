#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int  id, fd, i;
    char buf[1024];
    int  rate;      /*simple rate 44.1KHz*/
    int  format;    /*quatize args*/
    int  channels;  /*sound channel*/

    if(argc != 2)
    {
        fprintf(stderr, "usage : %s \n", argv[0]);
        exit(-1);
    }

    if((fd = open(argv[1], O_RDONLY)) < 0)
    {
        fprintf(stderr, "Can't open sound file!\n");
        exit(-2);
    }

    if((id = open("/dev/dsp", O_WRONLY)) <  0)
    {
        write(id, buf, i);
        //printf("i=%d\n", i);
    }

    close(fd);
    close(id);

    exit(0);

}
