#ifndef __p2ptun_h__
#define __p2ptun_h__

#ifdef __cplusplus

extern "C"
{

#endif

#define P2PTUN_ERROR_NONE				0
#define P2PTUN_ERROR_MEM				-1

	struct P2PTUN_CONN_SESSION
	{
		char name[12];
	};

	int p2ptun_init();
	int p2ptun_deinit();

	struct P2PTUN_CONN_SESSION *p2ptun_connect(char *peername);
	int p2ptun_disconnect(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_send_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int len);
	int p2ptun_set_recvdata_cb(struct P2PTUN_CONN_SESSION *session, void *cb);

#ifdef __cplusplus
};
#endif

#endif
