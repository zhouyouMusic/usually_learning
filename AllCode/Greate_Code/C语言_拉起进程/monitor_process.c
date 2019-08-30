#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#define fileName        "/opt/hello"
#define fileNameOld     fileName"old"
#define fileNameNew     fileName"new"

int main(int argc, char *argv[])
{
    FILE   *    stream;
    char        buf[1024];
    uint32_t    i;
    int         ret;
    uint8_t     bValidOld = 0;
    uint8_t     bValid    = 0;
    uint8_t     bValidNew = 0;

    if(argc > 1)
    {
        if(argv[1][0] == 'v')
        {
            printf("20160122\n");
            return 0;
        }
    }
    printf("app deamon running\n");

    while(1)
    {
        memset(buf,0,sizeof(buf));
        stream = popen( "ps -ef | grep "fileName" | grep -v grep | wc -l", "r" );
        fread( buf, sizeof(char), sizeof(buf), stream);
        pclose(stream);

        for(i=0;i<sizeof(buf);i++)
        {
            if((buf[i] <= '0') || ('9' >= buf[i]))
            {
                buf[i] = 0;
                break;
            }
        }

        ret = atoi(buf);
        printf("ret = %d\n",ret);

        if(ret)
        {
            sleep(2);
            continue;
        }

        printf("dest app isn't running , now check it\n");

        bValidOld = 0;
        bValid    = 0;
        bValidNew = 0;

        if(0 == access(fileNameNew,F_OK))
        {
            printf("found "fileNameNew);

            system("chmod +x "fileNameNew);

            memset(buf,0,sizeof(buf));
            stream = popen( "file "fileNameNew, "r");
            fread( buf, sizeof(char), sizeof(buf), stream);
            pclose(stream);

            if(strstr(buf,"executable"))
            {
                printf(" and it is executable \n");
                bValidNew = 1;
            }
            else
            {
                printf(" but it isn't executable , remove it\n");
                remove(fileNameNew);
            }
        }
        if(0 == access(fileName,F_OK))
        {
            printf("found "fileName);
            system("chmod +x "fileName);

            memset(buf,0,sizeof(buf));
            stream = popen( "file "fileName, "r");
            fread( buf, sizeof(char), sizeof(buf), stream);
            pclose(stream);

            if(strstr(buf,"executable"))
            {
                printf(" and it is executable \n");
                bValid = 1;
            }
            else
            {
                printf(" but it isn't executable , remove it\n");
                remove(fileName);
            }
        }
        if(0 == access(fileNameOld,F_OK))
        {
            printf("found "fileNameOld);
            system("chmod +x "fileNameOld);

            memset(buf,0,sizeof(buf));
            stream = popen( "file "fileNameOld, "r");
            fread( buf, sizeof(char), sizeof(buf), stream);
            pclose(stream);

            if(strstr(buf,"executable"))
            {
                printf(" and it is executable \n");
                bValidOld = 1;
            }
            else
            {
                printf(" but it isn't executable , remove it\n");
                remove(fileNameOld);
            }
        }

        if(bValidNew)
        {
            if(bValid)
            {
                if(bValidOld)
                {
                    printf("remove old\n");
                    remove(fileNameOld);
                }
                printf("rename current to old\n");
                rename(fileName,fileNameOld);
            }
            printf("rename new to current\n");
            rename(fileNameNew,fileName);
        }
        else
        {
            if(bValid)
            {
                if(bValidOld)
                {
                    printf("current is valid , remove old\n");
                    remove(fileNameOld);
                }
            }
            else
            {
                if(bValidOld)
                {
                    printf("rename old to current\n");
                    rename(fileNameOld,fileName);
                }
            }
        }
        system("sync");
        system(fileName);
        sleep(2);
    }
    return 0;
}
