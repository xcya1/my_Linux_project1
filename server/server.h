#ifndef __SERVER_H
#define __SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdio.h>


struct msgInfo
{
    struct sockaddr_in addr; // 保存通信对方的地址   sockaddr_in系统定义的结构体
};

struct localAddr
{
    in_port_t port;
    char szIP[16];
};


int  init_server(char *s_IP,u_int16_t prot);
void server_handl_eEvent(int epObj, void (*s_print)(char *,struct localAddr *,int),void(*pdeng_lu_client)(int));

#endif