#ifndef __MQTT_H__
#define __MQTT_H__

#define ADDRESS "tcp://easy-iot.cc:1883"  //更改此处地址
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME ""
#define PASSWORD ""

/*
topic 规则
IOTDEVICE/MEDIA/P2PCAM/{devid}
*/

struct MQTTClientData
{
    unsigned char conn;
};

typedef int (*MQTT_RECV_MSG)(char *from, char *msg);

int start_mqtt_client();
void mqtt_client_thread(void *p);

int set_mqtt_clientid(char *clientid);

int set_p2psignal_callback(MQTT_RECV_MSG msgrecv);
void p2psignal_subscribe(void *p);
int send_p2psignal_msg(char *to, char *msg);
#endif
