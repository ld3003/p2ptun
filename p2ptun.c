#include "p2ptun.h"

int p2ptun_init()
{
    return 0;
}
int p2ptun_deinit()
{
    return 0;
}

struct P2PTUN_CONN_SESSION *p2ptun_connect(char *peername)
{
    struct P2PTUN_CONN_SESSION *session;
    session = malloc(sizeof(struct P2PTUN_CONN_SESSION));
    if (session <= 0)
        return P2PTUN_ERROR_MEM;
    memset(session,0x0,sizeof(struct P2PTUN_CONN_SESSION));

    

    
    return session;
}
int p2ptun_disconnect(struct P2PTUN_CONN_SESSION *session)
{
    return 0;
}

int p2ptun_send_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int len)
{
    return 0;
}
int p2ptun_set_recvdata_cb(struct P2PTUN_CONN_SESSION *session, void *cb)
{
    return 0;
}