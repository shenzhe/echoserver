#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/time.h>

//端口
#define PORT  9548

//最大连接数
#define MAX_CONN 100

#define BUFLEN 10
char readBuf[BUFLEN];

int epollfd;
struct epoll_event eventList[MAX_CONN];

static void acceptConn(int listenfd);
static void recvData(int connfd);

int main()
{

	int listenfd;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0) 
	{
		printf("socket error\n");
		return -1;
	} 

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("bind %d error\n", PORT);
		return -1;
	}

	listen(listenfd, 5);

	epollfd = epoll_create(MAX_CONN);     

	struct epoll_event event;
	event.events = EPOLLIN|EPOLLET;
	event.data.fd = listenfd;    

	//listenfd 加入 epoll监听
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0) 
	{
		printf("epoll add error: fd=%d\n", listenfd);
		return -1;
	}

	int timeout; //超时
	struct epoll_event eventList[MAX_CONN];
	int ret;
	while(1)
	{
		timeout = 3000;

		int ret = epoll_wait(epollfd, eventList, MAX_CONN, timeout);

		if(ret < 0) 
		{
			printf("epoll wait error\n");
			break;
		}
		else if(ret == 0)
		{
			printf("timeout\n");
			continue;
		}

		int n = 0;

		for(n=0; n<ret; n++)
		{
			if((eventList[n].events & EPOLLERR) ||
					(eventList[n].events & EPOLLHUP) ||
					!(eventList[n].events & EPOLLIN))
			{
				printf("epoll error\n");
				close(eventList[n].data.fd);
				return -1;
			}

			if(eventList[n].data.fd == listenfd)
			{   //新连接
				acceptConn(listenfd);
			}
			else //收到数据 
			{
				recvData(eventList[n].data.fd);
			}
		}


	}
	close(epollfd);
	close(listenfd);
	printf("server close");
	return 0;
}
void acceptConn(int listenfd)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(struct sockaddr_in);

	bzero(&sin, len);

	int connfd = accept(listenfd, (struct sockaddr*)&sin, &len);

	if(connfd < 0)
	{
		printf("accept error\n");
		return;
	}
	else
	{
		printf("new conn, fd=%d\n", connfd);
	}

	struct epoll_event event;
	event.data.fd = connfd;
	event.events = EPOLLIN|EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
}

void recvData(int connfd)
{
        printf("fd=%d has data\n", connfd);
	int ret;
	memset(readBuf, 0, BUFLEN);

	ret = recv(connfd, readBuf, ret, 0);
	printf("read from fd=%d, data=%s, len=%d\n", connfd,readBuf, ret);
        
	send(connfd, readBuf, ret, 0);
}


