#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p2ptun.h"
#include "cJSON.h"
#include "msg2json.h"
#include "linux_udp.h"

pthread_mutex_t mutex_lock;
struct P2PTUN_CONN_SESSION *p2psession;
short udp_port;
//callback:

#define HOST_ADDR "47.93.103.232"
#define HOST_PORT_MSG 29001
#define HOST_PORT_ECHO1 HOST_PORT_MSG+1
#define HOST_PORT_ECHO2 HOST_PORT_MSG+2

int p2pdataArrived_Fun(unsigned char *data, int len)
{
	printf("p2pdataArrived_Fun !!!\n");
	return 0;
}

void udpArrived_Fun(struct sockaddr_in *addr, unsigned char *data, int len)
{
	pthread_mutex_lock(&mutex_lock);

	p2ptun_input_data(p2psession,data,len);

	pthread_mutex_unlock(&mutex_lock);
	return 0;
}

void __send_msg(char *msg)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT_MSG);
	addr.sin_addr.s_addr = inet_addr(HOST_ADDR);
	printf("MSGSND:%s\n", msg);
	return send_linux_udp_data(&addr, msg, strlen(msg));
}

int __senddata_func(unsigned char *data, int len, char pkgtype)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	switch (pkgtype)
	{
	case 0: //获取公网AP-1包
		addr.sin_family = AF_INET;
		addr.sin_port = htons(HOST_PORT_ECHO1);
		addr.sin_addr.s_addr = inet_addr(HOST_ADDR);
		return send_linux_udp_data(&addr, data, len);
		break;
	case 1: //获取公网AP-2包 ，第二次主要用于探测是否处于对称网络
		addr.sin_family = AF_INET;
		addr.sin_port = htons(HOST_PORT_ECHO2);
		addr.sin_addr.s_addr = inet_addr(HOST_ADDR);
		return send_linux_udp_data(&addr, data, len);
		break;

	case 2: //UDP MESSAGE 包 ，主要用于 P2P 之间的信令交互
		addr.sin_family = AF_INET;
		addr.sin_port = htons(p2psession->remote_port);
		addr.sin_addr.s_addr = inet_addr(p2psession->remote_ipaddr);
		//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		printf("send to remote_ipaddr : %s:%d\n", p2psession->remote_ipaddr,p2psession->remote_port);
		return send_linux_udp_data(&addr, data, len);
		break;

	}
}

//thread ：

void msgthread(void *p)
{
	char tmp[128];
	sleep(3);
	for (;;)
	{

		#define MSGSTR "\{\"from\":\"%s\"\,\"to\":\"%s\"\}"
		snprintf(tmp, 64, MSGSTR, p2psession->local_peername, p2psession->local_peername);
		__send_msg(tmp);
		sleep(5);
	}
}

void udp_recv_thread(void *p)
{
	create_udp_sock(udp_port, udpArrived_Fun);
}

void session_timer_thread(void *p)
{
	struct P2PTUN_CONN_SESSION *session = (struct P2PTUN_CONN_SESSION *)p;
	for (;;)
	{
		pthread_mutex_lock(&mutex_lock);
		p2ptun_input_timer(session);
		pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}
	//
}

int main(int argc, char **argv)
{
	int ret;
	pthread_t msgthreadid;
	pthread_t s1threadid;
	pthread_t udpthread;

	p2psession = p2ptun_alloc_session();
	p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
	p2psession->out_dat = __senddata_func;
	p2psession->out_msg = __send_msg;
	p2psession->out_p2pdat = p2pdataArrived_Fun;

	while ((ret = getopt(argc, argv, "sc")) != -1)
	{
		switch (ret)
		{
		case 's':
			printf("running in server\n");
			p2psession->workmode = P2PTUN_WORKMODE_SERVER;
			sprintf(p2psession->local_peername, "device_ser");
			udp_port = 10088;
			break;
		case 'c':
			printf("running in client\n");
			p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
			sprintf(p2psession->remote_peername, "device_ser");
			sprintf(p2psession->local_peername, "device_cli");
			udp_port = 10099;
			break;
		default:
			return -1;
			break;
		}
	}

	pthread_mutex_init(&mutex_lock, NULL);

	if ((pthread_create(&udpthread, NULL, udp_recv_thread, (void *)NULL)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	if ((pthread_create(&msgthreadid, NULL, msgthread, (void *)NULL)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	printf("create session1_thread !\n");
	if ((pthread_create(&s1threadid, NULL, session_timer_thread, (void *)p2psession)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	for (;;)
	{
		sleep(1);
	}
	return 0;
}
