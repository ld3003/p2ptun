#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p2ptun.h"
#include "MQTTLinux.h"
#include "mqtt.h"
#include "cJSON.h"
#include "linux_udp.h"

pthread_mutex_t mutex_lock;
struct P2PTUN_CONN_SESSION *p2psession;
short udp_port;
//callback:

void MessageArrived_Fun(void *pbuf, int len)
{
	pthread_mutex_lock(&mutex_lock);
	printf("data = %s\n", (unsigned char *)pbuf); //打印接收到的数据
	p2ptun_input_msg(p2psession, (char *)pbuf);
	pthread_mutex_unlock(&mutex_lock);
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
	//
}

int __senddata_func(unsigned char *data, int len, char pkgtype)
{
	struct sockaddr_in addr;

	switch (pkgtype)
	{
	case 0:
		addr.sin_family = AF_INET;
		addr.sin_port = htons(8883);
		addr.sin_addr.s_addr = inet_addr("47.93.103.232");
		return send_linux_udp_data(&addr, data, len);
		break;
	case 1:
		addr.sin_family = AF_INET;
		addr.sin_port = htons(8884);
		addr.sin_addr.s_addr = inet_addr("47.93.103.232");
		return send_linux_udp_data(&addr, data, len);
		break;

	case 3:
		addr.sin_family = AF_INET;
		addr.sin_port = htons(p2psession->remote_port);
		addr.sin_addr.s_addr = inet_addr(p2psession->remote_ipaddr);
		return send_linux_udp_data(&addr, data, len);
		break;
	}
}

//thread ：

void mqttthread(void *p)
{
	cloud_mqtt_thread((void *)MessageArrived_Fun);
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
	pthread_t mqttthreadid;
	pthread_t s1threadid;
	pthread_t udpthread;

	pthread_mutex_init(&mutex_lock, NULL);

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

	p2psession = p2ptun_alloc_session();
	p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
	p2psession->out_dat = __senddata_func;
	p2psession->out_msg = __send_msg;

	while ((ret = getopt(argc, argv, "sc")) != -1)
	{
		switch (ret)
		{
		case 's':
			printf("running in server\n");
			p2psession->workmode = P2PTUN_WORKMODE_SERVER;
			udp_port = 17788;
			break;
		case 'c':
			printf("running in client\n");
			p2psession->workmode = P2PTUN_WORKMODE_CLIENT;
			udp_port = 17789;
			break;
		default:
			return -1;
			break;
		}
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
