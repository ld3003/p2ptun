#ifndef __MQTT_H__
#define __MQTT_H__

struct MQTTClientData {
	unsigned char conn;
};

typedef int (*MQTT_RECV_MSG)(char *from, char *msg);

int start_mqtt_client();
void p2psignal_subscribe(void *p);
int set_mqttrecv_callback(MQTT_RECV_MSG msgrecv);
int set_mqtt_clientid(char *clientid);
int send_p2psignal_msg(char *to, char *msg);
#endif
