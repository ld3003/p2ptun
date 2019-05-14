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

#define P2PTUNSRV_ADDR "47.93.103.232"
#define P2PTUNSRV_PORT_MSG 29001
#define P2PTUNSRV_PORT_ECHO1 (P2PTUNSRV_PORT_MSG + 1)
#define P2PTUNSRV_PORT_ECHO2 (P2PTUNSRV_PORT_MSG + 2)

int p2pdataArrived_Fun(unsigned char *data, int len)
{
	printf("p2pdataArrived_Fun !!!\n");
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
		return send_linux_udp_data(&addr, data, len);
		break;
	case P2PTUN_UDPPKG_TYPE_PING2: //获取公网AP-2包 ，第二次主要用于探测是否处于对称网络
		addr.sin_family = AF_INET;
		addr.sin_port = htons(P2PTUNSRV_PORT_ECHO2);
		addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
		return send_linux_udp_data(&addr, data, len);
		break;

	case P2PTUN_UDPPKG_TYPE_P2PRAWDATA:
	case P2PTUN_UDPPKG_TYPE_P2PMSG: //UDP MESSAGE 包 ，主要用于 P2P 之间的信令交互
		addr.sin_family = AF_INET;
		addr.sin_port = htons(p2psession->remote_port);
		addr.sin_addr.s_addr = inet_addr(p2psession->remote_ipaddr);
		//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		printf("send to remote_ipaddr : %s:%d\n", p2psession->remote_ipaddr, p2psession->remote_port);
		return send_linux_udp_data(&addr, data, len);
		break;


	case P2PTUN_UDPPKG_TYPE_RELAYMSG: //UDP MESSAGE 包 ，主要用于 服务器转发

		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(P2PTUNSRV_PORT_MSG);
		addr.sin_addr.s_addr = inet_addr(P2PTUNSRV_ADDR);
		return send_linux_udp_data(&addr, data, len);
		break;
	}
}

//thread ：

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

	pthread_t s1threadid;
	pthread_t udpthread;

	p2psession = p2ptun_alloc_session();
	p2psession->workmode = P2PTUN_WORKMODE_CLIENT; /*<定义工作模式*/
	p2psession->out_dat = __senddata_func;		   /*<定义工作模式*/
	p2psession->out_p2pdat = p2pdataArrived_Fun;   /*<定义工作模式*/

	/*根据运行输入参数 来决定 运行s端还是c端*/
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
