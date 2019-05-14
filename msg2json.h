#ifndef __msgjson_h__
#define __msgjson_h__


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