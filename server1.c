#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT  9538
#define BUFFLEN  255

int main()
{
	int count=0;
	int listenfd, socketfd, nread;
	struct sockaddr_in servaddr;
	struct timeval timeoutval;
	char readbuf[BUFFLEN];

	//创建一个ipv4的tcp socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(listenfd == -1) {
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
	
	socketfd = accept(listenfd, NULL, NULL);
	
	while (1)
	{
		printf("start recive %d... \n", count++);
		memset(readbuf, sizeof(readbuf), 0);
		
		nread = recv(socketfd, readbuf, BUFFLEN-1, 0);
		
		if(nread >0)
		{
			readbuf[nread] = '\0';
			printf("received %s, len=%d\nn", readbuf, nread);
			send(socketfd, readbuf, nread, 0);
		}
		
	}

	return 0;
	
	
}
