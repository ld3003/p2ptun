#include "p2ptun.h"
#include "p2ptun_common.h"
#include "p2ptun_session_status.h"
#include "cJSON.h"
#include "msg2json.h"

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
    struct JSONDATA dat;
    if (json2data(msg, &dat) == 0)
    {
        switch (session->cur_status)
        {
        case P2PTUN_STATUS_CONNECTING_WAIT_PONG:
            if (dat.cmd == 8101)
            {
                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE);
                //
            }
            break;

        case P2PTUN_STATUS_LISTEN:
        {
            if (dat.cmd == 0101)
                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE);
            break;
        }

        case P2PTUN_STATUS_LISTEN_HANDSHAKE:
            break;
        }
    }

    return 0;
}
int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    //
    return 0;
}

int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_CONNECTED:
        break;

    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1:
    {
        struct JSONDATA dat;
        if (json2data(data, &dat) == 0)
        {
            if (dat.cmd == 101)
            {
                snprintf(session->local_ipaddr, 32, "%s", dat.addr);
                if (session->local_port == dat.port)
                {
                    session->local_nettype = 0;
                }
                else
                {
                    session->local_nettype = 1;
                }
                printf("RECV LOCAL NETTYPE %d\n", session->local_nettype);
            }
        }
        break;
    }
    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE: //P2PTUN_STATUS_CONNECTING_WAIT_UDPTUN:
    {
        struct JSONDATA dat;
        if (json2data(data, &dat) == 0)
        {
            if (dat.cmd == 101)
            {
                snprintf(session->local_ipaddr, 32, "%s", dat.addr);
                session->local_port = dat.port;
                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1);
                printf("RECV LOCAL AP %s %d\n", session->local_ipaddr, session->local_port);
                session->out_dat("PING0", 5, 1);
            }
        }

        break;
    }

    case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1:
    {
        struct JSONDATA dat;
        if (json2data(data, &dat) == 0)
        {
            if (dat.cmd == 101)
            {
                snprintf(session->local_ipaddr, 32, "%s", dat.addr);
                if (session->local_port == dat.port)
                {
                    session->local_nettype = 0;
                }
                else
                {
                    session->local_nettype = 1;
                }
                printf("RECV LOCAL NETTYPE %d\n", session->local_nettype);
            }
        }
        break;
    }
    case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE:
    {
        struct JSONDATA dat;
        if (json2data(data, &dat) == 0)
        {
            if (dat.cmd == 101)
            {
                snprintf(session->local_ipaddr, 32, "%s", dat.addr);
                session->local_port = dat.port;
                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1);
                printf("RECV LOCAL AP %s %d\n", session->local_ipaddr, session->local_port);
                session->out_dat("PING0", 5, 1);
            }
        }

        break;
    }

    default:
        break;
    }

    //data_2_json(cmd, seq, 0, "");
}

void p2ptun_client_timer(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING);
        break;

    case P2PTUN_STATUS_CONNECTING:
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE);
        session->out_dat("PING0", 5, 0);
        //session->out_msg("PING");
        /*
            发送MQTT PING命令
        */
        break;
    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        switch (cmp_sec)
        {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            session->out_dat("PING2", 5, 0);
            break;
        default:
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            //time out
            break;
        }
        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        switch (cmp_sec)
        {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            session->out_dat("PING1", 5, 1);
            break;
        default:
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            //time out
            break;
        }
        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_PONG:
    {
        /*
            等待 PONG 命令
            间隔500ms发一次ping，5秒钟认为超时，应该写在红定义
        */

        int cmp_sec = get_sub_tim_sec(&session->status_time);
        switch (cmp_sec)
        {
        case 0:
        case 2:
        case 4:
            break;
        case 1:
        case 3:
        case 5:
            session->out_msg("ping");
            break;
        default:
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            //time out
            break;
        }

        break;
    }
    case P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE:
    {
        /*
            等待 GET NETTYPE 命令
            间隔500ms发一次 NETTYPE，5秒钟认为超时，应该写在红定义
        */
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        switch (cmp_sec)
        {
        case 0:
        case 2:
        case 4:
            break;
        case 1:
        case 3:
        case 5:
            session->out_msg("getnettype");
            break;
        default:
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            //time out
            break;
        }
        break;
    }

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

    //printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);
    return;
}

void p2ptun_server_timer(struct P2PTUN_CONN_SESSION *session)
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

    printf("RUNNING TIMER:%s %d\n", session->local_peername, session->status_time.sec);

    return;
}
void p2ptun_input_timer(struct P2PTUN_CONN_SESSION *session)
{
    if (session->workmode == P2PTUN_WORKMODE_CLIENT)
    {
        p2ptun_client_timer(session);
    }
    else
    {
        p2ptun_server_timer(session);
    }

    //
}

void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status)
{
    printf("CHANGE STATUS %d ----> %d \n", session->cur_status, status);
    session->prev_status = session->cur_status;
    session->cur_status = status;
    p2ptun_get_current_time(&session->status_time);
}
