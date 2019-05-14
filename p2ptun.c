#include "p2ptun.h"
#include "p2ptun_common.h"
#include "p2ptun_session_status.h"
#include "cJSON.h"
#include "msg2json.h"
#include "p2ptun_client.h"
#include "p2ptun_server.h"

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
    free(session);
    return 0;
}

int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    switch (session->workmode)
    {
    case P2PTUN_WORKMODE_CLIENT:
        return p2ptun_input_msg_client(session, msg);
        break;
    case P2PTUN_WORKMODE_SERVER:
        return p2ptun_input_msg_server(session, msg);
        break;
    }
}
int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    if (session->cur_status == P2PTUN_STATUS_CONNECTED)
    {
        unsigned char *_tmp = malloc(length + 1);
        if (_tmp > 0)
        {
            _tmp[0] = 'D';
            memcpy(_tmp + 1, data, length);

            session->out_dat(_tmp, length + 1, P2PTUN_UDPPKG_TYPE_P2PRAWDATA);

            free(_tmp);
        }
    }
    return 0;
}

int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    switch (session->workmode)
    {
    case P2PTUN_WORKMODE_CLIENT:
        p2ptun_input_data_client(session, data, length);
        break;
    case P2PTUN_WORKMODE_SERVER:
        p2ptun_input_data_server(session, data, length);
        break;
    }
}

void p2ptun_input_timer(struct P2PTUN_CONN_SESSION *session)
{

    char tmp[128];
    int cmp_sec = get_sub_tim_sec(&session->reg_time);

    if (session->workmode == P2PTUN_WORKMODE_CLIENT)
    {
        p2ptun_client_timer_client(session);
    }
    else
    {
        p2ptun_client_timer_server(session);
    }

    //间隔5秒发送注册包
    if (cmp_sec > 5)
    {
#define MSGSTR "\{\"from\":\"%s\"\,\"to\":\"%s\"\}"
        snprintf(tmp, 64, MSGSTR, session->local_peername, session->local_peername);
        session->out_dat(tmp, strlen(tmp), P2PTUN_UDPPKG_TYPE_RELAYMSG);
        p2ptun_get_current_time(&session->reg_time);
    }

    p2ptun_input_p2pdata(session,"test",4);
    //
}

void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status)
{
    printf("CHANGE STATUS %d ----> %d \n", session->cur_status, status);
    session->prev_status = session->cur_status;
    session->cur_status = status;
    p2ptun_get_current_time(&session->status_time);
}
