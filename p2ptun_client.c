
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
    for (i = 0; i < 1; i++)
    {
        session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
    }
    free(json);

    //通过MSG通知对方向本地发送 UDP_TEST
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSG_REQUESTUDPTEST;
    dat.port = session->local_port;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    json = data2json(&dat);
    //session->out_msg(json);
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
    free(json);
    return 0;
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
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
    free(json);
    return 0;
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
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
    free(json);
    return 0;
}

static int p2ptun_send_msg_connected(struct P2PTUN_CONN_SESSION *session)
{
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSG_CONNECTED;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    dat.port = session->local_port;
    json = data2json(&dat);
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
    free(json);
    return 0;
}

static int p2ptun_send_udp_hb(struct P2PTUN_CONN_SESSION *session)
{
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_UDP_HB;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    dat.port = session->local_port;
    json = data2json(&dat);
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
    free(json);
    return 0;
}

static int p2ptun_send_disconnect(struct P2PTUN_CONN_SESSION *session)
{
    char *json;
    struct JSONDATA dat;
    memset(&dat, 0x0, sizeof(dat));
    dat.cmd = P2PTUN_CMD_MSG_DISCONNECT;
    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
    json = data2json(&dat);
    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
    free(json);
    return 0;
}

int p2ptun_input_msg_client(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    struct JSONDATA indat;
    if (json2data(msg, &indat) == 0)
    {

        printf("[IN PKG] incmd %d status : %d\n", indat.cmd, session->cur_status);
        if (indat.cmd == P2PTUN_CMD_MSGPING)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_MSGPONG;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
            free(json);
            snprintf(session->remote_peername, sizeof(session->remote_peername), "%s", indat.from);
        }

        switch (session->cur_status)
        {

        case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
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
                    printf("当前网络类型:%d %s:%d\n", session->local_nettype, session->local_ipaddr, session->local_port);

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
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_UDPPONG)
                {
                    snprintf(session->local_ipaddr, 32, "%s", indat.addr);
                    session->local_port = indat.port;
                    p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1);
                    //printf("RECV LOCAL AP %s:%d\n", session->local_ipaddr, session->local_port);
                    printf("当前网络ap : %s:%d\n", session->local_ipaddr, session->local_port);
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat, 0x0, sizeof(dat));
                    dat.cmd = P2PTUN_CMD_UDPPING;
                    json = data2json(&dat);
                    session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_PING2);
                    free(json);
                }
            }

            //P2PTUN_CMD_UDP_RESPTEST

            break;
        }

        case P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_UDP_RESPTEST)
                {
                    printf("收到远端PC的UDP数据打洞成功!!!!!!!!!!!!!!!!!!!!\n");
                    p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_CONNECTED);
                    p2ptun_send_msg_connected(session);
                }
            }
            break;
        }

        case P2PTUN_CMD_MSG_DISCONNECT:
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            break;
        }

        case P2PTUN_STATUS_CONNECTING_WAIT_CONNECTED:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_MSG_RESPCONNECTED)
                {
                    printf("确认连接成功!!!!!!!!!!!!!!!!!!!!\n");
                    p2ptun_get_current_time(&session->recvhb_time); //将心跳计时器重新开始计时
                    p2ptun_initkcp(session);
                    p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTED);
                    printf("\n\n");
                    printf("--------------------------------------------------\n");
                    printf("         CONNECT SUCCESS !                        \n");
                    printf("[REMOTE] %s - %s:%d\n", session->remote_peername, session->remote_ipaddr, session->remote_port);
                    printf("[LOCAL ] %s - %s:%d\n", session->local_peername, session->local_ipaddr, session->local_port);
                    printf("+--------------------------------------------------\n");
                    printf("\n\n");
                }
            }
            break;
        }

        case P2PTUN_STATUS_CONNECTED:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_UDP_HB)
                {
                    printf("recv hb\n");
                    p2ptun_get_current_time(&session->recvhb_time);
                }
            }

            break;
        }

            //-----------------------------------------------------------------

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

                    printf("均为非对称NAT\n");
                }
                else if ((session->local_nettype == 1) && (session->remote_nettype == 0))
                {
                    printf("本地为对称，对方为非对称 NAT\n");
                }
                else if ((session->local_nettype == 0) && (session->remote_nettype == 1))
                {
                }
            }

            break;

        default:
            break;
        }
    }

    return 0;
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
        session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_PING1);
        free(json);

        break;
    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);

        if (cmp_sec <= 5)
        {
            p2ptun_send_msgping(session);
        }
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
        }

        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if (cmp_sec <= 5)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_UDPPING;
            json = data2json(&dat);
            session->out_dat((unsigned char*)json, strlen(json), P2PTUN_UDPPKG_TYPE_PING2);
            free(json);
        }
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
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
        if (cmp_sec <= P2PTUN_PKGTIMEOUT)
        {
            p2ptun_send_msgping(session);
        }
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
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
        if (cmp_sec <= P2PTUN_PKGTIMEOUT)
        {
            p2ptun_send_msg_getntype(session);
        }
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
        }

        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if (cmp_sec <= P2PTUN_PKGTIMEOUT)
            p2ptun_send_dadong_pkg(session);
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
        }
        break;
    }

    case P2PTUN_STATUS_CONNECTING_WAIT_CONNECTED:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if (cmp_sec <= P2PTUN_PKGTIMEOUT)
            p2ptun_send_msg_connected(session);
        else
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
        }
    }

    case P2PTUN_STATUS_CONNECTED:
    {
        int cmp_sec2 = get_sub_tim_sec(&session->recvhb_time);

#if 0
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if ((cmp_sec % P2PTUN_SENDHB_TIME) == 0)
        {
            p2ptun_send_udp_hb(session);
        }
#endif

        if ((cmp_sec2 % P2PTUN_SENDHB_TIME) == 0)
        {
            p2ptun_send_udp_hb(session);
        }

        if (cmp_sec2 > (P2PTUN_PKGTIMEOUT))
        {
            printf("超时了，断开链接 %d \n", cmp_sec2);
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            p2ptun_send_disconnect(session);
        }
        break;
    }
    case P2PTUN_STATUS_DISCONNECT:
    {
        printf("重新链接……\n");
        p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING);
        break;
    }
    default:
        break;
    }

    //printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);
    return;
}
