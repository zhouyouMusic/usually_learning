#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

void List(char *path)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path);
	char allFiles[20][30]={""};
//	memset(allFiles,0,600);
	int i = 0;
    while (NULL != (ent=readdir(pDir)))
    {
#if 0
        if (ent->d_reclen==24)
        {    
            if (ent->d_type==8)
            {
                printf("general file [%s]\n", ent->d_name);
            }
        }
        else if(ent->d_reclen==16)
        {
            printf("[.]目录或隐藏[%s]\n",ent->d_name);
        }
        else
        {
        }
#endif
		strcpy(allFiles[i], ent->d_name);
		i++;
	}
	for(i=0;i < 20;i++)
		printf("%s\n",allFiles[i]);
	closedir(pDir);
}

int main(int argc, char *argv[])
{
     List(argv[1]);
     return 0;
}
