#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#define fileName    "main"
#define execName    "./main"

int main(int argc, char *argv[])
{
    FILE   *    stream;
    FILE   *    stdOUT;
    char        buf[1024] = "";
    char        killCmd[50] = "";
    char        pidBuf[10] = "";
	int         pidArry[10] = {0};
    uint32_t    i;
    int         ret,j,k,n;

    while(1)
    {
        memset(buf,0,sizeof(buf));
		memset(pidArry,0,sizeof(pidArry));
        stream = popen( "ps -ef | grep "fileName" | grep -v grep | wc -l", "r" );
        fread( buf, sizeof(char), sizeof(buf), stream);
        pclose(stream);
        for(i=0;i<sizeof(buf);i++)
        {
            if((buf[i] < 49) || (buf[i] > 56 ))
            {
                buf[i] = 0;
            }
        }
        ret = atoi(buf);
        if(ret == 1)
        {
            usleep(100000);
            continue;
        }else{
			if(ret > 1)
			{
				n = 0;
				memset(pidBuf,0,10);
        		memset(buf,0,sizeof(buf));
        		stream = popen( "ps -ef|grep "fileName"|grep -v grep|grep -v PPID|awk '{ print $2}'", "r");
        		fread(buf, sizeof(char), sizeof(buf), stream);
        		pclose(stream);	
        		for(i=0;i<sizeof(buf);i++)
        		{
           			if((buf[i] > 47) && (buf[i] < 58))
            		{	
                		for(j=0,k=i;j < 10;j++,k++)
						{
							if(buf[k] == '\n' || buf[k] == '\0')	
							{
								memcpy(pidBuf,buf+i,j);
								pidBuf[j] = '\0';
								pidArry[n] = atoi(pidBuf);
								n++;
								i = i + j;
								break;
							}
						}
            		}else{
						buf[i] = 0;	
					}
        		}
				for(i=0;i < 10;i++)
				{
					if(pidArry[i] > 0)
					{
						memset(killCmd,0,50);
						sprintf(killCmd,"%s%d","kill -9 ",pidArry[i]);
						system("sync");
						system(killCmd);
					}
				}
			}
			if(ret == 0)
			{
				memset(killCmd,0,50);
				sprintf(killCmd,"%s%s",execName,"& ");
				system("sync");
				system(killCmd);
			}
		}
		usleep(100000);
	}
    return 0;
}
