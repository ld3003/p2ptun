#ifndef __p2p_tun_server_h__
#define __p2p_tun_server_h__

#include "p2ptun.h"

int p2ptun_input_msg_server(struct P2PTUN_CONN_SESSION *session, char *msg);
void p2ptun_client_timer_server(struct P2PTUN_CONN_SESSION *session);


#endif


