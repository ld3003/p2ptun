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
        return 0;

#if (ENABLE_KCP == 1)
    session->kcp = ikcp_create(0x0001, (void *)session);
    session->kcp->output = kcp_output;

#if !PRACTICAL_CONDITION

    ikcp_wndsize(session->kcp, 1024, 1024);
    ikcp_nodelay(session->kcp, 1, 1, 1, 1);
    ikcp_setmtu(session->kcp, 548);
    session->kcp->stream = 0;
    session->kcp->rx_minrto = 5;

#else

    // equal to kcpp default config
    ikcp_wndsize(session->kcp, 128, 128);
    ikcp_nodelay(session->kcp, 1, 10, 1, 1);
    session->kcp->stream = 0;
    session->kcp->rx_minrto = 10;
    ikcp_setmtu(session->kcp, 548);

#endif

#endif

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
int p2ptun_input_p2pdata_mux(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length, unsigned char mux)
{

    int ret = 0;
    if (session->cur_status == P2PTUN_STATUS_CONNECTED)
    {
        unsigned char *_tmp = malloc(length + 1);
        if (_tmp > 0)
        {
            _tmp[0] = mux;
            memcpy(_tmp + 1, data, length);

            ret = session->out_dat(_tmp, length + 1, P2PTUN_UDPPKG_TYPE_P2PRAWDATA);
            if (ret >= 1)
                ret -= 1;
            free(_tmp);
        }
    }
    //printf("p2ptun_input_p2pdata %d\n", ret);
    return ret;
}

int p2ptun_set_arrived_callback(struct P2PTUN_CONN_SESSION *session, OUTPUT_P2PDAT raw_cb, OUTPUT_P2PDAT kcp_cb)
{
    session->out_p2pdat = raw_cb;
    session->out_p2pdat_kcp = kcp_cb;
    return 0;
}

int p2ptun_input_p2pdata_raw(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{

    return p2ptun_input_p2pdata_mux(session, data, length, 'D');
}

int p2ptun_input_p2pdata_kcp(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
#if (ENABLE_KCP == 1)
    int ret = -P2PTUN_ERROR;
    if ((ikcp_waitsnd(session->kcp)) <= KCPBUFFER_COUNT)
    {
        ret = ikcp_send(session->kcp, data, length);
        //ikcp_update(session->kcp, iclock());
        ret = P2PTUN_OK;
    }
    else
    {
    }

    return ret;
#else
    return 0;
#endif
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
            session->out_p2pdat(data + 1, length - 1);
            break;
        }

#if (ENABLE_KCP == 1)
        case 'K':
        {

            int kcpdata_len;
            unsigned char *kcpdata;
            /*
            将数据交给KCP，然后立即通过RECV函数读取出来，再扔给上层,这样效率最高，不用轮训操作
            */

            ikcp_input(session->kcp, data + 1, length - 1);
            kcpdata = malloc(length);
            if (kcpdata > 0)
            {
                kcpdata_len = ikcp_recv(session->kcp, kcpdata, length);
                
                if (kcpdata_len > 0)
                {
                    printf("P2PDATA->KCP %d %d\n", length, kcpdata_len);
                    session->out_p2pdat_kcp(kcpdata, kcpdata_len);
                }
                free(kcpdata);
            }
            else
            {
                printf("P2PDATA->KCP NODATA\n");
            }

            break;
        }
#endif
        }
    }
}

void p2ptun_input_timer(struct P2PTUN_CONN_SESSION *session)
{

    char tmp[128];

#if (ENABLE_KCP == 1)
    //10ms update kcp , 理论上应该用 ikcp_CHECK 来判断下次update时间
    int cmp_kcpupdatetime = get_sub_tim_ms(&session->kcpupdate_time);
    if (cmp_kcpupdatetime >= 10)
    {
        ikcp_update(session->kcp, iclock());
        p2ptun_get_current_time(&session->kcpupdate_time);
    }
#endif

    //1000ms routing
    int cmp_routing_time = get_sub_tim_ms(&session->routing_time);
    if (cmp_routing_time >= 1000)
    {
        if (session->workmode == P2PTUN_WORKMODE_CLIENT)
        {
            p2ptun_client_timer_client(session);
        }
        else
        {
            p2ptun_client_timer_server(session);
        }
        p2ptun_get_current_time(&session->routing_time);
    }

#if 0
    //间隔5000ms发送注册包
    int regpkg_time = get_sub_tim_ms(&session->regpkg_time);
    if (regpkg_time >= 5000)
    {
        const char MSGSTR[] = "\{\"from\":\"%s\"\,\"to\":\"%s\"\}";
        snprintf(tmp, 64, MSGSTR, session->local_peername, session->local_peername);
        session->out_dat(tmp, strlen(tmp), P2PTUN_UDPPKG_TYPE_RELAYMSG);
        p2ptun_get_current_time(&session->regpkg_time);
    }
#endif

    //
}

void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status)
{
    printf("CHANGE STATUS %d ----> %d \n", session->cur_status, status);
    session->prev_status = session->cur_status;
    session->cur_status = status;
    p2ptun_get_current_time(&session->status_time);
}
#if (ENABLE_KCP == 1)
int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    struct P2PTUN_CONN_SESSION *session = user;
    p2ptun_input_p2pdata_mux(session, (unsigned char *)buf, len, (unsigned char)'K');
    return 0;
}
#endif