#ifndef __p2ptun_h__
#define __p2ptun_h__

#ifdef __cplusplus

extern "C"
{
#endif

#include <pthread.h>

	enum
	{
		P2PTUN_OK = 0,
		P2PTUN_ERROR,
		P2PTUN_MEMERR,
		P2PTUN_STATUSERR,
	};

	enum
	{
		P2PTUN_STATUS_INIT,
		P2PTUN_STATUS_LISTEN,
		P2PTUN_STATUS_CONNECTING,
		P2PTUN_STATUS_CONNECTED,
	};

	struct P2PTUN_CONN_SESSION
	{
		char peername[32];
		int socketfd;
		int msgpipe;
		char currentstatus;
		pthread_t threadid
	};

	struct P2PTUN_CONN_SESSION *p2ptun_alloc_session();
	int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_listen(struct P2PTUN_CONN_SESSION *session);
	int p2ptun_connect(struct P2PTUN_CONN_SESSION *session, char *peername);
	int p2ptun_disconnect(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_send_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int len);
	int p2ptun_set_recvdata_cb(struct P2PTUN_CONN_SESSION *session, void *cb);

	int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg);

#ifdef __cplusplus
};
#endif

#endif
