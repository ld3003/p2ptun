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

int p2ptun_input_msg_server(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    struct JSONDATA indat;
    if (json2data(msg, &indat) == 0)
    {

        if (indat.cmd == P2PTUN_CMD_MQTTPING)
        {
            char *json;
            struct JSONDATA dat;
            memset(&dat,0x0,sizeof(dat));
            dat.cmd = P2PTUN_CMD_MQTTPONG;
            snprintf(session->remote_peername, sizeof(session->remote_peername), "%s", indat.from);
            snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
            snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
            json = data2json(&dat);
            session->out_msg(json);
            free(json);
            printf("#################### %s\n",indat.from);
            
        }

        //P2PTUN_CMD_MQTTGETNTYPE
        if (indat.cmd == P2PTUN_CMD_MQTTGETNTYPE)
        {
            switch (session->cur_status)
            {

            case P2PTUN_STATUS_LISTEN:
            case P2PTUN_STATUS_LISTEN_HANDSHAKE:

                if (get_sub_tim_sec(&session->getnettype_time) > 30)
                {
                    printf("重新获取网络类型\n");
                    p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE);
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat,0x0,sizeof(dat));
                    dat.cmd = P2PTUN_CMD_MQTTPING;
                    json = data2json(&dat);
                    session->out_dat(json, strlen(json), 0);
                    free(json);
                }
                else
                {
                    printf("距离上次获取网络类型，不足30秒不用重新获取\n");
                    char *json;
                    struct JSONDATA dat;
                    memset(&dat,0x0,sizeof(dat));
                    dat.cmd = P2PTUN_CMD_MQTTRRESPNTYPE;
                    snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                    snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                    dat.ntype = session->local_nettype;
                    dat.port = session->local_port;
                    snprintf(dat.addr, sizeof(dat.addr), "%s", session->local_ipaddr);
                    json = data2json(&dat);
                    session->out_msg(json);
                    free(json);
                }
                break;
            }
        }

        switch (session->cur_status)
        {

        case P2PTUN_STATUS_LISTEN:
        {
            if (indat.cmd == P2PTUN_CMD_MQTTPING)
                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE);
            break;
        }

        case P2PTUN_STATUS_LISTEN_HANDSHAKE:
            break;
        }
    }
}
int p2ptun_input_data_server(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{

    switch (session->cur_status)
    {
    case P2PTUN_STATUS_CONNECTED:
        break;

    case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1:
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

                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE);

                char *json;
                struct JSONDATA dat;
                memset(&dat,0x0,sizeof(dat));
                dat.cmd = P2PTUN_CMD_MQTTRRESPNTYPE;
                snprintf(dat.from, sizeof(dat.from), "%s", session->local_peername);
                snprintf(dat.to, sizeof(dat.to), "%s", session->remote_peername);
                dat.ntype = session->local_nettype;
                dat.port = session->local_port;
                snprintf(dat.addr, sizeof(dat.addr), "%s", session->local_ipaddr);
                json = data2json(&dat);
                session->out_msg(json);
                free(json);
            }
        }
        break;
    }
    case P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE:
    {
        struct JSONDATA indat;
        if (json2data(data, &indat) == 0)
        {
            if (indat.cmd == P2PTUN_CMD_UDPPONG)
            {
                snprintf(session->local_ipaddr, 32, "%s", indat.addr);
                session->local_port = indat.port;
                p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1);
                printf("RECV LOCAL AP %s %d\n", session->local_ipaddr, session->local_port);
                char *json;
                struct JSONDATA dat;
                memset(&dat,0x0,sizeof(dat));
                dat.cmd = P2PTUN_CMD_MQTTPING;
                json = data2json(&dat);
                session->out_dat(json, strlen(json), 0);
                free(json);
            }
        }

        break;
    }

    default:
        break;
    }
}

void p2ptun_client_timer_server(struct P2PTUN_CONN_SESSION *session)
{
    switch (session->cur_status)
    {
    case P2PTUN_STATUS_INIT:
        p2ptun_setstatus(session, P2PTUN_STATUS_LISTEN);
        break;
    }
}