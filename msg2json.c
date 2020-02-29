#include "msg2json.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdio.h>

int json2data(char *json, struct JSONDATA *dat)
{
    cJSON *pSub;
    cJSON *pJson;

    //printf("JONS:%s\n", json);
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

    pSub = cJSON_GetObjectItem(pJson, "from");
    if (NULL != pSub)
    {
        snprintf(dat->from, sizeof(dat->from), "%s", pSub->valuestring);
    }

    pSub = cJSON_GetObjectItem(pJson, "to");
    if (NULL != pSub)
    {
        snprintf(dat->to, sizeof(dat->to), "%s", pSub->valuestring);
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

    char *p;
    cJSON *pJsonRoot = NULL;

    pJsonRoot = cJSON_CreateObject();
    if (NULL == pJsonRoot)
    {
        return 0;
    }

    cJSON_AddNumberToObject(pJsonRoot, "cmd", dat->cmd);
    cJSON_AddNumberToObject(pJsonRoot, "seq", dat->seq);
    cJSON_AddStringToObject(pJsonRoot, "from", dat->from);
    cJSON_AddStringToObject(pJsonRoot, "to", dat->to);
    cJSON_AddStringToObject(pJsonRoot, "addr", dat->addr);
    cJSON_AddNumberToObject(pJsonRoot, "port", dat->port);
    cJSON_AddNumberToObject(pJsonRoot, "ntype", dat->ntype);

    p = cJSON_Print(pJsonRoot);

    if (NULL == p)
    {
        //convert json list to string faild, exit
        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free

        cJSON_Delete(pJsonRoot);

        return 0;
    }
    //free(p);
    cJSON_Delete(pJsonRoot);

    return p;
    //
}