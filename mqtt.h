#ifndef __MQTT_H__
#define __MQTT_H__


typedef int (*MQTT_RECV_MSG)(char *from, char *msg);

int p2psignal_subscribe();
int set_mqttrecv_callback(MQTT_RECV_MSG msgrecv);
int set_mqtt_clientid(char *clientid);
int send_p2psignal_msg(char *to, char *msg);
#endif
