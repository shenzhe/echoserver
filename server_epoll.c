#include "server_epoll.h"

void setNonBlocking(int connfd) 
{
      int opts;
      opts=fcntl(connfd,F_GETFL);
      if(opts<0)
      {
          perror("fcntl(connfd,GETFL)");
          return;
      }
      opts = opts|O_NONBLOCK;
      if(fcntl(connfd,F_SETFL,opts)<0)
      {
          perror("fcntl(connfd,SETFL,opts)");
          return;
      }
 }

void closeAndRemove(int connfd, int n) 
{
    if(connfd > 0) 
    {
        epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &eventList[n]);
        printf("fd=%d closed=\n", connfd);
        close(connfd);
        removeFd(connfd);
        char w[100];
        sprintf(w,"%d 离开了聊天室!\n", connfd);
        sendAll(w, connfd, 1);
    }
}

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
        int ret;
	while(1)
	{
            timeout = 3000;

            ret = epoll_wait(epollfd, eventList, MAX_CONN, timeout);

            if(ret < 0) 
            {
                    printf("epoll wait error\n");
                    break;
            }
            else if(ret == 0)
            {
                    //printf("timeout\n");
                    continue;
            }

            int n = 0;

            for(n=0; n<ret; n++)
            {
                printf("1 event=%d\n", eventList[n].events);
//                if((eventList[n].events & EPOLLERR) ||
//                                (eventList[n].events & EPOLLHUP) ||
//                                !(eventList[n].events & EPOLLIN)||
//                                !(eventList[n].events & EPOLLOUT))
//                {
//                    close(eventList[n].data.fd);
//
//                    printf("2 event=%d\n", EPOLLERR);
//                    printf("3 event=%d\n", EPOLLHUP);
//                    printf("4 event=%d\n", EPOLLIN);
//                    printf("5 event=%d\n", EPOLLRDHUP);
//                    if(eventList[n].events & EPOLLRDHUP) {
//                       epoll_ctl(epollfd, EPOLL_CTL_DEL, eventList[n].data.fd, &eventList[n]);
//                       printf("fd=%d closed====\n", eventList[n].data.fd);
//                       continue;
//                    }      
//                    printf("epoll error\n");
//                    return -1;
//                }

                if(eventList[n].data.fd == listenfd)
                {   //新连接
                    acceptConn(listenfd);
                }
                else if(eventList[n].events & EPOLLIN) //收到数据 
                {
                        printf("6 event=%d\n", eventList[n].events);
                        printf("7 event=%d\n", EPOLLIN);
                        recvData(eventList[n].data.fd, n);
                }
                else if (eventList[n].events & EPOLLOUT) //发送数据
                {
                    printf("8 event=%d\n", eventList[n].events);
                    printf("9 event=%d\n", EPOLLOUT);
                    sendData(eventList[n].data.fd, n);
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
        
        setNonBlocking(connfd);
	struct epoll_event event;
	event.data.fd = connfd;
	event.events = EPOLLIN|EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
        addFd(connfd);
        char s[100];
        sprintf(s, "%s %d\n输入quit，退出聊天室\n", welcome, connfd);
        send(connfd, s, strlen(s), 0);
        char w[100];
        sprintf(w, "欢迎 %d 来到聊到室\n", connfd);
        sendAll(w, connfd, 1);
}

void recvData(int connfd, int n)
{
        printf("fd=%d has data\n", connfd);
	int ret;
        int offset = 0;
        int readOk = 0;
	memset(readBuf, 0, BUFLEN);
        while (1) 
        {
            ret = recv(connfd, readBuf + offset, BUFLEN, 0);
            printf("read from fd=%d, data=%s, len=%d\n", connfd,readBuf, ret);

            if(ret < 0) 
            {  //出错了
                  if(errno == EAGAIN) 
                  {
                      // 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
                      // 在这里就当作是该次事件已处理处.
                       readOk = 1;
                       break;
                  } 
                  else if (errno == ECONNRESET) 
                  {
                      closeAndRemove(connfd, n);
                      break ;
                  } 
                  else if (errno == EINTR) 
                  {
                      continue;
                  }
                  else 
                  {
                      closeAndRemove(connfd, n);
                      break ;
                  }
            }

            if(0 == ret) 
            { //close
                closeAndRemove(connfd, n);
                return;
            }
            offset += ret;
        }
        
        if(readOk) 
        {
            if(0 == strncasecmp(readBuf, "quit", 4)) { //退出
                closeAndRemove(connfd, n);
                return;
            }
            eventList[n].data.fd = connfd;
            eventList[n].events = EPOLLOUT | EPOLLET;
            epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &eventList[n]);
        }
        
        return;
}

void sendData(int connfd, int n)
{
    int ret;
    int offset = 0;
    int writeOk = 0;
    printf("read len:%d\n", (int)strlen(readBuf));
    char sendBuf[SENDLEN] = "server:";
//    printf("send start len:%d\n", (int)strlen(sendBuf));
//    strcat(sendBuf, readBuf);
//    printf("send cat len:%d\n", (int)strlen(sendBuf));
    sprintf(sendBuf, "%d说:%s\n", connfd, readBuf);
    sendAll(sendBuf, connfd, 1);
    while(1) 
    {
        ret = send(connfd, sendBuf + offset, (int)strlen(sendBuf)-offset, 0);
        printf("send to fd=%d, data=%s, len=%d\n", connfd, sendBuf, ret);
        if(ret < 0)
        {
            if(errno == EAGAIN)
            {
                writeOk = 1;
                break;
            }
            else if (errno == ECONNRESET)
            {
                closeAndRemove(connfd, n);
                break;
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else 
            {
                closeAndRemove(connfd, n);
                break;
            }
        }
        
        if(ret == 0)
        {
            closeAndRemove(connfd, n);
            break;
        }
        
        offset += ret;
        
        if(offset == strlen(sendBuf)) {
            writeOk = 1;
            break;
        }
         
    }
    
    if (writeOk)
    {
        eventList[n].data.fd = connfd;
        eventList[n].events = EPOLLIN | EPOLLET;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &eventList[n]);
    }
    
    return;
}

void addFd(int connfd)
{
    int i = 0;
    for(i=0; i<MAX_CONN; i++) 
    {
        if(!fds[i]) {
            fds[i] = connfd;
            return;
        }
    }
}

void removeFd(int connfd)
{
    int i = 0;
    for(i=0; i<MAX_CONN; i++) 
    {
        if(fds[i] == connfd) {
            fds[i] = 0;
            return;
        }
    }
}

void sendAll(char * data, int connfd, int flag)
{
    int i = 0;
    for(i=0; i<MAX_CONN; i++) 
    {
        if(fds[i]) {
            if(flag && connfd == fds[i]) {
                continue;
            }
            printf("send to %d: %s=%d\n", fds[i], data, strlen(data));
            send(fds[i], data, strlen(data), 0);
        }
    }
}