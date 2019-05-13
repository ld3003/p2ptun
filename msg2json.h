#ifndef __msgjson_h__
#define __msgjson_h__

/*

100:UDP PING
101:UDP PONG

102 :MSGPING
8102:MSGPONG

103 :MSGGETNTYPE
8103:MSGGETNTYPE RESP




*/

#define P2PTUN_CMD_UDPPING 100
#define P2PTUN_CMD_UDPPONG 101

#define P2PTUN_CMD_MSGPING 102
#define P2PTUN_CMD_MSGPONG 8102

#define P2PTUN_CMD_MSGGETNTYPE 103
#define P2PTUN_CMD_MSGRRESPNTYPE 8103

#define P2PTUN_CMD_UDP_TEST 104
#define P2PTUN_CMD_UDP_RESPTEST 8104

#define P2PTUN_CMD_MSG_REQUESTUDPTEST 105
#define P2PTUN_CMD_MSG_RESPONSEUDPTEST 8105

#define P2PTUN_CMD_MSG_CONNECTED 106
#define P2PTUN_CMD_MSG_RESPCONNECTED 8106



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