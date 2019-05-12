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
    return 0;
}

int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg)
{
    switch (session->workmode)
    {
    case P2PTUN_WORKMODE_CLIENT:
        return p2ptun_input_msg_client(session,msg);
        break;
    case P2PTUN_WORKMODE_SERVER:
        return p2ptun_input_msg_server(session,msg);
        break;
    }
}
int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    
    return 0;
}

int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length)
{
    switch (session->workmode)
    {
    case P2PTUN_WORKMODE_CLIENT:
        p2ptun_input_data_client(session,data,length);
        break;
    case P2PTUN_WORKMODE_SERVER:
        p2ptun_input_data_server(session,data,length);
        break;
    }
}

void p2ptun_input_timer(struct P2PTUN_CONN_SESSION *session)
{
    
    if (session->workmode == P2PTUN_WORKMODE_CLIENT)
    {
        p2ptun_client_timer_client(session);
    }
    else
    {
        p2ptun_client_timer_server(session);
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
