
#include "p2ptun_client.h"
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

static int p2ptun_send_dadong_pkg(struct P2PTUN_CONN_SESSION *session)
{
    int i = 0;
    //向对方UDP端口发送 UDP_TEST
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_UDP_TEST;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    json = data2json(&dat);
    for(i=0;i<1;i++)
    {
        session->out_dat(json, strlen(json), 2);
        usleep(1000*10);
    }
    free(json);

    //通过MSG通知对方向本地发送 UDP_TEST
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSG_REQUESTUDPTEST;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    json = data2json(&dat);
    session->out_msg(json);
    free(json);
}

static int p2ptun_send_msgping(struct P2PTUN_CONN_SESSION *session)
{
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSGPING;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    json = data2json(&dat);
    session->out_msg(json);
    free(json);
}

static int p2ptun_send_msg_getntype(struct P2PTUN_CONN_SESSION *session)
{
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSGGETNTYPE;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    snprintf(dat.addr, sizeof(dat.addr), "%s", session->local_ipaddr);
    dat.port = session->local_port;
    json = data2json(&dat);
    session->out_msg(json);
    free(json);
}

int p2ptun_input_msg_client(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    struct JSONDATA indat;
    if (json2data(msg, &indat) == 0)
    {

        if (indat.cmd == P2PTUN_CMD_MSGPING)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_MSGPONG;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_msg(json);
            free(json);
            snprintf(session->remote_peername, sizeof(session->remote_peername), "%s", indat.from);
        }

        switch (session->cur_status)
        {
        case P2PTUN_STATUS_CONNECTING_WAIT_PONG:
            if (indat.cmd == P2PTUN_CMD_MSGPONG)
            {
                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE);
                p2ptun_send_msg_getntype(session);
                //
            }
            break;

        case P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE:

            if (indat.cmd == P2PTUN_CMD_MSGRRESPNTYPE)
            {
                //保存远程IP地址信息
                session->remote_nettype = indat.ntype;
                session->remote_port = indat.port;
                snprintf(session->remote_ipaddr, sizeof(session->remote_ipaddr), "%s", indat.addr);

                if ((session->local_nettype == 0) && (session->remote_nettype == 0))
                {

                    p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO);
                    p2ptun_send_dadong_pkg(session);

                    printf("均为对称NAT\n");
                }
            }

            break;

        default:
            break;
        }
    }

    return 0;
}

int p2ptun_input_data_client(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{

    printf("DEBUG INPUT DATA MSG %s data\n");

    switch (session->cur_status)
    {
    case P2PTUN_STATUS_CONNECTED:
        break;
    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1:
    {
        struct JSONDATA indat;
        if (json2data(data, &indat) == 0)
        {
            if (indat.cmd == P2PTUN_CMD_UDPPONG)
            {
                snprintf(session->local_ipaddr, 32, "%s", indat.addr);
                if (session->local_port == indat.port)
                {
                    session->local_nettype = 0;
                }
                else
                {
                    session->local_nettype = 1;
                }
                printf("RECV LOCAL NETTYPE %d\n", session->local_nettype);

                p2ptun_get_current_time(&session->getnettype_time);

                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_PONG);
                p2ptun_send_msgping(session);
            }
        }
        break;
    }
    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE: //P2PTUN_STATUS_CONNECTING_WAIT_UDPTUN:
    {
        struct JSONDATA indat;
        if (json2data(data, &indat) == 0)
        {
            if (indat.cmd == P2PTUN_CMD_UDPPONG)
            {
                snprintf(session->local_ipaddr, 32, "%s", indat.addr);
                session->local_port = indat.port;
                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1);
                printf("RECV LOCAL AP %s %d\n", session->local_ipaddr, session->local_port);
                char *json;
                struct JSONDATA dat;
                memset(&dat, 0x0, sizeof(dat));
                dat.cmd = P2PTUN_CMD_UDPPING;
                json = data2json(&dat);
                session->out_dat(json, strlen(json), 1);
                free(json);
            }
        }

        //P2PTUN_CMD_UDP_RESPTEST

        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO:
    {
        struct JSONDATA indat;
        if (json2data(data, &indat) == 0)
        {
            if (indat.cmd == P2PTUN_CMD_UDP_RESPTEST)
            {
                printf("打洞成功!!!!!!!!!!!!!!!!!!!!\n");
            }
        }
        break;
    }

    default:
        break;
    }
}

void p2ptun_client_timer_client(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING);
        break;

    case P2PTUN_STATUS_CONNECTING:
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE);
        char *json;
        struct JSONDATA dat;
        memset(&dat, 0x0, sizeof(dat));
        dat.cmd = P2PTUN_CMD_UDPPING;
        json = data2json(&dat);
        session->out_dat(json, strlen(json), 0);
        free(json);

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
        {
            p2ptun_send_msgping(session);
            break;
        }
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
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_UDPPING;
            json = data2json(&dat);
            session->out_dat(json, strlen(json), 1);
            free(json);
            break;
        }
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
        {
            p2ptun_send_msgping(session);
            break;
        }
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
        {
            p2ptun_send_msg_getntype(session);
            break;
        }
        default:
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            //time out
            break;
        }
        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO:
    {
        //p2ptun_setstatus(session,P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO);
        p2ptun_send_dadong_pkg(session);
    }

    default:
        break;
    }

    //printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);
    return;
}