
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

int p2ptun_input_msg_client(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    struct JSONDATA indat;
    if (json2data(msg, &indat) == 0)
    {

        if (indat.cmd == P2PTUN_CMD_MQTTPING)
        {
            char *json;
            struct JSONDATA dat;
            dat.cmd = P2PTUN_CMD_MQTTPONG;
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
            if (indat.cmd == P2PTUN_CMD_MQTTPONG)
            {
                p2ptun_setstatus(session, P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE);
                char *json;
                struct JSONDATA dat;
                dat.cmd = P2PTUN_CMD_MQTTGETNTYPE;
                snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                json = data2json(&dat);
                session->out_msg(json);
                free(json);
                //
            }
            break;

        case P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE:
            break;

        default:
            break;
        }
    }

    return 0;
}

int p2ptun_input_data_client(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{

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
                char *json;
                struct JSONDATA dat;
                dat.cmd = P2PTUN_CMD_MQTTPING;
                snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                json = data2json(&dat);
                session->out_msg(json);
                free(json);
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
                dat.cmd = P2PTUN_CMD_UDPPING;
                json = data2json(&dat);
                session->out_dat(json, strlen(json), 1);
                free(json);
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
            char *json;
            struct JSONDATA dat;
            dat.cmd = P2PTUN_CMD_MQTTPING;
            json = data2json(&dat);
            session->out_dat(json, strlen(json), 0);
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
            char *json;
            struct JSONDATA dat;
            dat.cmd = P2PTUN_CMD_MQTTPING;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_msg(json);
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
            char *json;
            struct JSONDATA dat;
            dat.cmd = P2PTUN_CMD_MQTTGETNTYPE;
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_msg(json);
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

    default:
        break;
    }

    //printf("RUNNING session:%s %d\n", session->local_peername, session->status_time.sec);
    return;
}