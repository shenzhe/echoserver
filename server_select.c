#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>


//绑定端口
#define PORT  9548

//读buff长度
#define BUFFLEN  255
char readBuf[BUFFLEN];

//client结构体
typedef struct _client 
{
	int fd;
	//struct sockaddr_in addr;
} client;

//最大连接数
#define MAX_CONN  100


int main()
{
	int listenfd, socketfd, nread;
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;
	struct timeval timeoutval;
	client clients[MAX_CONN];

	//创建一个ipv4的tcp socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(listenfd == -1) 
	{
		printf("socket error\n");
		return -1;
	} 

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
	{
		printf("bind %d error\n", PORT);
		return -1;
	}
	
	//backlog = 5
	listen(listenfd, 5);
	
	int i=0;	
	//初始化clients
	for(i=0; i< MAX_CONN; i++) 
	{
		clients[i].fd = -1;
	}
	
	//socketfd = accept(listenfd, NULL, NULL);
	
	//读写句柄数组
	fd_set readfds, writefds;
	int sockMax, ret;
	socklen_t addrlen;
	while (1)
	{
		FD_ZERO(&readfds);	
		FD_SET(listenfd, &readfds); //把server socket fd 加入到read监听
		sockMax = listenfd;
		for(i=0; i<MAX_CONN; i++)
		{
			if(clients[i].fd > 0)
			{
				FD_SET(clients[i].fd, &readfds);
				if(sockMax < clients[i].fd)
				{
					sockMax = clients[i].fd;
				}
			}
		}
			
		timeoutval.tv_sec = 3;
		timeoutval.tv_usec = 0;
		
		ret = select((int)sockMax+1, &readfds, NULL, NULL, &timeoutval);
		
		if(ret < 0)
		{
			printf("select error\n");
			return -1;
		} 
		else if(ret == 0)
		{
			printf("timeout...\n");
			continue;	
		}
		
		printf("select succ\n");
		
		//读取client发送的数据, select只能遍历所有的client
		for(i=0; i<MAX_CONN; i++)
		{
			if(clients[i].fd > 0 && FD_ISSET(clients[i].fd, &readfds))
			{
				nread = recv(clients[i].fd, readBuf, nread, 0);
				printf("read from fd=%d, data=%s, len=%d\n", clients[i].fd,readBuf, nread);
				send(clients[i].fd, readBuf, nread, 0);
			}
		}

		//listen socket有新连接
		if(FD_ISSET(listenfd, &readfds))
		{
			printf("new accept\n");
			//bzero(&clientaddr, sizeof(clientaddr));
			//socketfd = accept(listenfd, (struct sockaddr*) &clientaddr, NULL);
			socketfd = accept(listenfd, NULL, NULL);
			
			if(socketfd == -1)
			{
				printf("accept error\n");
				continue;
			}
			//找出空闲的client分配之 
			for(i=0; i<MAX_CONN; i++)
			{
				if(clients[i].fd < 0) {
					clients[i].fd = socketfd;
					//clients[i].addr = clientaddr;
					//printf("You got a connection from %s \n",inet_ntoa(clients[i].addr.sin_addr) );  
                    			break; 
				}
			}
		}		
	}

	return 0;
	
	
}
