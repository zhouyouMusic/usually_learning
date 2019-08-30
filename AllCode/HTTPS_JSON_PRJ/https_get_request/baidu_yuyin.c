#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int https_get_request(char *ip,char *name,int portnumber,char *get_str)
{
	int sockfd=0;
	int ret;
	int  nbytes;
	char host_addr[256];
	char request[1024];
	int send, totalsend;
	SSL *ssl;
	SSL_CTX *ctx;
	char server_ip[16]={0};
	struct hostent *host;
	struct in_addr addr;
    struct sockaddr_in    servaddr;

	if(ip==NULL||strlen(ip)<7)
	{
		if((host=gethostbyname(name)) == NULL)
		{
			printf("gethostbyname fail..............\n");
			return -1;
		}
		memcpy(&addr.s_addr,host->h_addr_list[0],sizeof(addr.s_addr));
		strcpy(server_ip,(char *)inet_ntoa(addr));
		//printf("gethostbyname %s ==> %s ..............\n",name,server_ip);
	}
	else
	{
		strcpy(server_ip,ip);
	}
	printf("%s\n",server_ip);
	/* 客户程序开始建立 sockfd描述符 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{ 	   /*建立SOCKET连接 */
		printf("socket fail..............\n");
		return -1;
	}

	/* 客户程序填充服务端的资料 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portnumber);
    if(inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n",server_ip);
        close(sockfd);
		return -1;
    }

	/* 客户程序发起连接请求 */
	if (connect(sockfd, (struct sockaddr *) (&servaddr), sizeof(struct sockaddr)) == -1)
	{		 
		printf( "Connect Error:%s\a\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	/* SSL初始化 */
	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		close(sockfd);
		return -1;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		close(sockfd);
		return -1;
	}

	/* 把socket和SSL关联 */
	ret = SSL_set_fd(ssl, sockfd);
	if (ret == 0)
	{
		close(sockfd);
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	ret = SSL_connect(ssl);
	if (ret != 1)
	{
		close(sockfd);
		return -1;
	}

	sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\n\
		User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
		Host: %s:%d\r\nConnection: Close\r\n\r\n", get_str, host_addr,portnumber);


	//printf( "%s", request);		  /*准备request，将要发送给主机 */
    printf("request======>:\t\t[%s]\n",get_str);

	/*发送https请求request */
	send = 0;
	totalsend = 0;
	nbytes = strlen(request);
	while (totalsend < nbytes) 
	{
		send = SSL_write(ssl, request + totalsend, nbytes - totalsend);
		if (send == -1) 
		{
			close(sockfd);
			return -1;
		}
		totalsend += send;
		//printf("%d bytes send OK!\n", totalsend);
	}
	
	/* 结束通讯 */
	SSL_shutdown(ssl);
	close(sockfd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();

	return 0;
}

int https_get_request_wait_rsp(char *ip,char *name,int portnumber,char *get_str,char *rsp_str,int rsp_buf_len)
{
	int sockfd=0;
	int ret;
	char buffer[1024*400];
	int  nbytes;
	char host_addr[256];
	char request[1024];
	int send, totalsend;
	int i;
	SSL *ssl;
	SSL_CTX *ctx;
	char server_ip[16]={0};
	struct hostent *host;
	struct in_addr addr;
    struct sockaddr_in    servaddr;

	if(ip==NULL||strlen(ip)<7)
	{
		if((host=gethostbyname(name)) == NULL)
		{
			printf("gethostbyname fail..............\n");
			return -1;
		}
		memcpy(&addr.s_addr,host->h_addr_list[0],sizeof(addr.s_addr));
		strcpy(server_ip,(char *)inet_ntoa(addr));
		//printf("gethostbyname %s ==> %s ..............\n",name,server_ip);
	}
	else
	{
		strcpy(server_ip,ip);
	}

	/* 客户程序开始建立 sockfd描述符 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{ 	   /*建立SOCKET连接 */
		printf("socket fail..............\n");
		return -1;
	}

	/* 客户程序填充服务端的资料 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portnumber);
    if(inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n",server_ip);
        close(sockfd);
		return -1;
    }

	/* 客户程序发起连接请求 */
	if (connect(sockfd, (struct sockaddr *) (&servaddr), sizeof(struct sockaddr)) == -1)
	{		 
		printf( "Connect Error:%s\a\n", strerror(errno));
		close(sockfd);
		return -1;
	}

	/* SSL初始化 */
	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		close(sockfd);
		return -1;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		close(sockfd);
		return -1;
	}

	/* 把socket和SSL关联 */
	ret = SSL_set_fd(ssl, sockfd);
	if (ret == 0)
	{
		close(sockfd);
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	ret = SSL_connect(ssl);
	if (ret != 1)
	{
		close(sockfd);
		return -1;
	}

	sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\n\
User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
Host: %s:%d\r\nConnection: Close\r\n\r\n", get_str, host_addr,portnumber);


	//printf( "%s", request);		  /*准备request，将要发送给主机 */
    printf("request======>:\t\t[%s]\n",get_str);

	/*发送https请求request */
	send = 0;
	totalsend = 0;
	nbytes = strlen(request);
	while (totalsend < nbytes) 
	{
		send = SSL_write(ssl, request + totalsend, nbytes - totalsend);
		if (send == -1) 
		{
			close(sockfd);
			return -1;
		}
		totalsend += send;
		//printf("%d bytes send OK!\n", totalsend);
	}

	//printf("\nThe following is the response header:\n");
	
	i = 0;
	/* 连接成功了，接收https响应，response */
	while((nbytes = SSL_read(ssl, buffer, 1)) == 1) 
	{
		if(i < 4) 
		{
			if (buffer[0] == '\r' || buffer[0] == '\n')
			{
				i++;
				if(i>=4)
				{
					break;
				}
			}
			else
			{
				i = 0;
			}	
			//printf("%c", buffer[0]);		/*把https头信息打印在屏幕上 */
		} 
	}
	memset(rsp_str,0,rsp_buf_len);
	ret = SSL_read(ssl, rsp_str, rsp_buf_len);
	if(ret<0)
	{
		printf("response ret =%d=============\n",ret);
		close(sockfd);
		return -1;
	}
	printf("response ret =%d=============>\t\t[%s]\n\n",ret,rsp_str); 

	/* 结束通讯 */
	SSL_shutdown(ssl);
	close(sockfd);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();

	return 0;
}





int main()
{
    char rsp_buf[2048]={0};
    
    //只发请求  不需要等待回复
//    https_get_request("ipx.xxx.xxx.xx",NULL,80,"xxxx/xxx.req?aa=21");         //替换成自己需要的HTTP请求       
    //发请求  不需要等待回复
//    https_get_request_wait_rsp("ipx.xxx.xxx.xx",NULL,80,"xxxx/xxx.req?aa=21",rsp_buf,sizeof(rsp_buf));
    
    //用域名
//    https_get_request_wait_rsp(NULL,"www.baidu.com",8080,"xxxx/xxx.req?aa=21",rsp_buf,sizeof(rsp_buf));
    https_get_request(NULL,"www.baidu.com",8080,"xxxx/xxx.req?aa=21");
    
    printf("=============>\n");
    
	return 0;
}
