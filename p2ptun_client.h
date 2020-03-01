#ifndef __p2p_tun_client_h__
#define __p2p_tun_client_h__

#include "p2ptun.h"


int p2ptun_input_msg_client(struct P2PTUN_CONN_SESSION *session, char *msg);
void p2ptun_client_update_client(struct P2PTUN_CONN_SESSION *session);

#endif
