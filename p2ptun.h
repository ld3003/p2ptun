#ifndef __p2ptun_h__
#define __p2ptun_h__

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef int (*OUTPUT_MSG)(char *msg);
	typedef void (*OUTPUT_DAT)(unsigned char *data, int length);

	enum
	{
		P2PTUN_OK = 0,
		P2PTUN_ERROR,
		P2PTUN_MEMERR,
		P2PTUN_STATUSERR,
	};

	struct P2PTUN_CONN_SESSION
	{
		char peername[32];

		int msgpipe;

		unsigned char status;
		unsigned int status_time;

		OUTPUT_MSG out_msg;
		OUTPUT_DAT out_dat;
		OUTPUT_DAT out_p2pdat;
	};

	struct P2PTUN_CONN_SESSION *p2ptun_alloc_session(char *peername);
	int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg);
	int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);
	int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);
	void p2ptun_mainloop(struct P2PTUN_CONN_SESSION *session);

#ifdef __cplusplus
};
#endif

#endif
