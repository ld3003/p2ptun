#include "p2ptun_server.h"
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
    session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
    free(json);
}

int p2ptun_input_msg_server(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    struct JSONDATA indat;
    if (json2data(msg, &indat) == 0)
    {
        //根据消息命令来进行相应处理
        if (indat.cmd == P2PTUN_CMD_MSGPING)
        {
            char *json;
            struct JSONDATA dat;

            //当收到PING命令就认为客户端重启发起了链接，无条件将状态机恢复到初始状态
            p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN);

            snprintf(session->remote_peername, sizeof(session->remote_peername), "%s", indat.from);
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_MSGPONG;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
            free(json);

            session->remote_port = indat.port;
        }

        if (indat.cmd == P2PTUN_CMD_MSG_REQUESTUDPTEST)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_UDP_RESPONSEUDPTEST;
            snprintf(session->remote_peername, sizeof(session->remote_peername), "%s", indat.from);
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
            free(json);

            //获取本次port
            printf("session->remote_port = %d\n", indat.port);
            session->remote_port = indat.port;
        }

        if (indat.cmd == P2PTUN_CMD_MSG_CONNECTED)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat, 0x0, sizeof(dat));
            dat.cmd = P2PTUN_CMD_MSG_RESPCONNECTED;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
            free(json);

            p2ptun_get_current_time(&session->recvhb_time); //将心跳计时器重新开始计时
            p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTED);

            printf("\n\n");
            printf("--------------------------------------------------\n");
            printf("         NEW CONNECT !                        \n");
            printf("[REMOTE] %s - %s:%d\n", session->remote_peername, session->remote_ipaddr, session->remote_port);
            printf("[LOCAL ] %s - %s:%d\n", session->local_peername, session->local_ipaddr, session->local_port);
            printf("+--------------------------------------------------\n");
            printf("\n\n");
        }

        //P2PTUN_CMD_MQTTGETNTYPE
        if (indat.cmd == P2PTUN_CMD_MSGGETNTYPE)
        {
            //P2PTUN_CMD_MSGGETNTYPE 指令中包含远程和本地地址
            snprintf(session->remote_ipaddr, sizeof(session->remote_ipaddr), "%s", indat.addr);
            session->remote_port = indat.port;
            printf("################## %s:%d\n", session->remote_ipaddr, session->remote_port);
            switch (session->cur_status)
            {

            case P2PTUN_STATUS_LISTEN:
            case P2PTUN_STATUS_LISTEN_HANDSHAKE:

                //关闭获取网络类型的超时判断
                if (1 /*get_sub_tim_sec(&session->getnettype_time) > 30*/)
                {
                    printf("重新获取网络类型\n");
                    p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE);
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat, 0x0, sizeof(dat));
                    dat.cmd = P2PTUN_CMD_MSGPING;
                    json = data2json(&dat);
                    session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_PING1);
                    free(json);
                }
                else
                {
                }
                break;
            }
        }

        //根据当前状态来执行操作
        switch (session->cur_status)
        {

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

        case P2PTUN_STATUS_LISTEN_HANDSHAKE:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_UDP_TEST)
                {
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat, 0x0, sizeof(dat));
                    dat.cmd = P2PTUN_CMD_UDP_RESPTEST;
                    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                    json = data2json(&dat);
                    session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_P2PMSG);
                    free(json);
                }
            }

            break;
        }

        case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1:
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

                    p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE);

                    char *json;
                    struct JSONDATA dat;
                    memset(&dat, 0x0, sizeof(dat));
                    dat.cmd = P2PTUN_CMD_MSGRRESPNTYPE;
                    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                    dat.ntype = session->local_nettype;
                    dat.port = session->local_port;
                    snprintf(dat.addr, sizeof(dat.addr), "%s", session->local_ipaddr);
                    json = data2json(&dat);
                    session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_RELAYMSG);
                    free(json);
                }
            }
            break;
        }
        case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE:
        {
            struct JSONDATA indat;
            if (json2data(msg, &indat) == 0)
            {
                if (indat.cmd == P2PTUN_CMD_UDPPONG)
                {
                    snprintf(session->local_ipaddr, 32, "%s", indat.addr);
                    session->local_port = indat.port;
                    p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1);
                    printf("RECV LOCAL AP %s %d\n", session->local_ipaddr, session->local_port);
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat, 0x0, sizeof(dat));
                    dat.cmd = P2PTUN_CMD_MSGPING;
                    json = data2json(&dat);
                    session->out_dat(json, strlen(json), P2PTUN_UDPPKG_TYPE_PING1);
                    free(json);
                }
            }

            break;
        }

        case P2PTUN_CMD_MSG_DISCONNECT:
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
            break;
        }

        case P2PTUN_STATUS_LISTEN:
        {
            if (indat.cmd == P2PTUN_CMD_MSGPING)
                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE);
            break;
        }
        }
    }
}

void p2ptun_client_timer_server(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN);
        break;
    case P2PTUN_STATUS_LISTEN_HANDSHAKE:
    {
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if (cmp_sec > P2P_SESSION_TIMEOUT)
        {
            p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN);
        }
        break;
    }

    case P2PTUN_STATUS_CONNECTED:
    {
        int cmp_sec2 = get_sub_tim_sec(&session->recvhb_time);
        int cmp_sec = get_sub_tim_sec(&session->status_time);
        if ((cmp_sec % P2PTUN_SENDHB_TIME) == 0)
        {
            p2ptun_send_udp_hb(session);
        }

        if (cmp_sec2 > (P2PTUN_SENDHB_TIME * 3))
        {
            printf("超时了，断开链接\n");
            p2ptun_setstatus(session, P2PTUN_STATUS_DISCONNECT);
        }
        break;
    }
    }
}