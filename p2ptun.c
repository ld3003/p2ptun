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

    session->kcp = ikcp_create(0x11223344, (void *)0);

    return session;
}

int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session)
{
    ikcp_release(session->kcp); //
    free(session);
    return 0;
}

int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    printf("p2ptun_input_msg : %s\n", msg);

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

int p2ptun_input_p2pdata_kcp(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    return ikcp_send(session->kcp, data, length);
}

int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{

    if (length > 1)
    {
        switch (data[0])
        {
        case '{':
            p2ptun_input_msg(session, data);
            break;
        case 'D':
        {
            int kcpdata_len;
            unsigned char *kcpdata;
            session->out_p2pdat(data, length);

            /*
            将数据交给KCP，然后立即通过RECV函数读取出来，再扔给上层,这样效率最高，不用轮训操作
            */

            ikcp_input(session->kcp, data, length);
            kcpdata = malloc(kcpdata_len);
            if (kcpdata > 0)
            {
                kcpdata_len = ikcp_recv(session->kcp, kcpdata, length);
                if (kcpdata_len > 0)
                    session->out_p2pdat_kcp(kcpdata_len, kcpdata_len);
                printf("P2PDATA->KCP %d %d\n",length,kcpdata_len);
                free(kcpdata);
            }
            
        }
        break;
        }
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

    p2ptun_input_p2pdata(session, "test", 4);
    //
}

void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status)
{
    printf("CHANGE STATUS %d ----> %d \n", session->cur_status, status);
    session->prev_status = session->cur_status;
    session->cur_status = status;
    p2ptun_get_current_time(&session->status_time);
}
