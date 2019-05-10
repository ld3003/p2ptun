#include "p2ptun.h"
#include "p2ptun_common.h"
#include "p2ptun_session_status.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

struct P2PTUN_CONN_SESSION *p2ptun_alloc_session()
{
    int ret;
    struct P2PTUN_CONN_SESSION *session;
    session = malloc(sizeof(struct P2PTUN_CONN_SESSION));
    if (session <= 0)
        return -P2PTUN_MEMERR;
    return session;
}

int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session)
{
    return 0;
}

int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    //
    cJSON *pSub;
    cJSON *pJson;

    if (NULL == msg)
    {
        return;
    }
    pJson = cJSON_Parse(msg);
    if (NULL == pJson)
    {
        return;
    }

    pSub = cJSON_GetObjectItem(pJson, "cmd");
    if (NULL != pSub)
    {
        int seq = 0;
        char *name = 0;
        int cmd = pSub->valueint;
        pSub = cJSON_GetObjectItem(pJson, "seq");
        if (NULL != pSub)
        {
            seq = pSub->valueint;
        }
        else
        {
            return;
        }

        pSub = cJSON_GetObjectItem(pJson, "name");
        if (NULL != pSub)
        {
            name = pSub->string;
        }
        else
        {
            return;
        }

        switch (cmd)
        {
        case 1:
        case 2:
            break;
        default:
            break;
        }

        //data_2_json(cmd, seq, 0, "");
    }

    cJSON_Delete(pJson);

    return 0;
}
int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    //
    return 0;
}

int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    //
    return 0;
}

void p2ptun_client_mainloop(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING);
        break;

    case P2PTUN_STATUS_CONNECTING:

        //session->out_msg("PING");
        /*
            发送MQTT PING命令
        */
        break;
    case P2PTUN_STATUS_CONNECTING_WAIT_PONG:
        /*
            等待 PONG 命令
            间隔500ms发一次ping，5秒钟认为超时，应该写在红定义
        */
        break;
    case P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE:
        /*
            等待 GET NETTYPE 命令
            间隔500ms发一次 NETTYPE，5秒钟认为超时，应该写在红定义
        */
        break;

    case 888:
        /*
            双方均为CLONE类型
            发送UDP PING
            通知对方发送UDP PING ，等待ECHO报文
        */
        break;

    default:
        break;
    }

    printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);
    sleep(1);
    return;
}

void p2ptun_server_mainloop(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN);
        break;
    case P2PTUN_STATUS_LISTEN:
        break;
    default:
        break;
    }

    printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);

    return;
}
void p2ptun_mainloop(struct P2PTUN_CONN_SESSION *session)
{
    if (session->workmode == P2PTUN_WORKMODE_CLIENT)
    {
        p2ptun_client_mainloop(session);
    }
    else
    {
        p2ptun_server_mainloop(session);
    }

    //
}

void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status)
{
    session->prev_status = session->cur_status;
    session->cur_status = status;
    p2ptun_get_current_time(&session->status_time);
}
