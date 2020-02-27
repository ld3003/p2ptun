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
#define ADDRESS "tcp://easy-iot.cc:1883"	   //更改此处地址
#define CLIENTID mqttclientid				   //更改此处客户端ID
//#define SUB_CLIENTID "aaabbbccc_sub"		   //更改此处客户端ID
#define P2PSIGNAL_TOPIC "p2p_ctrl_signal"	  //更改发送的话题
#define PAYLOAD "Hello Man, Can you see me ?!" //
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME ""
#define PASSWORD ""

#define DISCONNECT "out"

static MQTT_RECV_MSG mqttrecv = 0;
static char mqttclientid[128] = "NULL";

int CONNECT = 1;
volatile MQTTClient_deliveryToken deliveredtoken;

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
	printf("message: ");

	if (strstr(topicName, P2PSIGNAL_TOPIC))
	{
		if (mqttrecv)
		{
			mqttrecv(0, (char*)message->payload);
		}
	}

	payloadptr = message->payload;
	if (strcmp(payloadptr, DISCONNECT) == 0)
	{
		printf(" \n out!!");
		CONNECT = 0;
	}

	for (i = 0; i < message->payloadlen; i++)
	{
		putchar(*payloadptr++);
	}
	printf("\n");

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause)
{
	printf("\nConnection lost\n");
	printf(" cause: %s\n", cause);
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

static MQTTClient mqttclient;
static MQTTClient_deliveryToken mqtttoken;

int p2psignal_subscribe()
{
	char topicbuf[512];
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;
	int ch;

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
		exit(EXIT_FAILURE);
	}

	snprintf(topicbuf, sizeof(topicbuf), "%s/%s_sub", P2PSIGNAL_TOPIC, CLIENTID);
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
		   "Press Q<Enter> to quit\n\n",
		   topicbuf, CLIENTID, QOS);
	MQTTClient_subscribe(mqttclient, topicbuf, QOS);
}

int set_mqtt_clientid(char *clientid)
{
	snprintf(mqttclientid, sizeof(mqttclientid), "%s", clientid);
	return 0;
	///
}

int set_mqttrecv_callback(MQTT_RECV_MSG msgrecv)
{
	mqttrecv = msgrecv;
	return 0;
}
int send_p2psignal_msg(char *to, char *msg)
{
	int rc;
	char topicbuf[512];
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
	printf("Message with delivery token %d delivered %d\n", mqtttoken,rc);
	return rc;
}