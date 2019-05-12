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