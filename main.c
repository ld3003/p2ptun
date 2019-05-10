#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p2ptun.h"
#include "MQTTLinux.h"
#include "mqtt.h"
#include "cJSON.h"
#include "linux_udp.h"

void MessageArrived_Fun(void *pbuf, int len)
{
	printf("data = %s\n", (unsigned char *)pbuf); //打印接收到的数据
}

void udpArrived_Fun(struct sockaddr_in *addr, unsigned char *data, int len)
{
	return 0;
}

void mqttthread(void *p)
{
	cloud_mqtt_thread((void *)MessageArrived_Fun);
}

void session_thread(void *p)
{
	struct P2PTUN_CONN_SESSION *session = (struct P2PTUN_CONN_SESSION *)p;
	for (;;)
	{
		p2ptun_mainloop(session);
	}
	//
}

void udp_recv_thread(void *p)
{
	create_udp_sock(17789, udpArrived_Fun);
}

int main(int argc, char **argv)
{
	int ret;
	pthread_t mqttthreadid;
	pthread_t s1threadid;
	pthread_t udpthread;

	if ((pthread_create(&mqttthreadid, NULL, mqttthread, (void *)NULL)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	if ((pthread_create(&udpthread, NULL, udp_recv_thread, (void *)NULL)) == -1)
	{
		printf("create error !\n");
		return -1;
	}

	struct P2PTUN_CONN_SESSION *p2psession = p2ptun_alloc_session();
	p2psession->workmode = P2PTUN_WORKMODE_CLIENT;

	while ((ret = getopt(argc, argv, "sc")) != -1)
	{
		switch (ret)
		{
		case 's':
			printf("running in server\n");
			p2psession->workmode = P2PTUN_WORKMODE_SERVER;
			break;
		case 'c':
			printf("running in client\n");
			p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
			break;
		default:
			return -1;
			break;
		}
	}

	printf("create session1_thread !\n");
	if ((pthread_create(&s1threadid, NULL, session_thread, (void *)p2psession)) == -1)
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
