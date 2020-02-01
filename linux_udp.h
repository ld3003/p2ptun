#ifndef __linux_udp_h__
#define __linux_udp_h__

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

typedef void (*UDP_RECV_PACKAGE)(struct sockaddr_in *addr, unsigned char *data, int len);
void create_udp_sock(short port, UDP_RECV_PACKAGE recvpkg_cb);
int send_linux_udp_data(struct sockaddr_in *addr, unsigned char *data, int len);
int sendudpmsg(char *msg, unsigned short localport);


#endif