#ifndef __msgjson_h__
#define __msgjson_h__

char *ping(char *to , char *from);
char *pong(char *to , char *from);

char *get_net_type(char *to , char *from);
char *resp_net_type(char *to , char *from , int nettype , char *ip , int port);

char *request_udp(char *to , char *from , char *ip , int port);

char *notify_connected(char *to , char *from);

struct JSONDATA {
    int cmd;
    int seq;
    char addr[32];
    int port;
    int ntype;
};

int json2data(char *json , struct JSONDATA *dat);
char* data2json(struct JSONDATA *dat);

#endif