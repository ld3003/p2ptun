#ifndef __msgjson_h__
#define __msgjson_h__

/*

100:UDP PING
101:UDP PONG

102 :MQTTPING
8102:MQTTPONG

103 :MQTTGETNTYPE
8103:MQTTGETNTYPE RESP




*/

#define P2PTUN_CMD_UDPPING          100
#define P2PTUN_CMD_UDPPONG          101

#define P2PTUN_CMD_MQTTPING         102
#define P2PTUN_CMD_MQTTPONG         8102

#define P2PTUN_CMD_MQTTGETNTYPE     103

struct JSONDATA
{
    int cmd;
    int seq;
    char from[32];
    char to[32];
    char addr[32];
    int port;
    int ntype;
};

int json2data(char *json, struct JSONDATA *dat);
char *data2json(struct JSONDATA *dat);

#endif