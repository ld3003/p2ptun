#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "mqtt.h"

#define NUM_THREADS 2
#define ADDRESS "tcp://easy-iot.cc:1883" //更改此处地址
#define CLIENTID mqttclientid			 //更改此处客户端ID
//#define SUB_CLIENTID "aaabbbccc_sub"		   //更改此处客户端ID
#define P2PSIGNAL_TOPIC "p2p_ctrl_signal"	  //更改发送的话题
#define PAYLOAD "Hello Man, Can you see me ?!" //
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME ""
#define PASSWORD ""

#define DISCONNECT "out"

//成员变量



volatile MQTTClient_deliveryToken deliveredtoken;
static MQTTClient mqttclient;
static MQTTClient_deliveryToken mqtttoken;

static struct MQTTClientData mqttcd;
static MQTT_RECV_MSG p2psignal_cb = 0;
static char mqttclientid[128] = "NULL";

pthread_t mqttthread;
static pthread_mutex_t mqttcMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mqttcCond = PTHREAD_COND_INITIALIZER;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	int i;
	char *payloadptr;

	printf("Message arrived\n");
	printf("topic: %s\n", topicName);

	if (strstr(topicName, P2PSIGNAL_TOPIC))
	{
		if (p2psignal_cb)
		{
			p2psignal_cb(0, (char *)message->payload);
		}
	}

	if (strcmp(message->payload, DISCONNECT) == 0)
	{
		printf(" \n out!!");
	}

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause)
{
	pthread_mutex_lock(&mqttcMutex);
	printf("\nConnection lost\n");
	printf(" cause: %s\n", cause);
	mqttcd.conn = 0;
	pthread_cond_signal(&mqttcCond);
	pthread_mutex_unlock(&mqttcMutex);
}

int start_mqtt_client()
{
	if ((pthread_create(&mqttthread, NULL, mqtt_client_thread, (void *)NULL)) == -1)
	{
		printf("create error !\n");
		return -1;
	}
	return 0;
}

void mqtt_client_thread(void *p)
{
	char topicbuf[512];
	int rc;
	int ch;

	mqttcd.conn = 0;

CONNECT:

	pthread_mutex_lock(&mqttcMutex);
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	MQTTClient_create(&mqttclient, ADDRESS, CLIENTID,
					  MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = USERNAME;
	conn_opts.password = PASSWORD;

	MQTTClient_setCallbacks(mqttclient, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(mqttclient, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		mqttcd.conn = 0;
		goto RECONNECT;
	}
	else
	{
		mqttcd.conn = 1;
	}

	//sub
	snprintf(topicbuf, sizeof(topicbuf), "%s/%s_sub", P2PSIGNAL_TOPIC, CLIENTID);
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n",
		   topicbuf, CLIENTID, QOS);
	MQTTClient_subscribe(mqttclient, topicbuf, QOS);

	//等待出现断开事件
	pthread_cond_wait(&mqttcCond, &mqttcMutex);
	if (mqttcd.conn == 0)
	{
	}

RECONNECT:
	MQTTClient_disconnect(mqttclient, 10000);
	MQTTClient_destroy(&mqttclient);
	sleep(3);
	pthread_mutex_unlock(&mqttcMutex);
	goto CONNECT;
}

int set_mqtt_clientid(char *clientid)
{
	snprintf(mqttclientid, sizeof(mqttclientid), "%s", clientid);
	return 0;
	///
}

int set_p2psignal_callback(MQTT_RECV_MSG msgrecv)
{
	p2psignal_cb = msgrecv;
	return 0;
}
int send_p2psignal_msg(char *to, char *msg)
{
	int rc;
	char topicbuf[512];
	pthread_mutex_lock(&mqttcMutex);
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = msg;
	pubmsg.payloadlen = strlen(msg);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	snprintf(topicbuf, sizeof(topicbuf), "%s/%s_sub", P2PSIGNAL_TOPIC, to);
	MQTTClient_publishMessage(mqttclient, topicbuf, &pubmsg, &mqtttoken);
	printf("Waiting for up to %d seconds for publication of %s\n"
		   "on topic %s for client with ClientID: %s MSG:%s\n",
		   (int)(TIMEOUT / 1000), PAYLOAD, topicbuf, CLIENTID, msg);
	rc = MQTTClient_waitForCompletion(mqttclient, mqtttoken, TIMEOUT);
	printf("Message with delivery token %d delivered %d\n", mqtttoken, rc);
	pthread_mutex_unlock(&mqttcMutex);
	return rc;
}

void *subClient(void *threadid)
{
#if 0
	long tid;
	tid = (long)threadid;
	printf("Hello World! It's me, thread #%ld!\n", tid);

	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;
	int ch;

	MQTTClient_create(&client, ADDRESS, SUB_CLIENTID,
					  MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = USERNAME;
	conn_opts.password = PASSWORD;

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
		   "Press Q<Enter> to quit\n\n",
		   TOPIC, CLIENTID, QOS);
	MQTTClient_subscribe(client, TOPIC, QOS);

	do
	{
		ch = getchar();
	} while (ch != 'Q' && ch != 'q');

	MQTTClient_unsubscribe(client, TOPIC);
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	pthread_exit(NULL);
#endif
}
void *pubClient(void *threadid)
{
#if 0
	long tid;
	tid = (long)threadid;
	int count = 0;
	printf("Hello World! It's me, thread #%ld!\n", tid);
	//声明一个MQTTClient
	MQTTClient client;
	//初始化MQTT Client选项
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	//#define MQTTClient_message_initializer { {'M', 'Q', 'T', 'M'}, 0, 0, NULL, 0, 0, 0, 0 }
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	//声明消息token
	MQTTClient_deliveryToken token;
	int rc;
	//使用参数创建一个client，并将其赋值给之前声明的client
	MQTTClient_create(&client, ADDRESS, CLIENTID,
					  MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = USERNAME;
	conn_opts.password = PASSWORD;
	//使用MQTTClient_connect将client连接到服务器，使用指定的连接选项。成功则返回MQTTCLIENT_SUCCESS
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	pubmsg.payload = PAYLOAD;
	pubmsg.payloadlen = strlen(PAYLOAD);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	while (CONNECT)
	{
		MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
		printf("Waiting for up to %d seconds for publication of %s\n"
			   "on topic %s for client with ClientID: %s\n",
			   (int)(TIMEOUT / 1000), PAYLOAD, TOPIC, CLIENTID);
		rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
		printf("Message with delivery token %d delivered\n", token);
		usleep(3000000L);
	}

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
#endif
}