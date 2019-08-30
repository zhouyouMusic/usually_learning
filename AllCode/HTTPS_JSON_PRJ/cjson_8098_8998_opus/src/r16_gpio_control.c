#include<stdio.h>
#include<stdint.h>
#include<sys/mman.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<unistd.h>
#include<string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <paths.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <libgen.h> 
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <mntent.h>
#include <sys/statfs.h>
#include <utmpx.h>
#include <utmp.h>
#include <utmp.h>
#include <utmpx.h>
#include <locale.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/****************** GPIO D control Addr 0x01C2086c    配置PD2/3口 值为0x77771177   **********/
/****************** GPIO D Data Addr  0x01c2087c	  配置PD2/3口 值为0xf	*********/


static unsigned long long handle_errors(unsigned long long v, char **endp)
{
	 char next_ch = **endp;
	 if (next_ch) {
			 return -1;
	 }
	 return v;
}
 
 
unsigned long long bb_strtoull(const char *arg, char **endp, int base)
{
	 unsigned long long v;
	 char *endptr;
 
	 if (!endp) endp = &endptr;
	 *endp = (char*) arg;
	 v = strtoull(arg, endp, base);
	 return handle_errors(v, endp);
}

int  xopen3(const char *pathname, int flags, int mode)
{
	int ret;

	ret = open(pathname, flags, mode);
	if (ret < 0) {
		printf("can't open '%s'", pathname);
	}
	return ret;
}
 
int  xopen(const char *pathname, int flags)
{
 	return xopen3(pathname, flags, 0666);
}


int periControl(unsigned width,uint64_t writeval,const char *chTag)
{
	void *map_base, *virt_addr;
	off_t target;
	unsigned page_size, mapped_size, offset_in_page;
	int fd;
	target = bb_strtoull(chTag, NULL, 0); 
	fd = xopen("/dev/mem", O_RDWR | O_SYNC);
	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + width > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size *= 2;
	}
	map_base = mmap(NULL,
			mapped_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED)
		printf("mmap failed");

	virt_addr = (char*)map_base + offset_in_page;
	switch (width) {
		case 8:
			*(volatile uint8_t*)virt_addr = writeval;
			break;
		case 16:
			*(volatile uint16_t*)virt_addr = writeval;
			break;
		case 32:
			*(volatile uint32_t*)virt_addr = writeval;
			break;
		case 64:
			*(volatile uint64_t*)virt_addr = writeval;
			break;
		default:
			break;
	}
	if(munmap(map_base, mapped_size) == -1)
		printf("munmap failed\n");
	close(fd);
}

