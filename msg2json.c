#include "msg2json.h"
#include "cJSON.h"

static char json_string[256];

char *ping(char *to, char *from)
{
    return json_string;
}
char *pong(char *to, char *from)
{
    return json_string;
}

char *get_net_type(char *to, char *from)
{
    return json_string;
}

char *resp_net_type(char *to, char *from, int nettype, char *ip, int port)
{
    return json_string;
}

char *request_udp(char *to, char *from, char *ip, int port)
{
    return json_string;
}

int json2data(char *json, struct JSONDATA *dat)
{
    cJSON *pSub;
    cJSON *pJson;
    //data[length] = 0x0;
    printf("JONS:%s\n", json);
    if (NULL == json)
    {
        printf("err json\n");
        return -1;
    }
    pJson = cJSON_Parse(json);
    if (NULL == pJson)
    {
        printf("err json\n");
        return -1;
    }

    pSub = cJSON_GetObjectItem(pJson, "cmd");
    if (NULL != pSub)
    {
        dat->cmd = pSub->valueint;
    }
    else
    {
        return -1;
    }

    pSub = cJSON_GetObjectItem(pJson, "seq");
    if (NULL != pSub)
    {
        dat->seq = pSub->valueint;
    }
    else
    {
        return -1;
    }

    pSub = cJSON_GetObjectItem(pJson, "port");
    if (NULL != pSub)
    {
        dat->port = pSub->valueint;
    }
    pSub = cJSON_GetObjectItem(pJson, "addr");
    if (NULL != pSub)
    {
        snprintf(dat->addr, sizeof(dat->addr), "%s", pSub->valuestring);
    }
    pSub = cJSON_GetObjectItem(pJson, "ntype");
    if (NULL != pSub)
    {
        dat->ntype = pSub->valueint;
    }

    return 0;

}

char *data2json(struct JSONDATA *dat)
{
    return 0;
}