/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server_epoll.h
 * Author: shenzhe
 *
 * Created on 2015年12月14日, 下午8:45
 */

#ifndef SERVER_EPOLL_H
#define SERVER_EPOLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

//端口
#define PORT  9548

//最大连接数
#define MAX_CONN 100

#define BUFLEN 100
char readBuf[BUFLEN];
#define SENDLEN 120

int epollfd;
struct epoll_event eventList[MAX_CONN];
char *welcome = "欢迎来到桶哥聊天室!!\n 你的id是:";

int fds[MAX_CONN];


void acceptConn(int listenfd);
void recvData(int connfd, int n);
void sendData(int connfd, int n);
void setNonBlocking(int connfd);
void closeAndRemove(int connfd, int n);

void addFd(int connfd);
void removeFd(int connfd);
void sendAll(char *data, int connfd, int flag);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_EPOLL_H */

