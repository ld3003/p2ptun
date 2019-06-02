#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p2ptun.h"
#include "cJSON.h"
#include "msg2json.h"
#include "linux_udp.h"

#define USE_LOCAL_TESTING 1

pthread_mutex_t mutex_lock;
struct P2PTUN_CONN_SESSION *p2psession;
short udp_port;
//callback:

#define SERVER_LOCAL_BIND_PORT 10087
#define CLIENT_LOCAL_BIND_PORT 10078

#define P2PTUNSRV_ADDR "47.93.103.232"
#define P2PTUNSRV_PORT_MSG 29001
#define P2PTUNSRV_PORT_ECHO1 (P2PTUNSRV_PORT_MSG + 1)
#define P2PTUNSRV_PORT_ECHO2 (P2PTUNSRV_PORT_MSG + 2)

int p2pdataArrived_Fun(unsigned char *data, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
	{
		//printf("%02x ", data[i]);
	}
	printf("\np2pdataArrived_Fun !!!\n");
	return 0;
}

int p2pdatakcpArrived_Fun(unsigned char *data, int len)
{
	int i = 0;
	printf("\n@@@@@@@@@@@@@@@@@@@RECV KCP !!!\n");
	for (i = 0; i < len; i++)
	{
		printf("%02x ", data[i]);
	}
	printf("\nRECV KCP !!!\n");
	return 0;
}

void udpArrived_Fun(struct sockaddr_in *addr, unsigned char *data, int len)
{
	pthread_mutex_lock(&mutex_lock);

	p2ptun_input_data(p2psession, data, len);

	pthread_mutex_unlock(&mutex_lock);
	return 0;
}

void __send_msg(char *msg)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(P2PTUNSRV_PORT_MSG);
	addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
	printf("MSGSND:%s\n", msg);
	return send_linux_udp_data(&addr, msg, strlen(msg));
}

int __senddata_func(unsigned char *data, int len, char pkgtype)
{

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	switch (pkgtype)
	{
	case P2PTUN_UDPPKG_TYPE_PING1: //获取公网AP-1包
		addr.sin_family = AF_INET;
		addr.sin_port = htons(P2PTUNSRV_PORT_ECHO1);
		addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
		break;
	case P2PTUN_UDPPKG_TYPE_PING2: //获取公网AP-2包 ，第二次主要用于探测是否处于对称网络
		addr.sin_family = AF_INET;
		addr.sin_port = htons(P2PTUNSRV_PORT_ECHO2);
		addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
		break;
	case P2PTUN_UDPPKG_TYPE_P2PRAWDATA:
	case P2PTUN_UDPPKG_TYPE_P2PMSG: //UDP MESSAGE 包 ，主要用于 P2P 之间的信令交互
		addr.sin_family = AF_INET;
		addr.sin_port = htons(p2psession->remote_port);
		addr.sin_addr.s_addr = inet_addr(p2psession->remote_ipaddr);

#if (USE_LOCAL_TESTING == 1)
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (p2psession->workmode == P2PTUN_WORKMODE_CLIENT)
		{
			addr.sin_port = htons(SERVER_LOCAL_BIND_PORT);
		}
		else
		{
			addr.sin_port = htons(CLIENT_LOCAL_BIND_PORT);
		}
#endif
		//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		printf("[%d] send to remote_ipaddr : %s:%d\n", pkgtype, p2psession->remote_ipaddr, p2psession->remote_port);
		break;

	case P2PTUN_UDPPKG_TYPE_RELAYMSG: //UDP MESSAGE 包 ，主要用于 服务器转发
		printf("p2ptun_output_msg : %s\n", data);
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(P2PTUNSRV_PORT_MSG);
		addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
		break;
	}
	return send_linux_udp_data(&addr, data, len);
}

//thread ：

void udp_recv_thread(void *p)
{
	create_udp_sock(udp_port, udpArrived_Fun);
}

void session_timer_thread(void *p)
{
	unsigned int count = 0;
	;
	struct P2PTUN_CONN_SESSION *session = (struct P2PTUN_CONN_SESSION *)p;
	for (;;)
	{
		int x;
		count++;
		pthread_mutex_lock(&mutex_lock);
		p2ptun_input_timer(session);
		pthread_mutex_unlock(&mutex_lock);
		usleep(1000);
	}
	//
}

int main(int argc, char **argv)
{
	int ret;

	pthread_t s1threadid;
	pthread_t udpthread;

	/*
	struct P2PTUN_TIME tm;
	p2ptun_get_current_time(&tm);
	usleep(1000);
	printf("CURRENT TIM %d\n",get_sub_tim_ms(&tm));
	return 0;
	*/

	p2psession = p2ptun_alloc_session();
	p2psession->workmode = P2PTUN_WORKMODE_CLIENT; /*<定义工作模式*/
	p2psession->out_dat = __senddata_func;		   /*<定义工作模式*/
	p2psession->out_p2pdat = p2pdataArrived_Fun;   /*<定义工作模式*/
	p2psession->out_p2pdat_kcp = p2pdatakcpArrived_Fun;

	/*根据运行输入参数 来决定 运行s端还是c端*/
	while ((ret = getopt(argc, argv, "sc")) != -1)
	{
		switch (ret)
		{
		case 's':
			printf("running in server\n");
			p2psession->workmode = P2PTUN_WORKMODE_SERVER;
			sprintf(p2psession->local_peername, "device_ser");
			udp_port = SERVER_LOCAL_BIND_PORT;
			break;
		case 'c':
			printf("running in client\n");
			p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
			sprintf(p2psession->remote_peername, "device_ser");
			sprintf(p2psession->local_peername, "device_cli");
			udp_port = CLIENT_LOCAL_BIND_PORT;
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

	printf("create session1_thread !\n");
	if ((pthread_create(&s1threadid, NULL, session_timer_thread, (void *)p2psession)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	for (;;)
	{
		int x;
		x = p2ptun_input_p2pdata_kcp(p2psession, "test!", 5);

		printf("p2ptun_input_p2pdata_kcp %d\n", x);
		if (x == 0)
			usleep(100);
		//sleep(1);
	}
	return 0;
}
